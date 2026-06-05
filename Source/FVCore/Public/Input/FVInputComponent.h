#pragma once

#include "EnhancedInputComponent.h"
#include "FVInputConfig.h"

#include "FVInputComponent.generated.h"

#define UE_API FLICKERVOIDCORE_API

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;

UCLASS(MinimalAPI, Config = Input)
class UFVInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UE_API void AddInputMaps(
		const UFVInputConfig* InputConfig, 
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	UE_API void RemoveInputMaps(
		const UFVInputConfig* InputConfig, 
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	void BindNativeAction(
		const UFVInputConfig* InputConfig, 
		const FGameplayTag& InputTag, 
		ETriggerEvent TriggerEvent, 
		UserClass* Object, 
		FuncType Func, 
		bool bLogIfNotFound);

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(
		const UFVInputConfig* InputConfig, 
		UserClass* Object, 
		PressedFuncType PressedFunc, 
		ReleasedFuncType ReleasedFunc, 
		TArray<uint32>& BindHandles);

	UE_API void RemoveBinds(TArray<uint32>& BindHandles);
};

template<class UserClass, typename FuncType>
inline void UFVInputComponent::BindNativeAction(const UFVInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
inline void UFVInputComponent::BindAbilityActions(const UFVInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FFVInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag).GetHandle());
			}

			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}

#undef UE_API