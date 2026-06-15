struct FFVDialogueLineDisplay
{
    UPROPERTY()
    FText SpeakerId;

    UPROPERTY()
    FText Text;
}

class UFVDialogueWidget : UUserWidget
{
    UFUNCTION()
    void SetNewDialogue(FFVDialogueLineDisplay NewDialogue)
    {
         
    }
}