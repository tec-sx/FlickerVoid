#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVCharacterTypes.h"
#include "GameplayTagContainer.h"
#include "FVLocomotionComponent.generated.h"

class UCharacterMoverComponent;
class UFVLocomotionData;
struct FFVGaitSettings;
struct FMoverInputCmdContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnLanded, FVector, LandingVelocity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnStartedFalling);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnGaitChanged, EFVGait, NewGait);

/**
 * Movement intent and state of a character driven by Mover's Character Mover Component.
 * Input sets the intent (move, gait, crouch, jump, aim); the component turns it into Mover input every simulation step,
 * writes the active gait into Mover's Common Legacy Movement Settings and keeps the state animation and traversal read.
 */
UCLASS(ClassGroup = (FlickerVoid), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDCHARACTER_API UFVLocomotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVLocomotionComponent();

	static UFVLocomotionComponent* Find(const AActor* Actor);

	/** Fills Mover's default character inputs. Called by the owning pawn, which is Mover's input producer. */
	void ProduceInput(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent", meta = (ToolTip = "Sets the move input in control space: X forward, Y right, each -1 to 1. Zero stops."))
	void RequestMove(FVector Direction);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent", meta = (ToolTip = "Switches between walking and running. Ignored while sprinting."))
	void ToggleWalk();

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent", meta = (ToolTip = "Starts or stops sprinting. Starting stands the character up."))
	void SetSprinting(bool bValue);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent", meta = (ToolTip = "Crouches, or stands up when crouching."))
	void ToggleCrouch();

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent")
	void StopCrouching();

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent", meta = (ToolTip = "Presses jump. When crouching, stands up instead."))
	void StartJump();

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent")
	void StopJump();

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent", meta = (ToolTip = "While aiming the character faces the camera."))
	void SetAiming(bool bValue);

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Intent", meta = (ToolTip = "Marks a traversal action (vault, mantle, hurdle) as running, for animation and the traversal check."))
	void SetTraversing(bool bValue);

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	FVector GetMoveDirection() const { return MoveDirection; }

	UFUNCTION(BlueprintPure, Category = "Locomotion|State", meta = (ToolTip = "The gait in effect: sprint only counts while moving forward."))
	EFVGait GetGait() const { return Gait; }

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	EFVStance GetStance() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	EFVMovementMode GetMovementMode() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	bool IsWalking() const { return bWantsToWalk; }

	UFUNCTION(BlueprintPure, Category = "Locomotion|State", meta = (ToolTip = "True while sprint is held, even when the gait is not Sprinting yet."))
	bool WantsToSprint() const { return bWantsToSprint; }

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	bool IsAiming() const { return bAiming; }

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	bool IsTraversing() const { return bTraversing; }

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	bool IsCrouching() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	bool IsOnGround() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion|State", meta = (ToolTip = "Falling, including the way up of a jump."))
	bool IsFalling() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion|State")
	bool JustLanded() const { return JustLandedTimeLeft > 0.f; }

	UFUNCTION(BlueprintPure, Category = "Locomotion|State", meta = (ToolTip = "False while move input is ignored or the owner's ability system has a movement blocking tag."))
	bool CanMove() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion|State", meta = (ToolTip = "Horizontal speed on the ground, 0 in the air."))
	float GetGroundSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion|State", meta = (ToolTip = "Snapshot of the movement state for animation."))
	FFVCharacterRuntimeState GetRuntimeState() const;

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	UCharacterMoverComponent* GetMover() const { return Mover; }

	UPROPERTY(BlueprintAssignable, Category = "Locomotion|Events")
	FFVOnLanded OnLanded;

	UPROPERTY(BlueprintAssignable, Category = "Locomotion|Events", meta = (ToolTip = "Broadcast when the character leaves the ground without jumping or after a jump starts."))
	FFVOnStartedFalling OnStartedFalling;

	UPROPERTY(BlueprintAssignable, Category = "Locomotion|Events")
	FFVOnGaitChanged OnGaitChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion", meta = (ToolTip = "Ability system tags that stop movement input."))
	FGameplayTagContainer MovementBlockingTags;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion", meta = (ToolTip = "Gait speeds and turning. Empty uses the defaults of UFVLocomotionData."))
	TObjectPtr<const UFVLocomotionData> LocomotionData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion", meta = (ClampMin = "0", ToolTip = "How long JustLanded stays true after landing, in seconds."))
	float JustLandedDuration = 0.3f;

private:
	const UFVLocomotionData& GetData() const;
	const FFVGaitSettings& GetGaitSettings() const;
	EFVGait ResolveGait() const;
	bool ShouldFaceControl() const;
	float GetDirectionalSpeedScale() const;
	float GetSpeedMultiplier() const;
	FRotator GetControlYaw() const;
	void ApplyGaitToMover() const;
	void UpdateMovementEvents(float DeltaTime);

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMoverComponent> Mover;

	FVector MoveDirection = FVector::ZeroVector;
	FVector LastOrientationIntent = FVector::ForwardVector;
	EFVGait Gait = EFVGait::Running;
	bool bWantsToWalk = false;
	bool bWantsToSprint = false;
	bool bAiming = false;
	bool bTraversing = false;
	bool bJumpJustPressed = false;
	bool bJumpHeld = false;

	bool bWasOnGround = true;
	bool bWasFalling = false;
	FVector LastVelocity = FVector::ZeroVector;
	FVector LandingVelocity = FVector::ZeroVector;
	float JustLandedTimeLeft = 0.f;
};
