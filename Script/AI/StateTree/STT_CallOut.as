class USTT_CallOut : UStateTreeTaskBlueprintBase
{
    UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	AFVAICharacter Actor;

    UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	AFVAICharacterController AIController;

    
    UFUNCTION(BlueprintOverride)
    void LatentEnterState(FStateTreeTransitionResult Transition)
    {
        AInteractableAICharacter InteractableCharacter = Cast<AInteractableAICharacter>(Actor);

        if (!IsValid(InteractableCharacter))
        {
             return;          
        }

        if (!IsValid(InteractableCharacter.CharacterData))
        {
            return;
        }

        // FCallOut Config = InteractableCharacter.CharacterData.CalloutConfig;

        // if (IsValid(Config.CalloutDB))
        // {

        // }
    }
}