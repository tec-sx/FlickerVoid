// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FVAICharacterController.generated.h"

class UBoxComponent;
enum class EFVStateTreeEvent : uint8;
class UAISenseConfig_Prediction;
class UAISenseConfig_Damage;
class UAISenseConfig_Hearing;
class UAISenseConfig_Sight;
class AFVAICharacter;
class UFVStateTreeAIComponent;
class AFVCharacter;

// Delegate for perception updates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVStimulusEventSignature, AActor*, Actor, const FAIStimulus&, Stimulus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVStimulusForgottenSignature, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVInteractionZoneEventSignature, AActor*, Actor);

UENUM(BlueprintType, meta = (DisplayName = "Stimulus Sense"))
enum class EFVStimulusSenseType : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Sight   UMETA(DisplayName = "Sight"),
	Hearing UMETA(DisplayName = "Hearing"),
	Damage  UMETA(DisplayName = "Damage")
};

/**
 * Base AI Controller for FlickerVoid characters.
 * Handles NPC behavior and takes control when player releases a character.
 */
UCLASS()
class FLICKERVOIDAI_API AFVAICharacterController : public AAIController
{
	GENERATED_BODY()

public:
	AFVAICharacterController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Returns the State Tree AI Component for this character
	// This component handles the AI behavior for the enemy character.
	UFUNCTION(BlueprintCallable, Category = "AI", meta = (DisplayName = "Get State Tree AI Component"))
	UFVStateTreeAIComponent* GetStateTreeAIComponent() const { return StateTreeAIComponent; };

	UFUNCTION(BlueprintCallable, Category = "AI", meta = (DisplayName = "Get Current Stimulus Sense Type"))
	EFVStimulusSenseType GetCurrentStimulusSenseType() const { return CurrentStimulusSenseType; };
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	void ForgetPerceptionActor(AActor* ActorToForget);
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	void ForgetPerceptionActors(const TArray<AActor*>& ActorsToForget);
	
	UFUNCTION(BlueprintCallable, Category = "AI")
    TArray<AActor*> GetAllDamageSensedActors() const;
	
	UFUNCTION(BlueprintCallable, Category = "AI")
    TArray<AActor*> GetAllHeardActors() const;
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	TArray<AActor*> GetAllSeenActors() const;
	
	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	void ReportDamageEvent(AActor* DamagedActor, AActor* InstigatorActor, float DamageAmount);
	
	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	void ReportNoiseEvent(AActor* NoiseInstigator, FVector NoiseLocation, float Loudness = 1.f, float MaxRange = 1200.f);

	/*
	 * Properties
	*/
	
	UPROPERTY(BlueprintAssignable, Category = "AI|Events", meta = (DisplayName = "On Damage Stimulus Detected"))
	FFVStimulusEventSignature OnDamageStimulusDetected;

	/** Broadcast delegates */
	UPROPERTY(BlueprintAssignable, Category = "AI|Events", meta = (DisplayName = "On Sight Stimulus Detected"))
	FFVStimulusEventSignature OnSightStimulusDetected;

	UPROPERTY(BlueprintAssignable, Category = "AI|Events", meta = (DisplayName = "On Sight Stimulus Forgotten"))
	FFVStimulusForgottenSignature OnSightStimulusForgotten;

	UPROPERTY(BlueprintAssignable, Category = "AI|Events", meta = (DisplayName = "On Hearing Stimulus Detected"))
	FFVStimulusEventSignature OnHearingStimulusDetected;

	UPROPERTY(BlueprintAssignable, Category = "AI|Events", meta = (DisplayName = "On Hearing Stimulus Forgotten"))
	FFVStimulusForgottenSignature OnHearingStimulusForgotten;
	
	UPROPERTY(BlueprintAssignable, Category = "AI|Events", meta = (DisplayName = "On Enter Interaction Range"))
	FFVInteractionZoneEventSignature OnEnterInteractionZone;
	
	UPROPERTY(BlueprintAssignable, Category = "AI|Events", meta = (DisplayName = "On Exit Interaction Range"))
	FFVInteractionZoneEventSignature OnExitInteractionZone;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Output, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> AcquiredTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Output, meta = (AllowPrivateAccess = "true"))
	EFVStateTreeEvent CurrentStateTreeState;

protected:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnTargetPerceptionForgotten(AActor* Actor);
	
	UFUNCTION()
	void HandleEnterInteractionZone(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	UFUNCTION()
	void HandleExitInteractionZone(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFVStateTreeAIComponent> StateTreeAIComponent;
	
	UPROPERTY(Transient)
	TObjectPtr<AFVAICharacter> PossesedCharacter = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<UBoxComponent> InteractionZone;
	
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;
	
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess=true))
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;
	
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Prediction> PredictionConfig;
	
	EFVStimulusSenseType CurrentStimulusSenseType = EFVStimulusSenseType::Unknown;
	// bool bIsPlayerClose = false;
	
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
};