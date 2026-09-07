class ARigidItem : AActor
{
    UPROPERTY(DefaultComponent)
    UInteractableComponent Interactable;
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
        Interactable.OnInteractionExecuted.AddUFunction(this, n"OnInteractionExecuted");
    }

    UFUNCTION()
    private void OnInteractionExecuted(const FGameplayTag&in ActionTag, UInteractorComponent Interactor)
    {
        Mesh.SimulatePhysics = true;
        Mesh.AddForce(FVector::ForwardVector);
    }
}