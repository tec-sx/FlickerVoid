#include "FVRelevanceDebugSubsystem.h"

#include "DrawDebugHelpers.h"
#include "FVDebugUtils.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"
#include "Relevance/FVRelevanceComponent.h"
#include "Relevance/FVRelevanceSettings.h"
#include "Relevance/FVRelevanceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRelevanceDebugSubsystem)

static TAutoConsoleVariable<int32> CVarRelevanceDebugHUD(
	TEXT("FVCvar.Relevance.Debug.HUD"),
	0,
	TEXT("1: show how many objects each relevance group has per band. 2: also draw the band rings and color objects by band (filter with FVCvar.Relevance.Debug.Group)."));

static TAutoConsoleVariable<FString> CVarRelevanceDebugGroup(
	TEXT("FVCvar.Relevance.Debug.Group"),
	TEXT(""),
	TEXT("Only show relevance groups whose tag contains this text."));

namespace
{
	FColor GetBandColor(const EFVRelevanceBand Band)
	{
		switch (Band)
		{
		case EFVRelevanceBand::Near: return FColor::Green;
		case EFVRelevanceBand::Mid: return FColor::Yellow;
		default: return FColor::Red;
		}
	}
}

bool UFVRelevanceDebugSubsystem::IsEnabled() const
{
	return CVarRelevanceDebugHUD.GetValueOnGameThread() > 0;
}

void UFVRelevanceDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const UFVRelevanceSubsystem* Relevance = UFVRelevanceSubsystem::Get(GetWorld());
	if (Relevance == nullptr)
	{
		OutLines.Add(TEXT("no relevance subsystem"));
		return;
	}

	const bool bDraw = CVarRelevanceDebugHUD.GetValueOnGameThread() > 1;
	const FString Filter = CVarRelevanceDebugGroup.GetValueOnGameThread();

	OutLines.Add(FString::Printf(TEXT("%d viewpoint(s)"), Relevance->GetViewpoints().Num()));

	for (const FFVRelevanceGroup& Group : UFVRelevanceSettings::Get().Groups)
	{
		const FString GroupName = Group.Group.ToString();
		if (!Filter.IsEmpty() && !GroupName.Contains(Filter))
		{
			continue;
		}

		TArray<UFVRelevanceComponent*> Components;
		Relevance->GetComponents(Group.Group, Components);

		int32 Counts[3] = {};
		for (const UFVRelevanceComponent* Component : Components)
		{
			++Counts[static_cast<uint8>(Component->GetBand())];

			if (bDraw && Component->GetOwner() != nullptr)
			{
				DrawDebugPoint(GetWorld(), Component->GetOwner()->GetActorLocation(), 12.f, GetBandColor(Component->GetBand()));
			}
		}

		OutLines.Add(FString::Printf(TEXT("%s: near %d, mid %d, far %d"), *GroupName, Counts[0], Counts[1], Counts[2]));

		if (bDraw)
		{
			for (const FTransform& Viewpoint : Relevance->GetViewpoints())
			{
				DrawDebugCircle(GetWorld(), Viewpoint.GetLocation(), Group.NearDistance, 64, FColor::Green, false, -1.f, 0, 1.f, FVector::ForwardVector, FVector::RightVector, false);
				DrawDebugCircle(GetWorld(), Viewpoint.GetLocation(), Group.MidDistance, 64, FColor::Yellow, false, -1.f, 0, 1.f, FVector::ForwardVector, FVector::RightVector, false);
			}
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs CmdRelevanceList(
	TEXT("FV.Relevance.List"),
	TEXT("FV.Relevance.List [Group] - log registered relevance components with their band and distance."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const UFVRelevanceSubsystem* Relevance = UFVRelevanceSubsystem::Get(World);
		if (Relevance == nullptr)
		{
			return;
		}

		for (const FFVRelevanceGroup& Group : UFVRelevanceSettings::Get().Groups)
		{
			if (!Args.IsEmpty() && !Group.Group.ToString().Contains(Args[0]))
			{
				continue;
			}

			TArray<UFVRelevanceComponent*> Components;
			Relevance->GetComponents(Group.Group, Components);

			for (const UFVRelevanceComponent* Component : Components)
			{
				UE_LOG(LogFVDebug, Display, TEXT("%s %s: %s, %.0f cm"), *Group.Group.ToString(), *GetNameSafe(Component->GetOwner()),
					*UEnum::GetDisplayValueAsText(Component->GetBand()).ToString(), Component->GetDistance());
			}
		}
	}));
