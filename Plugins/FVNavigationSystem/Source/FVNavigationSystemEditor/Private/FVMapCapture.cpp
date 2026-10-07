#include "FVMapCapture.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Editor.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "FVMapCaptureActor.h"
#include "FVMapDefinition.h"
#include "FVNavigationSystem.h"
#include "FVNavigationTypes.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "ObjectTools.h"
#include "RenderingThread.h"
#include "ScopedTransaction.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FVMapCapture"

namespace FVMapCapture
{
	static const FName DefaultLayerName(TEXT("Ground"));
	static const TCHAR* DefaultCaptureFolder = TEXT("/Game/Navigation/MapCaptures");

	static FString MakeTexturePath(const AFVMapCaptureActor& Actor, FName LayerName)
	{
		FString Name = Actor.TextureName;
		if (Name.IsEmpty())
		{
			const FString MapName = Actor.Map ? Actor.Map->GetName() : Actor.GetActorLabel();
			Name = FString::Printf(TEXT("T_%s_%s"), *MapName, *LayerName.ToString());
		}

		FString Folder = UFVNavigationSettings::Get().CaptureFolder.Path;
		if (Folder.IsEmpty())
		{
			Folder = DefaultCaptureFolder;
		}
		return Folder / ObjectTools::SanitizeObjectName(Name);
	}

	static UTextureRenderTarget2D* Render(AFVMapCaptureActor& Actor)
	{
		const FIntPoint Size = Actor.GetCaptureSize();

		UTextureRenderTarget2D* Target = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
		Target->RenderTargetFormat = RTF_RGBA8_SRGB;
		Target->ClearColor = FLinearColor::Black;
		Target->InitAutoFormat(Size.X, Size.Y);
		Target->UpdateResourceImmediate(true);

		USceneCaptureComponent2D* Capture = Actor.GetCaptureComponent();
		UTextureRenderTarget2D* PreviousTarget = Capture->TextureTarget;

		Actor.PrepareCapture();
		Capture->TextureTarget = Target;
		Capture->CaptureScene();
		FlushRenderingCommands();
		Capture->TextureTarget = PreviousTarget;

		return Target;
	}

	static UTexture2D* SaveTexture(AFVMapCaptureActor& Actor, UTextureRenderTarget2D* Target, const FFVMapLayer* Layer, FName LayerName)
	{
		if (UTexture2D* Existing = Layer ? Layer->Texture.LoadSynchronous() : nullptr)
		{
			UKismetRenderingLibrary::ConvertRenderTargetToTexture2DEditorOnly(&Actor, Target, Existing);
			return Existing;
		}

		UTexture2D* Texture = UKismetRenderingLibrary::RenderTargetCreateStaticTexture2DEditorOnly(
			Target, MakeTexturePath(Actor, LayerName), TC_Default, TMGS_FromTextureGroup);
		if (Texture != nullptr)
		{
			Texture->LODGroup = TEXTUREGROUP_UI;
			Texture->CompressionNoAlpha = true;
			Texture->PostEditChange();
		}
		return Texture;
	}

	FResult Capture(AFVMapCaptureActor& Actor)
	{
		FResult Result;

		if (Actor.GetWorld() == nullptr || Actor.GetCaptureComponent() == nullptr)
		{
			Result.Message = LOCTEXT("NoWorld", "Map capture actor is not in a world.");
			return Result;
		}

		const FScopedTransaction Transaction(LOCTEXT("CaptureMap", "Capture Map"));

		UFVMapDefinition* Map = Actor.Map;
		const FName LayerName = Actor.Layer.IsNone() ? DefaultLayerName : Actor.Layer;
		int32 LayerIndex = INDEX_NONE;

		if (Map != nullptr)
		{
			Map->Modify();
			LayerIndex = Map->FindLayerByName(LayerName);
			if (LayerIndex == INDEX_NONE)
			{
				LayerIndex = Map->Layers.AddDefaulted();
				Map->Layers[LayerIndex].Name = LayerName;
				Map->Layers[LayerIndex].DisplayName = FText::FromName(LayerName);
			}
		}

		UTextureRenderTarget2D* Target = Render(Actor);
		UTexture2D* Texture = SaveTexture(Actor, Target, Map ? Map->GetLayer(LayerIndex) : nullptr, LayerName);
		Target->ReleaseResource();

		if (Texture == nullptr)
		{
			Result.Message = FText::Format(LOCTEXT("SaveFailed", "{0}: the capture could not be saved as a texture. See the message log."),
				FText::FromString(Actor.GetActorLabel()));
			return Result;
		}

		Result.bSuccess = true;
		Result.Texture = Texture;

		const FIntPoint Size = Actor.GetCaptureSize();
		if (Map == nullptr)
		{
			Result.Message = FText::Format(LOCTEXT("SavedNoMap", "Captured {0} ({1}x{2}). No map is set, so no layer was updated."),
				FText::FromString(Texture->GetName()), Size.X, Size.Y);
			return Result;
		}

		const FBox2D Area = Actor.GetCapturedArea();
		const FBox Bounds = Actor.GetCaptureBounds();

		FFVMapLayer& Layer = Map->Layers[LayerIndex];
		Layer.Texture = Texture;
		Layer.WorldMin = Area.Min;
		Layer.WorldMax = Area.Max;
		Layer.MinZ = Bounds.Min.Z;
		Layer.MaxZ = Bounds.Max.Z;

		Map->PostEditChange();
		Map->MarkPackageDirty();

		Result.Message = FText::Format(LOCTEXT("Saved", "Captured {0} ({1}x{2}) into {3}, layer {4}."),
			FText::FromString(Texture->GetName()), Size.X, Size.Y, FText::FromString(Map->GetName()), FText::FromName(LayerName));
		return Result;
	}

	FResult CaptureAndNotify(AFVMapCaptureActor* Actor)
	{
		if (Actor == nullptr)
		{
			return FResult();
		}

		const FResult Result = Capture(*Actor);
		UE_LOG(LogFVNavigationSystem, Display, TEXT("%s"), *Result.Message.ToString());

		FNotificationInfo Info(Result.Message);
		Info.ExpireDuration = 6.f;
		Info.bUseSuccessFailIcons = true;

		if (Result.Texture != nullptr)
		{
			Info.HyperlinkText = LOCTEXT("ShowTexture", "Show texture");
			Info.Hyperlink = FSimpleDelegate::CreateLambda([Texture = TWeakObjectPtr<UTexture2D>(Result.Texture)]
			{
				if (Texture.IsValid() && GEditor)
				{
					const TArray<UObject*> Objects = { Texture.Get() };
					GEditor->SyncBrowserToObjects(Objects);
				}
			});
		}

		if (const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Item->SetCompletionState(Result.bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		}
		return Result;
	}

	TArray<AFVMapCaptureActor*> FindCaptureActors()
	{
		TArray<AFVMapCaptureActor*> Actors;
		UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (World != nullptr)
		{
			for (TActorIterator<AFVMapCaptureActor> It(World); It; ++It)
			{
				Actors.Add(*It);
			}
		}
		return Actors;
	}
}

#undef LOCTEXT_NAMESPACE
