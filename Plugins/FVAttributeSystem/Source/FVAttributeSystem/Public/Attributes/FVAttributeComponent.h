#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Attributes/FVAttributeDefinition.h"
#include "FVAttributeComponent.generated.h"

UENUM(BlueprintType)
enum class EFVAttributeModifierOp : uint8
{
	Add,
	Multiply,
	Override
};

/** A temporary change to one attribute, keyed by the Source that applied it (an outfit, a status, a quest). */
USTRUCT(BlueprintType)
struct FVATTRIBUTESYSTEM_API FFVAttributeModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	EFVAttributeModifierOp Op = EFVAttributeModifierOp::Add;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	FGameplayTag Source;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnAttributeChanged, const UFVAttributeDefinition*, Attribute, float, OldValue, float, NewValue);

/**
 * Holds an actor's attribute values. Base values are saved; modifiers are rebuilt at runtime by
 * whoever owns them (equipment, statuses, quests) and removed by their Source tag.
 */
UCLASS(ClassGroup = FV, meta = (BlueprintSpawnableComponent))
class FVATTRIBUTESYSTEM_API UFVAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVAttributeComponent();

	static UFVAttributeComponent* Find(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Attributes", meta = (DisplayName = "Get FV Attribute Component"))
	static UFVAttributeComponent* Get(const AActor* Actor) { return Find(Actor); }

	/** Final value: base, then Add, then Multiply, with Override winning. */
	UFUNCTION(BlueprintPure, Category = "FV|Attributes")
	float GetValue(const UFVAttributeDefinition* Attribute) const;

	UFUNCTION(BlueprintPure, Category = "FV|Attributes")
	float GetBaseValue(const UFVAttributeDefinition* Attribute) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Attributes")
	void SetBaseValue(const UFVAttributeDefinition* Attribute, float Value);

	UFUNCTION(BlueprintCallable, Category = "FV|Attributes")
	void ModifyBaseValue(const UFVAttributeDefinition* Attribute, float Delta);

	UFUNCTION(BlueprintCallable, Category = "FV|Attributes")
	void ApplySet(const UFVAttributeSetDefinition* Set);

	UFUNCTION(BlueprintCallable, Category = "FV|Attributes")
	void AddModifier(const FFVAttributeModifier& Modifier);

	UFUNCTION(BlueprintCallable, Category = "FV|Attributes")
	void RemoveModifiersFromSource(FGameplayTag Source);

	UFUNCTION(BlueprintPure, Category = "FV|Attributes")
	TArray<UFVAttributeDefinition*> GetKnownAttributes() const;

	UPROPERTY(BlueprintAssignable, Category = "FV|Attributes")
	FFVOnAttributeChanged OnAttributeChanged;

protected:
	virtual void BeginPlay() override;

	/** Starting values; the set is applied first, then these entries override it. */
	UPROPERTY(EditAnywhere, Category = "Attributes")
	TObjectPtr<const UFVAttributeSetDefinition> StartingSet;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (TitleProperty = "Attribute"))
	TArray<FFVAttributeValue> StartingValues;

private:
	void Recompute(const UFVAttributeDefinition* Attribute);

	UFUNCTION()
	void HandleActorDataLoaded();

	UPROPERTY(SaveGame)
	TMap<TObjectPtr<const UFVAttributeDefinition>, float> BaseValues;

	UPROPERTY(Transient)
	TMap<TObjectPtr<const UFVAttributeDefinition>, float> CurrentValues;

	UPROPERTY(Transient)
	TArray<FFVAttributeModifier> Modifiers;
};
