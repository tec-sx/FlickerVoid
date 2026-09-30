#include "Math/FVEasingLibrary.h"

float UFVEasingLibrary::Ease(float Alpha, EFVEasing Easing, float Exponent)
{
	const float A = FMath::Clamp(Alpha, 0.f, 1.f);

	switch (Easing)
	{
	case EFVEasing::EaseIn:     return FMath::InterpEaseIn(0.f, 1.f, A, Exponent);
	case EFVEasing::EaseOut:    return FMath::InterpEaseOut(0.f, 1.f, A, Exponent);
	case EFVEasing::EaseInOut:  return FMath::InterpEaseInOut(0.f, 1.f, A, Exponent);
	case EFVEasing::SmoothStep: return FMath::SmoothStep(0.f, 1.f, A);
	case EFVEasing::ExpoIn:     return FMath::InterpExpoIn(0.f, 1.f, A);
	case EFVEasing::ExpoOut:    return FMath::InterpExpoOut(0.f, 1.f, A);
	case EFVEasing::ExpoInOut:  return FMath::InterpExpoInOut(0.f, 1.f, A);
	case EFVEasing::CircIn:     return FMath::InterpCircularIn(0.f, 1.f, A);
	case EFVEasing::CircOut:    return FMath::InterpCircularOut(0.f, 1.f, A);
	case EFVEasing::CircInOut:  return FMath::InterpCircularInOut(0.f, 1.f, A);
	default:                    return A;
	}
}

EFVEasing UFVEasingLibrary::EasingFromName(FName Name)
{
	const UEnum* Enum = StaticEnum<EFVEasing>();
	const int64 Value = Enum->GetValueByNameString(Name.ToString());
	return Value == INDEX_NONE ? EFVEasing::Linear : static_cast<EFVEasing>(Value);
}
