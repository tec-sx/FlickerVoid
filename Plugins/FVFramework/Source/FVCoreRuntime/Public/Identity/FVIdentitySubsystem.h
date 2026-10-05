#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVIdentitySubsystem.generated.h"

class UFVCharacterDefinition;
class UFVIdentityComponent;

/** Registry of actors with an identity. */
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

	void Register(UFVIdentityComponent* Component);
	void Unregister(UFVIdentityComponent* Component);

private:
	TArray<TWeakObjectPtr<UFVIdentityComponent>> Components;
};
