// Fill out your copyright notice in the Description page of Project Settings.


#include "FVInteractionSystemUISettings.h"

#include "GameplayTagContainer.h"
#include "Core/InteractionUITypes.h"

UFVInteractionSystemUISettings::UFVInteractionSystemUISettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = "Flicker Void UI";
	SectionName = "Interaction System UI";
}

FInteractionSlotStyle UFVInteractionSystemUISettings::GetSlotStyle(const FGameplayTag ActionTag) const
{
	if (const UDataTable* SlotStyleTable = InteractionSlotStyles.Get())
	{
		const auto SlotStyle = SlotStyleTable->FindRow<FInteractionSlotStyle>(ActionTag.GetTagName(), TEXT("GetSlotStyle"), true);
		
		return *SlotStyle;
	}
	
	return FInteractionSlotStyle();
}
