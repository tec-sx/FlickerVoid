// Fill out your copyright notice in the Description page of Project Settings.

#include "FVAICharacterController.h"

#include "FVAIConfigData.h"
#include "FVCoreTags.h"
#include "FVStateTreeAIComponent.h"
#include "Actors/FVAICharacter.h"
#include "StateTree.h"
#include "Components/BoxComponent.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Prediction.h"
#include "Perception/AISenseConfig_Sight.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAICharacterController)

AFVAICharacterController::AFVAICharacterController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAttachToPawn = true;
	
    StateTreeAIComponent = CreateDefaultSubobject<UFVStateTreeAIComponent>(TEXT("State Tree AI Component"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));
	   //
    // SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    // SightConfig->SightRadius = 1000.0f;
    // SightConfig->LoseSightRadius = 1500.0f;
    // SightConfig->PeripheralVisionAngleDegrees = 35.0f;
    // SightConfig->SetMaxAge(5.f);
    // SightConfig->PointOfViewBackwardOffset = 260.0f;
    // SightConfig->NearClippingRadius = 200.0f;
    // SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.0f;
    // SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    // SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    // SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
    //
    // PerceptionComponent->ConfigureSense(*SightConfig);
	   //
    // HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    // HearingConfig->HearingRange = 1200.f;
    // HearingConfig->SetMaxAge(3.f);
    // HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    // HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    // HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    // PerceptionComponent->ConfigureSense(*HearingConfig);
    //
    // // Add damage perception
    // DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    // PerceptionComponent->ConfigureSense(*DamageConfig);
    //
    // // Add prediction sense
    // PredictionConfig = CreateDefaultSubobject<UAISenseConfig_Prediction>(TEXT("PredictionConfig"));
    // PredictionConfig->SetMaxAge(1.0f); // How long the prediction lasts
    // PredictionConfig->SetStartsEnabled(true); // Start enabled
    // PerceptionComponent->ConfigureSense(*PredictionConfig);
	
	StateTreeAIComponent->SetStartLogicAutomatically(false);
}

void AFVAICharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	PossesedCharacter = Cast<AFVAICharacter>(InPawn);
	if (!PossesedCharacter)
	{
		FV_LOG_WARNING(LogFVAI, "AFVAICharacterController::OnPossess - InPawn is not a FVAICharacter!");
		return;
	}
	
	
	TArray<UAISenseConfig*> SensesConfig = PossesedCharacter->AIConfig->SensesConfig;
	for (UAISenseConfig* Config : SensesConfig)
	{
		PerceptionComponent->ConfigureSense(*Config);
	}
	
	PerceptionComponent->SetDominantSense(PossesedCharacter->AIConfig->DominantSense);
	
	// Bind perception events
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AFVAICharacterController::OnTargetPerceptionUpdated);
	PerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &AFVAICharacterController::OnTargetPerceptionForgotten);
}

void AFVAICharacterController::OnUnPossess()
{
	PerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(this, &AFVAICharacterController::OnTargetPerceptionUpdated);
	PerceptionComponent->OnTargetPerceptionForgotten.RemoveDynamic(this, &AFVAICharacterController::OnTargetPerceptionForgotten);
	
	PossesedCharacter = nullptr;
	Super::OnUnPossess();
}

void AFVAICharacterController::ForgetPerceptionActor(AActor* ActorToForget)
{
	if (!ActorToForget)
	{
		FV_LOG_WARNING(LogFVAI, "ForgetPerceptionActor: ActorToForget is nullptr.");
		return;
	}

	UAIPerceptionComponent* PC = GetPerceptionComponent();
	if (PC)
	{
		PC->ForgetActor(ActorToForget);
	}
	else
	{
		FV_LOG_WARNING(LogFVAI,"ForgetPerceptionActor: PerceptionComponent is null.");
	}
}

void AFVAICharacterController::ForgetPerceptionActors(const TArray<AActor*>& ActorsToForget)
{
	UAIPerceptionComponent* PC = GetPerceptionComponent();
	if (!PC)
	{
		FV_LOG_WARNING(LogFVAI, "ForgetPerceptionActors: PerceptionComponent is null.");
		return;
	}

	if (ActorsToForget.Num() == 0)
	{
		FV_LOG_WARNING(LogFVAI, "ForgetPerceptionActors: No actors provided.");
		return;
	}

	for (AActor* Actor : ActorsToForget)
	{
		if (Actor)
		{
			PC->ForgetActor(Actor);
		}
		else
		{
			FV_LOG_WARNING(LogFVAI, "ForgetPerceptionActors: Encountered null actor, skipping.");
		}
	}
}

TArray<AActor*> AFVAICharacterController::GetAllDamageSensedActors() const
{
	TArray<AActor*> SensedActors;
	if (PerceptionComponent)
	{
		PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Damage::StaticClass(), SensedActors);
	}
	
	SensedActors = SensedActors.FilterByPredicate([ this ] (AActor* Actor)
	{
		return Actor && GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
	});
    
	return SensedActors;
}

TArray<AActor*> AFVAICharacterController::GetAllHeardActors() const
{
	TArray<AActor*> SensedActors;
	if (PerceptionComponent)
	{
		PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Hearing::StaticClass(), SensedActors);
	}

	// Filter out non-hostiles
	SensedActors = SensedActors.FilterByPredicate([this](AActor* Actor)
	{
		return Actor && GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
	});

	return SensedActors;
}

TArray<AActor*> AFVAICharacterController::GetAllSeenActors() const
{
	TArray<AActor*> SensedActors;
	if (PerceptionComponent)
	{
		PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SensedActors);
	}

	// Filter out non-hostiles
	SensedActors = SensedActors.FilterByPredicate([ this ] (AActor* Actor)
	{
		return Actor && GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
	});

	return SensedActors;
}

void AFVAICharacterController::ReportDamageEvent(AActor* DamagedActor, AActor* InstigatorActor, float DamageAmount)
{
	UWorld* World = GetWorld();
	if (!World || !DamagedActor)
	{
		FV_LOG_WARNING(LogFVAI, "ReportDamageEvent: Missing world or damaged actor.");
		return;
	}
	
	const FVector EventLocation = DamagedActor->GetActorLocation();
	const FVector InstigatorLocation = InstigatorActor ? InstigatorActor->GetActorLocation() : EventLocation;
	
	UAISense_Damage::ReportDamageEvent(
		World,
		DamagedActor,
		InstigatorActor,
		DamageAmount,
		EventLocation,
		InstigatorLocation
	);
}

void AFVAICharacterController::ReportNoiseEvent(AActor* NoiseInstigator, FVector NoiseLocation, float Loudness, float MaxRange)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		FV_LOG_WARNING(LogFVAI, "ReportNoiseEvent: No valid World context.");
		return;
	}
	
	UAISense_Hearing::ReportNoiseEvent(
		World,
		NoiseLocation,
		Loudness,
		NoiseInstigator,
		MaxRange
	);
}

void AFVAICharacterController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }
	
	if (GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
	{
		return;
	}
	
    static const FAISenseID SightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());
    static const FAISenseID HearingID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());
    static const FAISenseID DamageID = UAISense::GetSenseID(UAISense_Damage::StaticClass());
	
    FString SenseName;
    if (Stimulus.Type == SightID)
        SenseName = TEXT("Sight");
    else if (Stimulus.Type == HearingID)
        SenseName = TEXT("Hearing");
    else if (Stimulus.Type == DamageID)
        SenseName = TEXT("Damage");
    else {
        SenseName = TEXT("Unknown");
    }
	
    AActor* SensedActor = Actor;
    if (!SensedActor)
    {
        FV_LOG(LogFVAI, Verbose, "Sensed Actor %s is not a valid actor.", *GetNameSafe(Actor));
        return;
    }

    CurrentStimulusSenseType = EFVStimulusSenseType::Unknown;
	
    if (Stimulus.Type == DamageID)
    {
        CurrentStimulusSenseType = EFVStimulusSenseType::Damage;
        if (Stimulus.WasSuccessfullySensed())
        {
            OnDamageStimulusDetected.Broadcast(SensedActor, Stimulus);
        }
    }
    else if (Stimulus.Type == SightID)
    {
        CurrentStimulusSenseType = EFVStimulusSenseType::Sight;
        // static FTimerHandle DistanceCheckTimer;
        
    	if (Stimulus.WasSuccessfullySensed())
        {
            OnSightStimulusDetected.Broadcast(SensedActor, Stimulus);
        }
        else
        {
            OnSightStimulusForgotten.Broadcast(SensedActor);
        }
    }
    else if (Stimulus.Type == HearingID)
    {
        CurrentStimulusSenseType = EFVStimulusSenseType::Hearing;
        if (Stimulus.WasSuccessfullySensed())
        {
            OnHearingStimulusDetected.Broadcast(SensedActor, Stimulus);
        }
        else
        {
            OnHearingStimulusForgotten.Broadcast(SensedActor);
        }
    }
}

void AFVAICharacterController::OnTargetPerceptionForgotten(AActor* Actor)
{
	OnSightStimulusForgotten.Broadcast(Actor);
	OnHearingStimulusForgotten.Broadcast(Actor);
}

ETeamAttitude::Type AFVAICharacterController::GetTeamAttitudeTowards(const AActor& Other) const
{
	// Use this to recognize factions
	if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(&Other))
	{
		FGenericTeamId OtherTeamId = TeamAgent->GetGenericTeamId();

		// Player = 1, Enemy= 2
		if (OtherTeamId == FGenericTeamId(1))
		{
			return ETeamAttitude::Hostile;
		}
		else if (OtherTeamId == FGenericTeamId(2))
		{
			return ETeamAttitude::Friendly;
		}
	}

	return ETeamAttitude::Neutral; 
}

