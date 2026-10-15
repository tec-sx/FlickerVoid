#pragma once

#include "CoreMinimal.h"
#include "FlickerVoidDeveloperSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Flicker Void"))
class FLICKERVOID_API UFlickerVoidDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:

    UFlickerVoidDeveloperSettings()
    {
        CategoryName = TEXT("Game");
        SectionName  = TEXT("Flicker Void");
    }
};