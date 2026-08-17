// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVInteractionSet.h"
#include "GameplayTagContainer.h"
#include "Interactions/FVInteractionTypes.h"
#include "Engine/DataAsset.h"
#include "FVInteractionTargetConfig.generated.h"

UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionTargetConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Interactable"))
	FGameplayTag Type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Interactable"))
	TObjectPtr<UFVInteractionSet> InteractionSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float FocusRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	EFVInteractionFocusProfile FocusProfile = EFVInteractionFocusProfile::Precise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection",
		meta = (EditCondition = "FocusProfile == EFVInteractionFocusProfile::Custom"))
	FFVInteractionFocusProfile CustomFocusProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	FName AimProbeSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	FVector AimProbeOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	float ZonePadding = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	float ZoneWidthScale = 3.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	float ZoneDepthScale = 4.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	float MinZoneExtent = 50.f;

	FFVInteractionFocusProfile GetFocusProfile() const
	{
		switch (FocusProfile)
		{
		case EFVInteractionFocusProfile::Precise:
			return FFVInteractionFocusProfile::Precise();
		case EFVInteractionFocusProfile::Loose:
			return FFVInteractionFocusProfile::Loose();
		case EFVInteractionFocusProfile::Custom:
		default:
			return CustomFocusProfile;
		}
	}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
