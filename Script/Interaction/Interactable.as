UCLASS(Abstract)
class AInteractable : AActor
{
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;

    UPROPERTY(DefaultComponent, Category = Interaction)
	UBoxComponent InteractionZone;
    default InteractionZone.ComponentTags.Add(GameplayTags::Flow_Common_InteractionZone.GetTagName());

    UPROPERTY(DefaultComponent, Category = Interaction)
    UInteractionTargetComponent InteractionTargetComponent;
}