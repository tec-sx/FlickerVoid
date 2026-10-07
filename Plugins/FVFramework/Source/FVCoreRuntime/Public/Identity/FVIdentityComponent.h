#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVIdentityComponent.generated.h"

class UFVCharacterDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnIdentityChanged, UFVCharacterDefinition*, Definition);

/** Links an actor to its character definition and registers it so systems can find the actor by definition. */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVCORERUNTIME_API UFVIdentityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVIdentityComponent();

	static UFVIdentityComponent* Find(const AActor* Actor);

	/** Definition of the actor, or null when it has no identity. */
	UFUNCTION(BlueprintPure, Category = "FV|Identity")
	static UFVCharacterDefinition* GetActorDefinition(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Identity")
	UFVCharacterDefinition* GetDefinition() const { return Definition; }

	UFUNCTION(BlueprintCallable, Category = "FV|Identity")
	void SetDefinition(UFVCharacterDefinition* NewDefinition);

	UPROPERTY(BlueprintAssignable, Category = "FV|Identity")
	FFVOnIdentityChanged OnDefinitionChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TObjectPtr<UFVCharacterDefinition> Definition;
};
