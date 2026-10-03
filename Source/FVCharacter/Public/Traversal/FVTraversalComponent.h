#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Traversal/FVTraversalTypes.h"
#include "FVTraversalComponent.generated.h"

class ACharacter;
class UAnimInstance;
class UFVTraversalConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnTraversalChanged);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class FLICKERVOIDCHARACTER_API UFVTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	bool TryTraversalAction(EDrawDebugTrace::Type DrawDebugType);

	UFUNCTION(BlueprintPure, Category = "Traversal")
	const FFVTraversalCheckResult& GetLastResult() const { return LastResult; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	const FFVTraversalChooserOutput& GetLastChooserOutput() const { return LastChooserOutput; }

	UFUNCTION(BlueprintPure, Category = "Traversal")
	FTransform GetInteractionTransform() const;

	UFUNCTION(BlueprintPure, Category = "Traversal")
	UFVTraversalConfig* GetConfig() const { return Config; }

	UPROPERTY(BlueprintAssignable, Category = "Traversal")
	FFVOnTraversalChanged OnTraversalFound;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Traversal")
	FFVTraversalChooserOutput EvaluateChooserTable(const FFVTraversalChooserInput& Input, UAnimInstance* AnimInstance);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UFVTraversalConfig> Config;

private:
	// TODO: Should we make generic trace paramas for line/sphere/capsule in FVFramework?
	struct FTraceParams
	{
		float ForwardDistance = 0.f;
		FVector OriginOffset = FVector::ZeroVector;
		FVector EndOffset = FVector::ZeroVector;
		float HalfHeight = 0.f;
	};

	ACharacter* GetCharacter() const;
	FTraceParams MakeTraceParams(const ACharacter& Character) const;
	bool DoForwardTrace(const ACharacter& Character, const FTraceParams& Params, FFVTraversalCheckResult& Result) const;
	bool DoClearanceTraces(const ACharacter& Character, const FFVTraversalCheckResult& Result,
	                       FHitResult& OutBackHit) const;
	void DoFloorTrace(const ACharacter& Character, FFVTraversalCheckResult& Result) const;
	void CapsuleTrace(const ACharacter& Character, const FVector& Start, const FVector& End, FHitResult& OutHit) const;
	FFVTraversalChooserInput MakeChooserInput(const ACharacter& Character) const;
	static EFVTraversalActionType ClassifyAction(const FFVTraversalCheckResult& Result);
	static EFVMovementMode ToTraversalMode(EMovementMode Mode);
	void ReadDebugSettings();
	void DrawLedges(const FFVLedgeResult& Ledges) const;

	FFVTraversalCheckResult LastResult;
	FFVTraversalChooserOutput LastChooserOutput;
	TEnumAsByte<EDrawDebugTrace::Type> DebugType = EDrawDebugTrace::None;
	int32 DebugLevel = 0;
	float DebugDuration = 0.f;
};
