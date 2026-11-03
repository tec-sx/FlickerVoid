// Hands an item and money between the player and another character for the FV Give Item flow node (Event.Item.Give payload). With a Give Scene both characters play it; else the player plays the give montage (Animation.Item.Give); else the give happens at once. Always ends with Message.Item.GiveFinished, also when the give fails or is cancelled, so the node never waits forever.
class UFVGiveItemAbility : UFVItemMontageAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Give", meta = (ToolTip = "Contextual animation scene with the roles Giver and Receiver, played when both characters have a Contextual Anim Scene Actor component. The item changes hands when the scene ends. None: the player's give montage (Animation.Item.Give), else no animation."))
	UContextualAnimSceneAsset GiveScene;

	UPROPERTY(EditDefaultsOnly, Category = "Give", meta = (ClampMin = 1, Units = "s", ToolTip = "Safety: finishes the give if the scene never reports its end."))
	float SceneTimeout = 20.0;

	default TriggerEventTag = GameplayTags::Event_Item_Give;
	default ActivationOwnedTags.AddTag(GameplayTags::Status_HandsBusy);

	private AActor Giver;
	private AActor Receiver;
	private UFVItemDefinition GivenItem;
	private int GivenQuantity = 0;
	private int GivenMoney = 0;
	private bool bAsGift = false;
	private bool bGiving = false;
	private bool bSucceeded = false;
	private bool bInScene = false;
	private bool bSceneHandsBusy = false;
	private UContextualAnimSceneActorComponent PlayerScene;
	private FTimerHandle SceneTimer;
	private UStaticMeshComponent Prop;

	// Without a payload there is nothing to give.
	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		EndAbility();
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		UFVItemEventPayload Payload = FVItem::GetItemPayload(EventData);
		AActor Player = GetAvatarActorFromActorInfo();

		if (Payload == nullptr || Payload.OtherActor == nullptr)
		{
			EndAbility();
			return;
		}

		Giver = Payload.bPlayerGives ? Player : Payload.OtherActor;
		Receiver = Payload.bPlayerGives ? Payload.OtherActor : Player;
		GivenItem = Payload.Item;
		GivenQuantity = GivenItem != nullptr ? Payload.Quantity : 0;
		GivenMoney = Payload.Money;
		bAsGift = Payload.bAsGift;
		bSucceeded = false;
		bGiving = true;

		if (!FVItem::CanGiveItem(Giver, Receiver, GivenItem, GivenQuantity, GivenMoney))
		{
			EndAbility();
			return;
		}

		if (StartScene())
		{
			return;
		}

		UAnimMontage Montage = FVItem::FindMontage(Player, GameplayTags::Animation_Item_Give);

		if (Montage == nullptr)
		{
			Transfer();
			EndAbility();
			return;
		}

		PlayItemMontage(Montage);
	}

	// Montage path: the prop shows in the player's hand while the player gives, or arrives there on Apply when the player receives.
	void OnItemAttach() override
	{
		AActor Player = GetAvatarActorFromActorInfo();

		if (Giver == Player)
		{
			MovePropTo(Player);
		}
	}

	void OnItemApply() override
	{
		Transfer();

		AActor Player = GetAvatarActorFromActorInfo();
		MovePropTo(bSucceeded && Receiver == Player ? Player : nullptr);
	}

	void OnItemRelease() override
	{
		MovePropTo(nullptr);
	}

	void OnItemAbilityEnded(bool bWasApplied) override
	{
		System::ClearAndInvalidateTimerHandle(SceneTimer);

		if (IsValid(PlayerScene))
		{
			PlayerScene.OnLeftSceneDelegate.UnbindObject(this);
		}

		PlayerScene = nullptr;
		bInScene = false;

		if (bSceneHandsBusy)
		{
			FVItemAnim::SetHandsBusy(GetAvatarActorFromActorInfo(), false);
			bSceneHandsBusy = false;
		}

		MovePropTo(nullptr);

		if (bGiving)
		{
			bGiving = false;
			BroadcastFinished();
		}

		Giver = nullptr;
		Receiver = nullptr;
		GivenItem = nullptr;
	}

	// Plays the Give Scene with both characters; false if there is none or it can't start. The prop follows the Attach, Handover and Release notifies of the player's montage in the scene.
	private bool StartScene()
	{
		UContextualAnimSceneActorComponent GiverScene = GiveScene != nullptr ? UContextualAnimSceneActorComponent::Get(Giver) : nullptr;
		UContextualAnimSceneActorComponent ReceiverScene = GiveScene != nullptr ? UContextualAnimSceneActorComponent::Get(Receiver) : nullptr;

		if (!IsValid(GiverScene) || !IsValid(ReceiverScene))
		{
			return false;
		}

		TMap<FName, FContextualAnimSceneBindingContext> Roles;
		Roles.Add(n"Giver", UContextualAnimUtilities::SceneBindingContext_MakeFromActor(Giver));
		Roles.Add(n"Receiver", UContextualAnimUtilities::SceneBindingContext_MakeFromActor(Receiver));

		FContextualAnimSceneBindings Bindings;

		if (!UContextualAnimUtilities::CreateContextualAnimSceneBindings(GiveScene, Roles, Bindings))
		{
			return false;
		}

		AActor Player = GetAvatarActorFromActorInfo();
		PlayerScene = Player == Giver ? GiverScene : ReceiverScene;

		// Check: OnLeftSceneDelegate passes the scene actor component (FContextualAnimSceneActorCompDelegate).
		PlayerScene.OnLeftSceneDelegate.AddUFunction(this, n"HandleLeftScene");
		bInScene = true;

		// Check: StartContextualAnimScene returns whether the scene started.
		bool bStarted = GiverScene.StartContextualAnimScene(Bindings);

		// The scene already ended while starting; HandleLeftScene finished the give.
		if (!bInScene)
		{
			return true;
		}

		if (!bStarted)
		{
			bInScene = false;
			PlayerScene.OnLeftSceneDelegate.UnbindObject(this);
			PlayerScene = nullptr;
			return false;
		}

		FVItemAnim::SetHandsBusy(Player, true);
		bSceneHandsBusy = true;

		ListenForSceneNotify(GameplayTags::Event_Item_Notify_Attach, n"HandleSceneAttach");
		ListenForSceneNotify(GameplayTags::Event_Item_Notify_Handover, n"HandleSceneHandover");
		ListenForSceneNotify(GameplayTags::Event_Item_Notify_Release, n"HandleSceneRelease");

		SceneTimer = System::SetTimer(this, n"HandleSceneTimeout", SceneTimeout, false);
		return true;
	}

	private void ListenForSceneNotify(FGameplayTag NotifyTag, FName Handler)
	{
		UAbilityTask_WaitGameplayEvent NotifyTask = FVItemAnim::WaitNotify(this, NotifyTag);

		NotifyTask.EventReceived.AddUFunction(this, Handler);
		NotifyTask.ReadyForActivation();
	}

	private void Transfer()
	{
		bSucceeded = FVItem::GiveItem(Giver, Receiver, GivenItem, GivenQuantity, GivenMoney);
	}

	// The item's prop in the character's right hand; none removes it.
	private void MovePropTo(AActor Character)
	{
		FVItem::DetachProp(Prop);
		Prop = nullptr;

		if (Character != nullptr && GivenItem != nullptr)
		{
			Prop = FVItem::AttachProp(Character, GivenItem, FVItem::GetHandSocket(true));
		}
	}

	private void BroadcastFinished()
	{
		FFVItemGiveMessage Message;
		Message.Giver = Giver;
		Message.Receiver = Receiver;
		Message.Item = GivenItem;
		Message.Quantity = GivenQuantity;
		Message.Money = GivenMoney;
		Message.bSucceeded = bSucceeded;
		Message.bAsGift = bAsGift;

		UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Message_Item_GiveFinished, Message);
	}

	UFUNCTION()
	private void HandleLeftScene(UContextualAnimSceneActorComponent SceneActorComponent)
	{
		FinishScene();
	}

	UFUNCTION()
	private void HandleSceneTimeout()
	{
		FinishScene();
	}

	private void FinishScene()
	{
		if (!bInScene)
		{
			return;
		}

		bInScene = false;
		Transfer();
		EndAbility();
	}

	UFUNCTION()
	private void HandleSceneAttach(FGameplayEventData Payload)
	{
		MovePropTo(Giver);
	}

	UFUNCTION()
	private void HandleSceneHandover(FGameplayEventData Payload)
	{
		MovePropTo(Receiver);
	}

	UFUNCTION()
	private void HandleSceneRelease(FGameplayEventData Payload)
	{
		MovePropTo(nullptr);
	}
}
