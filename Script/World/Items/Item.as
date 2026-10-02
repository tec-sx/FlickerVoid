class AItem : AActor
{
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;

    UPROPERTY(DefaultComponent)
    UStaticMeshComponent Mesh;

    UPROPERTY(DefaultComponent)
    UFVInteractableComponent Interactable;

    UPROPERTY(DefaultComponent)
    UFVInteractableHighlightComponent Highlight;
    default Highlight.HighlightSetup.HighlightType = EFVHighlightType::PostProcessing;
    default Highlight.HighlightSetup.StencilID = 133;
}