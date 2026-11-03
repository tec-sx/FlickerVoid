#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Social/FVSocialTypes.h"
#include "FVSocialLibrary.generated.h"

class UFVCharacterDefinition;
class UFVFactionDefinition;
class UFVScaleDefinition;
class UFVTitleDefinition;

/**
 * The game's social rules: who belongs to which faction, disguises, what onlookers recognize and how factions treat
 * someone. Standing, notoriety, fame and relationships are scales; read and change them with FVScale.
 */
UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVSocialLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "The fame scale from the Social settings."))
	static UFVScaleDefinition* GetFameScale();

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "The global notoriety scale from the Social settings."))
	static UFVScaleDefinition* GetNotorietyScale();

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "The scale holding the player's relationship with the character, from its Social fragment."))
	static UFVScaleDefinition* GetRelationshipScale(const UFVCharacterDefinition* Character);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "Faction of an actor through its identity's Social fragment."))
	static UFVFactionDefinition* GetActorFaction(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "The actor wears something tagged under the Social settings' Disguise Root."))
	static bool IsDisguised(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "The actor carries the faction's disguise tag and both the faction's and the global notoriety are below its disguise limit."))
	static bool IsDisguisedAs(const AActor* Actor, const UFVFactionDefinition* Faction);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "The registered faction the actor currently passes as a member of, or null."))
	static UFVFactionDefinition* GetDisguiseFaction(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "Fame as an onlooker reads it: a disguise hides most of it."))
	static int32 GetRecognizedFame(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "Notoriety a faction reads on the actor: the higher of its own and the global one, less any disguise."))
	static int32 GetRecognizedNotoriety(const AActor* Actor, const UFVFactionDefinition* Faction);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "How members of the faction treat the subject: members and accepted disguises are Allied, high notoriety is Hostile, otherwise the standing band decides."))
	static EFVAttitude GetAttitude(const UFVFactionDefinition* Faction, const AActor* Subject);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "Factions registered in the Social settings."))
	static TArray<UFVFactionDefinition*> GetFactions();

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (WorldContext = "WorldContext", ToolTip = "Characters from the Social settings' Relationships list whose relationship has been set."))
	static TArray<UFVCharacterDefinition*> GetKnownCharacters(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static bool HasTitle(const UObject* WorldContext, const UFVTitleDefinition* Title);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "Relationship change a gift with these tags (its definition Tags and Category) makes on the receiver: the sum of their matching gift preferences, else their default."))
	static int32 GetGiftRelationshipChange(const AActor* Receiver, const FGameplayTagContainer& GiftTags);

	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (WorldContext = "WorldContext", ToolTip = "Changes the player's relationship with the receiver by their gift preferences. Returns the change that applied; 0 when the receiver has no relationship scale."))
	static int32 ReceiveGift(UObject* WorldContext, const AActor* Receiver, const FGameplayTagContainer& GiftTags);

	static const FFVCharacterFragment_Social* FindSocialFragment(const AActor* Actor);
};
