#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVExamineSecretsComponent.generated.h"

class UFVKnowledgeDefinition;

/** A detail seen only from one examine angle; finding it teaches its clue. */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVExamineSecret
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Examine", meta = (ToolTip = "Rotation of the actor in the examine view that reveals the secret."))
	FRotator ViewAngle = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Examine", meta = (ToolTip = "Learned when the secret is found."))
	TObjectPtr<UFVKnowledgeDefinition> Clue;
};

/** Secrets found by turning the owner in the examine view; each one teaches a clue. Keep at most three per actor. */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVExamineSecretsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVExamineSecretsComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintPure, Category = "FV|Examine")
	bool HasSecrets() const { return !Secrets.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "FV|Examine", meta = (ToolTip = "Secrets whose clue is known."))
	int32 CountFound() const;

	UFUNCTION(BlueprintPure, Category = "FV|Examine", meta = (ToolTip = "Every secret's clue is known; secrets without a clue don't count."))
	bool AllFound() const;

	UFUNCTION(BlueprintCallable, Category = "FV|Examine", meta = (ToolTip = "Learns the clue of every unknown secret within Angle Tolerance of the view rotation. True if one was learned now."))
	bool TryFind(const FRotator& ViewRotation);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

private:
	bool IsWithinTolerance(const FRotator& ViewRotation, const FRotator& ViewAngle) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Examine", meta = (TitleProperty = "Clue"))
	TArray<FFVExamineSecret> Secrets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Examine", meta = (ClampMin = 0, Units = "Degrees", ToolTip = "How far pitch, yaw and roll may each be from a secret's View Angle."))
	float AngleTolerance = 12.f;
};
