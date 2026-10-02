#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVEffect.h"
#include "Data/FVDefinition.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVCinematic.generated.h"

class ALevelSequenceActor;
class ULevelSequence;
class ULevelSequencePlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnCinematicEvent);

/** A cinematic. Id fact is set to 1 once watched (skipped counts as watched). */
UCLASS(BlueprintType)
class FVCORERUNTIME_API UFVCinematicDefinition : public UFVDefinition
{
GENERATED_BODY()

public:
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cinematic")
TSoftObjectPtr<ULevelSequence> Sequence;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cinematic")
bool bSkippable = true;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cinematic")
bool bDisablePlayerInput = true;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cinematic")
bool bHidePlayer = false;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cinematic")
FFVEffectList OnFinished;
};

UCLASS()
class FVCORERUNTIME_API UFVCinematicSubsystem : public UWorldSubsystem
{
GENERATED_BODY()

public:
static UFVCinematicSubsystem* Get(const UObject* WorldContext);

/** Stops any current cinematic first. Returns false if the sequence failed to load. */
UFUNCTION(BlueprintCallable, Category = "Cinematic")
bool Play(UFVCinematicDefinition* Cinematic);

/** Jumps to the end; finish handling runs as normal. No-op if not skippable. */
UFUNCTION(BlueprintCallable, Category = "Cinematic")
void Skip();

UFUNCTION(BlueprintPure, Category = "Cinematic")
bool IsPlaying() const { return Current != nullptr; }

UFUNCTION(BlueprintPure, Category = "Cinematic")
bool CanSkip() const { return Current && Current->bSkippable; }

UFUNCTION(BlueprintPure, Category = "Cinematic")
UFVCinematicDefinition* GetCurrent() const { return Current; }

UPROPERTY(BlueprintAssignable, Category = "Cinematic")
FFVOnCinematicEvent OnStarted;

UPROPERTY(BlueprintAssignable, Category = "Cinematic")
FFVOnCinematicEvent OnFinished;

private:
UFUNCTION()
void HandleFinished();

void SetPlayerState(bool bInCinematic) const;
void Cleanup();

UPROPERTY()
TObjectPtr<UFVCinematicDefinition> Current;

UPROPERTY()
TObjectPtr<ULevelSequencePlayer> Player;

UPROPERTY()
TObjectPtr<ALevelSequenceActor> SequenceActor;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Play Cinematic"))
struct FVCORERUNTIME_API FFVEffect_PlayCinematic : public FFVEffectBase
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, Category = "Effect")
TObjectPtr<UFVCinematicDefinition> Cinematic;

virtual void Apply(const FFVConditionContext& Context) const override;
virtual FText GetDescription() const override;
};