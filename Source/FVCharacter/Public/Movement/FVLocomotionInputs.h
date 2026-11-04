#pragma once

#include "CoreMinimal.h"
#include "FVCharacterTypes.h"
#include "MoverDataModelTypes.h"
#include "FVLocomotionInputs.generated.h"

/**
 * FlickerVoid's movement intent, sent to Mover next to its default character inputs every step.
 * Movement modes and transitions read it through UFVLocomotionLibrary.
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVLocomotionInputs : public FMoverDataStructBase
{
	GENERATED_BODY()

	virtual FMoverDataStructBase* Clone() const override;
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual void ToString(FAnsiStringBuilderBase& Out) const override;
	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override;
	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override;
	virtual void Merge(const FMoverDataStructBase& From) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", meta = (ToolTip = "Gait in effect: Sprinting only while sprint is held and the input points forward."))
	EFVGait Gait = EFVGait::Running;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", meta = (ToolTip = "Move input in control space: X forward, Y right."))
	FVector LocalMoveDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", meta = (ToolTip = "True while the character faces the camera (aiming or sideways input) instead of its movement."))
	bool bFacingControl = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bTraversing = false;
};

template<>
struct TStructOpsTypeTraits<FFVLocomotionInputs> : public TStructOpsTypeTraitsBase2<FFVLocomotionInputs>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
