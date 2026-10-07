#include "FVDebugHUDSubsystem.h"

#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDebugHUDSubsystem)

namespace
{
	uint64 CursorFrame = 0;
	float CursorY = 0.f;
}

void UFVDebugHUDSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	DrawHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateUObject(this, &UFVDebugHUDSubsystem::Draw));
}

void UFVDebugHUDSubsystem::Deinitialize()
{
	UDebugDrawService::Unregister(DrawHandle);
	DrawHandle.Reset();
	Super::Deinitialize();
}

bool UFVDebugHUDSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFVDebugHUDSubsystem::Draw(UCanvas* Canvas, APlayerController* PlayerController)
{
	if (!IsEnabled() || Canvas == nullptr || PlayerController == nullptr || PlayerController->GetWorld() != GetWorld())
	{
		return;
	}

	if (CursorFrame != GFrameCounter)
	{
		CursorFrame = GFrameCounter;
		CursorY = 60.f;
	}

	TArray<FString> Lines;
	CollectLines(Lines);

	const UFont* Font = GEngine->GetSmallFont();
	const float LineHeight = Font->GetMaxCharHeight() + 2.f;

	Canvas->SetDrawColor(FColor::Yellow);
	Canvas->DrawText(Font, GetTitle(), 20.f, CursorY);
	CursorY += LineHeight;

	Canvas->SetDrawColor(FColor::White);
	for (const FString& Line : Lines)
	{
		Canvas->DrawText(Font, Line, 30.f, CursorY);
		CursorY += LineHeight;
	}

	CursorY += LineHeight;
}
