#include "Traversal/FVTraversalComponent.h"

#include "Animation/AnimMontage.h"
#include "Chooser.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"
#include "Character/FVCharacter.h"
#include "Traversal/FVTraversable.h"
#include "Traversal/FVTraversalConfig.h"

void UFVTraversalComponentNative::BeginPlay()
{
Super::BeginPlay();
if (!Config)
{
Config = NewObject<UFVTraversalConfigNative>(this);
}
}

ACharacter* UFVTraversalComponentNative::GetCharacter() const
{
return Cast<ACharacter>(GetOwner());
}

void UFVTraversalComponentNative::ReadDebugSettings()
{
const IConsoleVariable* Level = IConsoleManager::Get().FindConsoleVariable(TEXT("FVCvar.Traversal.DrawDebugLevel"));
const IConsoleVariable* Duration = IConsoleManager::Get().FindConsoleVariable(TEXT("FVCvar.Traversal.DrawDebugDuration"));
DebugLevel = Level ? Level->GetInt() : 0;
DebugDuration = Duration ? Duration->GetFloat() : 0.f;
}

bool UFVTraversalComponentNative::TryTraversalAction(EDrawDebugTrace::Type DrawDebugType)
{
ACharacter* Character = GetCharacter();
if (!Character || !Config)
{
return false;
}
DebugType = DrawDebugType;
ReadDebugSettings();

FFVTraversalCheckResultNative Result;
if (!DoForwardTrace(*Character, MakeTraceParams(*Character), Result))
{
return false;
}

FHitResult BackHit;
if (!DoClearanceTraces(*Character, Result, BackHit))
{
return false;
}

if (BackHit.bBlockingHit)
{
Result.ObstacleDepth = FVector::Dist(BackHit.ImpactPoint, Result.Ledges.FrontLocation);
Result.Ledges.bHasBackLedge = false;
}
else
{
Result.ObstacleDepth = FVector::Dist(Result.Ledges.FrontLocation, Result.Ledges.BackLocation);
Result.Ledges.bHasBackLedge = true;
DoFloorTrace(*Character, Result);
}
Result.ActionType = ClassifyAction(Result);
LastResult = Result;

LastChooserOutput = EvaluateChooserTable(MakeChooserInput(*Character), Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr);
OnTraversalFound.Broadcast();
return true;
}

FFVTraversalChooserOutputNative UFVTraversalComponentNative::EvaluateChooserTable_Implementation(const FFVTraversalChooserInputNative& Input, UAnimInstance* AnimInstance)
{
FFVTraversalChooserOutputNative Output;
Output.ActionType = Input.ActionType;

const UFVTraversalConfigNative* TraversalConfig = GetConfig();
if (!TraversalConfig || !TraversalConfig->TraversalChooserTable)
{
return Output;
}

FFVTraversalChooserInputNative MutableInput = Input;
FChooserEvaluationContext Context(AnimInstance);
Context.AddStructParam(MutableInput);
Context.AddStructParam(Output);

UChooserTable::EvaluateChooser(Context, TraversalConfig->TraversalChooserTable,
FObjectChooserBase::FObjectChooserIteratorCallback::CreateLambda([&Output](UObject* Result)
{
UAnimMontage* Montage = Cast<UAnimMontage>(Result);
if (!Montage)
{
return FObjectChooserBase::EIteratorStatus::Continue;
}
Output.MontageToPlay = Montage;
return FObjectChooserBase::EIteratorStatus::Stop;
}));

return Output;
}

FTransform UFVTraversalComponentNative::GetInteractionTransform() const
{
return FTransform(FRotationMatrix::MakeFromZ(LastResult.Ledges.FrontNormal).Rotator(), LastResult.Ledges.FrontLocation);
}

UFVTraversalComponentNative::FTraceParams UFVTraversalComponentNative::MakeTraceParams(const ACharacter& Character) const
{
FTraceParams Params;
const UCharacterMovementComponent* Movement = Character.GetCharacterMovement();
if (Movement && (Movement->IsFalling() || Movement->IsFlying()))
{
Params.ForwardDistance = Config->AirTraceForwardDistance;
Params.OriginOffset = Config->AirTraceOriginOffset;
Params.EndOffset = Config->AirTraceEndOffset;
Params.HalfHeight = Config->AirTraceHalfHeight;
return Params;
}
const float ForwardSpeed = Character.GetActorRotation().UnrotateVector(Character.GetVelocity()).X;
Params.ForwardDistance = FMath::GetMappedRangeValueClamped(
Config->GroundSpeedRange,
FVector2D(Config->GroundTraceForwardDistanceMin, Config->GroundTraceForwardDistanceMax),
ForwardSpeed);
Params.OriginOffset = Config->GroundTraceOriginOffset;
Params.EndOffset = Config->GroundTraceEndOffset;
Params.HalfHeight = Config->GroundTraceHalfHeight;
return Params;
}

bool UFVTraversalComponentNative::DoForwardTrace(const ACharacter& Character, const FTraceParams& Params, FFVTraversalCheckResultNative& Result) const
{
const FVector Location = Character.GetActorLocation();
const FVector Start = Location + Params.OriginOffset;
const FVector End = Start + Character.GetActorForwardVector() * Params.ForwardDistance + Params.EndOffset;

FHitResult Hit;
UKismetSystemLibrary::CapsuleTraceSingle(
GetOwner(), Start, End, Config->TraceRadius, Params.HalfHeight, Config->ObstacleTraceType, false, {},
DebugLevel >= 2 ? DebugType.GetValue() : EDrawDebugTrace::None, Hit, true,
FLinearColor::Black, FLinearColor::Black, DebugDuration);

const AFVTraversableNative* Traversable = Cast<AFVTraversableNative>(Hit.GetActor());
if (!Hit.bBlockingHit || !Traversable || !Traversable->CanTraverse(GetOwner()))
{
return false;
}

const FFVLedgeResult Ledges = Traversable->GetLedgeTransforms(Hit.ImpactPoint, Location);
if (DebugLevel >= 1)
{
DrawLedges(Ledges);
}
if (!Ledges.bHasFrontLedge)
{
return false;
}

const float HalfHeight = Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
Result.HitComponent = Hit.GetComponent();
Result.Ledges = Ledges;
Result.ObstacleHeight = FMath::Abs(Location.Z - HalfHeight - Ledges.FrontLocation.Z);
return true;
}

bool UFVTraversalComponentNative::DoClearanceTraces(const ACharacter& Character, const FFVTraversalCheckResultNative& Result, FHitResult& OutBackHit) const
{
const UCapsuleComponent* Capsule = Character.GetCapsuleComponent();
const float Offset = Capsule->GetScaledCapsuleRadius() + Config->CapsuleOffsetDistance;
const FVector Up(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight() + Config->CapsuleOffsetDistance);

const FVector AboveFront = Result.Ledges.FrontLocation + Result.Ledges.FrontNormal * Offset + Up;
FHitResult FrontHit;
CapsuleTrace(Character, Character.GetActorLocation(), AboveFront, FrontHit);
if (FrontHit.bBlockingHit || FrontHit.bStartPenetrating)
{
return false;
}

const FVector AboveBack = Result.Ledges.BackLocation + Result.Ledges.BackNormal * Offset + Up;
CapsuleTrace(Character, AboveFront, AboveBack, OutBackHit);
return true;
}

void UFVTraversalComponentNative::DoFloorTrace(const ACharacter& Character, FFVTraversalCheckResultNative& Result) const
{
const UCapsuleComponent* Capsule = Character.GetCapsuleComponent();
const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
const float Offset = Capsule->GetScaledCapsuleRadius() + Config->CapsuleOffsetDistance;
const FVector Up(0.f, 0.f, HalfHeight + Config->CapsuleOffsetDistance);
const FVector Down(0.f, 0.f, Result.ObstacleHeight - HalfHeight + Config->FloorTraceVerticalOffset);
const FVector Base = Result.Ledges.BackLocation + Result.Ledges.BackNormal * Offset;

FHitResult Hit;
CapsuleTrace(Character, Base + Up, Base - Down, Hit);
if (!Hit.bBlockingHit)
{
return;
}
Result.bHasBackFloor = true;
Result.BackFloorLocation = Hit.ImpactPoint;
Result.BackLedgeHeight = FMath::Abs(Hit.ImpactPoint.Z - Result.Ledges.BackLocation.Z);
}

void UFVTraversalComponentNative::CapsuleTrace(const ACharacter& Character, const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
const UCapsuleComponent* Capsule = Character.GetCapsuleComponent();
UKismetSystemLibrary::CapsuleTraceSingle(
GetOwner(), Start, End, Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight(),
UEngineTypes::ConvertToTraceType(ECC_Visibility), false, {},
DebugLevel >= 3 ? DebugType.GetValue() : EDrawDebugTrace::None, OutHit, true,
FLinearColor::Red, FLinearColor::Green, DebugDuration);
}

FFVTraversalChooserInputNative UFVTraversalComponentNative::MakeChooserInput(const ACharacter& Character) const
{
FFVTraversalChooserInputNative Input;
Input.ActionType = LastResult.ActionType;
Input.bHasFrontLedge = LastResult.Ledges.bHasFrontLedge;
Input.bHasBackLedge = LastResult.Ledges.bHasBackLedge;
Input.bHasBackFloor = LastResult.bHasBackFloor;
Input.ObstacleHeight = LastResult.ObstacleHeight;
Input.ObstacleDepth = LastResult.ObstacleDepth;
Input.BackLedgeHeight = LastResult.BackLedgeHeight;
Input.DistanceToLedge = FVector::Dist(LastResult.Ledges.FrontLocation, Character.GetActorLocation());
Input.MovementMode = ToTraversalMode(Character.GetCharacterMovement()->MovementMode);
Input.Speed = Character.GetVelocity().Size2D();
if (const AFVCharacter* FVCharacter = Cast<AFVCharacter>(&Character))
{
Input.Gait = FVCharacter->GetRuntimeState().Gait;
}
return Input;
}

EFVTraversalActionTypeNative UFVTraversalComponentNative::ClassifyAction(const FFVTraversalCheckResultNative& Result)
{
if (!Result.Ledges.bHasBackLedge)
{
return EFVTraversalActionTypeNative::Mantle;
}
return Result.bHasBackFloor ? EFVTraversalActionTypeNative::Hurdle : EFVTraversalActionTypeNative::Vault;
}

EFVMovementMode UFVTraversalComponentNative::ToTraversalMode(EMovementMode Mode)
{
return Mode == MOVE_Falling || Mode == MOVE_Flying ? EFVMovementMode::InAir : EFVMovementMode::OnGround;
}

void UFVTraversalComponentNative::DrawLedges(const FFVLedgeResult& Ledges) const
{
if (Ledges.bHasFrontLedge)
{
DrawDebugSphere(GetWorld(), Ledges.FrontLocation, 10.f, 12, FColor::Green, false, DebugDuration);
}
if (Ledges.bHasBackLedge)
{
DrawDebugSphere(GetWorld(), Ledges.BackLocation, 10.f, 12, FColor::Cyan, false, DebugDuration);
}
}