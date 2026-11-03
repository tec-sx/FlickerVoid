#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVIdentitySubsystem.generated.h"

class UFVCharacterDefinition;
class UFVIdentityComponent;

/** Registry of actors with an identity, keyed by definition. */
UCLASS()
class FVCORERUNTIME_API UFVIdentitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVIdentitySubsystem* Get(const UObject* WorldContext);

	/** First registered actor with this definition. */
	UFUNCTION(BlueprintPure, Category = "FV|Identity")
	AActor* FindActor(const UFVCharacterDefinition* Definition) const;

	UFUNCTION(BlueprintPure, Category = "FV|Identity")
	TArray<AActor*> FindActors(const UFVCharacterDefinition* Definition) const;

	/** Files the component under its current definition; call Unregister before the definition changes and Register after. */
	void Register(UFVIdentityComponent* Component);
	void Unregister(UFVIdentityComponent* Component);

private:
	TMultiMap<TObjectKey<UFVCharacterDefinition>, TWeakObjectPtr<UFVIdentityComponent>> ComponentsByDefinition;
};
