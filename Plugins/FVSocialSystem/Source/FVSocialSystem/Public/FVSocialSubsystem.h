#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVSocialSubsystem.generated.h"

/** Decays faction notoriety with in-game time. */
UCLASS()
class FVSOCIALSYSTEM_API UFVSocialSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	UFUNCTION()
	void HandleHourChanged();
};
