#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Traversal/FVTraversalTypes.h"
#include "FVTraversalComponent.generated.h"

class ACharacter;
class UAnimInstance;
class UFVTraversalConfigNative;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnTraversalChanged);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class FLICKERVOIDCHARACTER_API UFVTraversalComponentNative : public UActorComponent
{
GENERATED_BODY()

public:
UFUNCTION(BlueprintCallable, Category = "Traversal")
bool TryTraversalAction(EDrawDebugTrace::Type DrawDebugType);

UFUNCTION(BlueprintPure, Category = "Traversal")
const FFVTraversalCheckResultNative& GetLastResult() const { return LastResult; }

UFUNCTION(BlueprintPure, Category = "Traversal")
const FFVTraversalChooserOutputNative& GetLastChooserOutput() const { return LastChooserOutput; }

UFUNCTION(BlueprintPure, Category = "Traversal")
FTransform GetInteractionTransform() const;

UFUNCTION(BlueprintPure, Category = "Traversal")
UFVTraversalConfigNative* GetConfig() const { return Config; }

UPROPERTY(BlueprintAssignable, Category = "Traversal")
FFVOnTraversalChanged OnTraversalFound;

protected:
virtual void BeginPlay() override;

UFUNCTION(BlueprintNativeEvent, Category = "Traversal")
FFVTraversalChooserOutputNative EvaluateChooserTable(const FFVTraversalChooserInputNative& Input, UAnimInstance* AnimInstance);

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
TObjectPtr<UFVTraversalConfigNative> Config;

private:
struct FTraceParams
{
float ForwardDistance = 0.f;
FVector OriginOffset = FVector::ZeroVector;
FVector EndOffset = FVector::ZeroVector;
float HalfHeight = 0.f;
};

ACharacter* GetCharacter() const;
FTraceParams MakeTraceParams(const ACharacter& Character) const;
bool DoForwardTrace(const ACharacter& Character, const FTraceParams& Params, FFVTraversalCheckResultNative& Result) const;
bool DoClearanceTraces(const ACharacter& Character, const FFVTraversalCheckResultNative& Result, FHitResult& OutBackHit) const;
void DoFloorTrace(const ACharacter& Character, FFVTraversalCheckResultNative& Result) const;
void CapsuleTrace(const ACharacter& Character, const FVector& Start, const FVector& End, FHitResult& OutHit) const;
FFVTraversalChooserInputNative MakeChooserInput(const ACharacter& Character) const;
static EFVTraversalActionTypeNative ClassifyAction(const FFVTraversalCheckResultNative& Result);
static EFVMovementMode ToTraversalMode(EMovementMode Mode);
void ReadDebugSettings();
void DrawLedges(const FFVLedgeResult& Ledges) const;

FFVTraversalCheckResultNative LastResult;
FFVTraversalChooserOutputNative LastChooserOutput;
TEnumAsByte<EDrawDebugTrace::Type> DebugType = EDrawDebugTrace::None;
int32 DebugLevel = 0;
float DebugDuration = 0.f;
};