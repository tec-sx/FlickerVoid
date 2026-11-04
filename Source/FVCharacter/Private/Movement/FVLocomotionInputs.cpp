#include "Movement/FVLocomotionInputs.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLocomotionInputs)

FMoverDataStructBase* FFVLocomotionInputs::Clone() const
{
	return new FFVLocomotionInputs(*this);
}

bool FFVLocomotionInputs::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);

	uint8 GaitValue = static_cast<uint8>(Gait);
	Ar << GaitValue;
	Gait = static_cast<EFVGait>(GaitValue);
	LocalMoveDirection.NetSerialize(Ar, Map, bOutSuccess);
	Ar.SerializeBits(&bFacingControl, 1);
	Ar.SerializeBits(&bAiming, 1);
	Ar.SerializeBits(&bTraversing, 1);

	bOutSuccess = true;
	return true;
}

void FFVLocomotionInputs::ToString(FAnsiStringBuilderBase& Out) const
{
	Super::ToString(Out);

	Out.Appendf("Gait: %d | Move: %s | FacingControl: %d | Aiming: %d | Traversing: %d\n",
		static_cast<int32>(Gait), TCHAR_TO_ANSI(*LocalMoveDirection.ToCompactString()), bFacingControl, bAiming, bTraversing);
}

bool FFVLocomotionInputs::ShouldReconcile(const FMoverDataStructBase& AuthorityState) const
{
	const FFVLocomotionInputs& Authority = static_cast<const FFVLocomotionInputs&>(AuthorityState);
	return Gait != Authority.Gait || bFacingControl != Authority.bFacingControl || bAiming != Authority.bAiming
		|| bTraversing != Authority.bTraversing || !LocalMoveDirection.Equals(Authority.LocalMoveDirection);
}

void FFVLocomotionInputs::Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct)
{
	*this = static_cast<const FFVLocomotionInputs&>(Pct < 0.5f ? From : To);
}

void FFVLocomotionInputs::Merge(const FMoverDataStructBase& From)
{
	const FFVLocomotionInputs& Other = static_cast<const FFVLocomotionInputs&>(From);
	bTraversing |= Other.bTraversing;
}
