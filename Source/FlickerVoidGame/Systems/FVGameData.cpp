// Fill out your copyright notice in the Description page of Project Settings.

#include "Systems/FVGameData.h"
#include "Systems/FVAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVGameData)

const UFVGameData& UFVGameData::Get()
{
	return UFVAssetManager::Get().GetGameData();
}
