// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "Engine/StreamableManager.h"

class FYapStreamableManager
{
public:
	static FStreamableManager& Get()
	{
		static FStreamableManager Instance;
		return Instance;
	}
};
