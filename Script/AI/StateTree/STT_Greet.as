class USTT_Greet : UStateTreeTaskBlueprintBase
{
    UPROPERTY(BlueprintReadOnly, Category = Context, Meta = (Context))
	TObjectPtr<AActor> NPC;

    UPROPERTY(BlueprintReadOnly, Category = Context, Meta = (Context))
	TObjectPtr<AActor> Player;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Signal)
    FGameplayTag SpeakerID;

	UFUNCTION(BlueprintOverride)
	void LatentEnterState(FStateTreeTransitionResult Transition)
	{
        UFVDialogueDatabaseSubsystem DB = Subsystem::GetGameInstanceSubsystem(UFVDialogueDatabaseSubsystem);
        UFVDialogueSubsystem DS = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);
        
        if (DB == nullptr || DS == nullptr) return;
    
        FFVDialogueSignal Signal;
        Signal.SpeakerID = SpeakerID;
        Signal.Signal     = GameplayTags::Dialogue_Signal_Greeting;
        Signal.Speaker    = Player;
        Signal.Listener   = NPC;

        DS.BroadcastSignal(Signal, false);
	}
}