#include "Items/FVThrownItemComponent.h"

#include "Components/PrimitiveComponent.h"
#include "FVGameplayModule.h"
#include "FVGameplayTags.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Items/FVItemMessages.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVThrownItemComponent)

void UFVThrownItemComponent::Arm(AActor* InThrower, const FFVItemFragment_Throwable& InThrowable)
{
	Thrower = InThrower;
	Throwable = InThrowable;
	Body = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	if (!Body)
	{
		UE_LOG(LogFVGameplay, Warning, TEXT("%s: a thrown item needs a primitive root to hit things."), *GetNameSafe(GetOwner()));
		return;
	}

	Body->SetNotifyRigidBodyCollision(true);
	Body->OnComponentHit.AddUniqueDynamic(this, &UFVThrownItemComponent::HandleHit);
}

void UFVThrownItemComponent::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AActor* ThrownBy = Thrower.Get();
	if (OtherActor && OtherActor == ThrownBy)
	{
		return;
	}

	Body->OnComponentHit.RemoveDynamic(this, &UFVThrownItemComponent::HandleHit);

	const FVector Location = GetOwner()->GetActorLocation();
	UAISense_Hearing::ReportNoiseEvent(this, Location, Throwable.Loudness, ThrownBy, Throwable.NoiseRange, TEXT("ItemNoise"));

	FFVItemNoiseMessage Message;
	Message.Instigator = ThrownBy;
	Message.Location = Location;
	Message.Loudness = Throwable.Loudness;
	Message.Range = Throwable.NoiseRange;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(FVGameplayTags::Message_Item_Noise, Message);

	if (USoundBase* Sound = Throwable.ImpactSound.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
	}

	if (Throwable.bBreakOnImpact)
	{
		GetOwner()->Destroy();
	}
}
