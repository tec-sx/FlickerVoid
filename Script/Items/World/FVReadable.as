// A note, letter, book or map in the world; knowledge only, never an inventory item. Reading it teaches its knowledge; with secrets it must be investigated (examined) first.
class AFVReadable : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	UStaticMeshComponent Mesh;
	default Mesh.ComponentTags.Add(n"Detectable");

	UPROPERTY(DefaultComponent)
	UFVInteractableComponent Interactable;

	UPROPERTY(DefaultComponent)
	UFVInteractableHighlightComponent Highlight;
	default Highlight.HighlightSetup.HighlightType = EFVHighlightType::PostProcessing;
	default Highlight.HighlightSetup.StencilID = 133;

	UPROPERTY(DefaultComponent)
	UFVSaveableComponent Saveable;

	UPROPERTY(DefaultComponent)
	UFVExamineSecretsComponent Secrets;

	UPROPERTY(EditAnywhere, Category = "Readable", meta = (ToolTip = "The note itself (Kind Document); learned when read or when all its secrets are found."))
	UFVKnowledgeDefinition Knowledge;

	UPROPERTY(EditAnywhere, Category = "Readable", meta = (ToolTip = "The note is taken away once read."))
	bool bHideWhenRead = false;

	UPROPERTY(SaveGame)
	bool bRead = false;

	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
		Saveable.OnActorDataLoaded.AddUFunction(this, n"RefreshPresence");
		RefreshPresence();
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Unsolved and some of its secrets aren't found yet: only investigating it helps."))
	bool NeedsInvestigation() const
	{
		return !IsSolved() && Secrets.HasSecrets() && !Secrets.AllFound();
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Its knowledge is known."))
	bool IsSolved() const
	{
		return Knowledge != nullptr && FVKnowledge::Knows(Knowledge);
	}

	UFUNCTION(meta = (ToolTip = "Learns the knowledge and broadcasts Message.Item.Read (the reader screen opens on it)."))
	void MarkRead(AActor Reader)
	{
		if (Knowledge != nullptr)
		{
			FVKnowledge::Learn(Knowledge);
		}

		bRead = true;

		FFVItemReadMessage Message;
		Message.Reader = Reader;
		Message.Knowledge = Knowledge;
		Message.Source = this;

		UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Message_Item_Read, Message);

		RefreshPresence();
	}

	UFUNCTION()
	private void RefreshPresence()
	{
		bool bPresent = !(bHideWhenRead && bRead);

		SetActorHiddenInGame(!bPresent);
		SetActorEnableCollision(bPresent);
	}
}
