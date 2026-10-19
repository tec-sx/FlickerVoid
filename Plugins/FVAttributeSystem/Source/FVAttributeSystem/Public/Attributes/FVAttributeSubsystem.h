#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FVAttributeSubsystem.generated.h"

class UFVAttributeDefinition;

/** Loads every attribute definition through the asset manager and finds them by attribute. */
UCLASS()
class FVATTRIBUTESYSTEM_API UFVAttributeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UFVAttributeSubsystem* Get(const UObject* WorldContext);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "FV|Attributes", meta = (ToolTip = "The definition describing this attribute, or null when it has none."))
	const UFVAttributeDefinition* FindDefinition(FGameplayAttribute Attribute) const;

	UFUNCTION(BlueprintPure, Category = "FV|Attributes", meta = (ToolTip = "Every attribute definition in the project, for UI lists."))
	TArray<UFVAttributeDefinition*> GetDefinitions() const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<const UFVAttributeDefinition>> Definitions;

	TMap<FGameplayAttribute, const UFVAttributeDefinition*> DefinitionsByAttribute;
};
