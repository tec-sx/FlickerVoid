#include "Movement/FVLocomotionData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLocomotionData)

UFVLocomotionData::UFVLocomotionData()
{
	Walk.ForwardSpeed = 180.f;
	Walk.StrafeSpeed = 160.f;
	Walk.BackwardSpeed = 150.f;

	Sprint.ForwardSpeed = 700.f;
	Sprint.StrafeSpeed = 700.f;
	Sprint.BackwardSpeed = 700.f;
	Sprint.Acceleration = 300.f;
	Sprint.GroundFriction = 3.f;

	Crouch.ForwardSpeed = 225.f;
	Crouch.StrafeSpeed = 200.f;
	Crouch.BackwardSpeed = 180.f;
}
