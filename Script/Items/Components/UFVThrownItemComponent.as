// Added to a thrown item by the throw ability: its first hit makes noise for AI hearing and the game (Message.Item.Noise), plays the impact sound and breaks a fragile item.
class UFVThrownItemComponent : UActorComponent
{
	private AActor Thrower;
	private FFVItemFragment_Throwable Throwable;
	private UPrimitiveComponent Body;

	UFUNCTION(meta = (ToolTip = "Starts listening for the first hit of the owner's root body; the throwable fragment gives the noise and the impact."))
	void Arm(AActor InThrower, FFVItemFragment_Throwable InThrowable)
	{
		Thrower = InThrower;
		Throwable = InThrowable;
		Body = Cast<UPrimitiveComponent>(GetOwner().RootComponent);

		if (Body == nullptr)
		{
			return;
		}

		Body.SetNotifyRigidBodyCollision(true);
		Body.OnComponentHit.AddUFunction(this, n"HandleHit");
	}

	UFUNCTION()
	private void HandleHit(UPrimitiveComponent HitComponent, AActor OtherActor, UPrimitiveComponent OtherComp, FVector NormalImpulse, const FHitResult&in Hit)
	{
		if (OtherActor != nullptr && OtherActor == Thrower)
		{
			return;
		}

		Body.OnComponentHit.UnbindObject(this);

		FVector Location = GetOwner().GetActorLocation();
		UAISense_Hearing::ReportNoiseEvent(Location, Throwable.Loudness, Thrower, Throwable.NoiseRange, n"ItemNoise");

		FFVItemNoiseMessage Message;
		Message.Instigator = Thrower;
		Message.Location = Location;
		Message.Loudness = Throwable.Loudness;
		Message.Range = Throwable.NoiseRange;

		UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Message_Item_Noise, Message);

		FVItemWorld::PlaySound(Throwable.ImpactSound, Location);

		if (Throwable.bBreakOnImpact)
		{
			GetOwner().DestroyActor();
		}
	}
}
