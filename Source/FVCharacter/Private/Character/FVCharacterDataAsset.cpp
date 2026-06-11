// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/FVCharacterDataAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacterDataAsset)

UFVCharacterDataAsset::UFVCharacterDataAsset()
{
	AttributeInitializationEffect = nullptr;
}

FPrimaryAssetId UFVCharacterDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("FVCharacterData"), GetFName());
}