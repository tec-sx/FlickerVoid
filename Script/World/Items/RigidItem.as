class ARigidItem : AActor
{
    UPROPERTY(DefaultComponent)
    UFVInteractableComponent Interactable;
    default Interactable.Type = GameplayTags::Interactable_Item;
    default Interactable.DetectionRadius = 120.f;
    default Interactable.FocusComponentTag = n"Interactable";
    
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;

    UPROPERTY(DefaultComponent)
    UStaticMeshComponent Mesh;
    default Mesh.CollisionProfileName = CollisionProfile::PhysicsActor;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        Interactable.InteractionEnded.AddUFunction(this, n"OnInteractionExecuted");
    }

    UFUNCTION()
    void OnInteractionExecuted(const FGameplayTag&in ActionTag, UFVInteractorComponent Interactor, bool bSuccess)
    {
        Mesh.SimulatePhysics = true;
        Mesh.AddForce(FVector::ForwardVector);
    }
}