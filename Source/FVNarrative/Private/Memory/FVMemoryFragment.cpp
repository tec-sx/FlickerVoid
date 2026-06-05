// Fill out your copyright notice in the Description page of Project Settings.

#include "Memory/FVMemoryFragment.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMemoryFragment)

UFVMemoryFragment::UFVMemoryFragment()
{
	MemoryId = NAME_None;
	IdentityContribution = 0.05f;
	SanityImpact = 0.0f;
	MinimumSanityForClarity = 0.0f;
}

FPrimaryAssetId UFVMemoryFragment::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("FVMemory"), GetFName());
}