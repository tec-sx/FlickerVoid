#include "GameModes/FVGameState.h"
#include "FVAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVGameState)

AFVGameState::AFVGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UFVAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* AFVGameState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
