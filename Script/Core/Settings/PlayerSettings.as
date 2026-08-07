UCLASS(Config = Game, DefaultConfig, Meta = (DisplayName = Player))
class UPlayerSettings : UDeveloperSettings
{
    UPROPERTY(Config, EditAnywhere, Category = UI)
    TSoftClassPtr<UDialogueWidget> DialogueWidgetClass;

    UPROPERTY(Config, EditAnywhere, Category = UI)
    TSoftClassPtr<UFVInteractionPromptWidget> InteractionPromptWidgetClass;
}