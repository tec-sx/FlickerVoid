#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVAttributeLibrary.generated.h"

class UAbilitySystemComponent;
class UFVAttributeDefinition;

/** Reads and writes an actor's attributes on its ability system, by definition. */
UCLASS()
class FVATTRIBUTESYSTEM_API UFVAttributeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Attributes", meta = (DefaultToSelf = "Actor", ToolTip = "True when the actor's ability system has an attribute set holding this attribute."))
	static bool HasAttribute(const AActor* Actor, const UFVAttributeDefinition* Attribute);

	UFUNCTION(BlueprintPure, Category = "FV|Attributes", meta = (DefaultToSelf = "Actor", ToolTip = "Current value, with active Gameplay Effects applied. 0 when the actor lacks the attribute."))
	static float GetValue(const AActor* Actor, const UFVAttributeDefinition* Attribute);

	UFUNCTION(BlueprintPure, Category = "FV|Attributes", meta = (DefaultToSelf = "Actor", ToolTip = "Base value, without temporary Gameplay Effects."))
	static float GetBaseValue(const AActor* Actor, const UFVAttributeDefinition* Attribute);

	UFUNCTION(BlueprintCallable, Category = "FV|Attributes", meta = (DefaultToSelf = "Actor", ToolTip = "Permanently sets the base value. Prefer an instant Gameplay Effect when the change is content."))
	static void SetBaseValue(AActor* Actor, const UFVAttributeDefinition* Attribute, float Value);

	UFUNCTION(BlueprintCallable, Category = "FV|Attributes", meta = (DefaultToSelf = "Actor", ToolTip = "Permanently adds Delta to the base value."))
	static void AddToBaseValue(AActor* Actor, const UFVAttributeDefinition* Attribute, float Delta);

	static UAbilitySystemComponent* FindAbilitySystem(const AActor* Actor, const UFVAttributeDefinition* Attribute);
};
