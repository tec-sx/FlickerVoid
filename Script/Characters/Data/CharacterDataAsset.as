class UCharacterDataAsset : UPrimaryDataAsset
{
    // Identity
    UPROPERTY(EditAnywhere, Category = "Identity")
    TSubclassOf<AFVAICharacter> CharacterClass;

    UPROPERTY(EditAnywhere, Category = "Identity")
    FText Name;

    UPROPERTY(EditAnywhere, Category = "Identity")
    FGameplayTagContainer IdentityTags;

    // Logic
    UPROPERTY(EditDefaultsOnly, Category = "Logic")
    UStateTree StateTree;

    UPROPERTY(EditDefaultsOnly, Category = "Logic")
    UFlowAsset FlowAsset;

    UPROPERTY(EditDefaultsOnly, Category = "Logic")
    USUDSScript DialogueScript;

    // Cosmetics
    UPROPERTY(EditDefaultsOnly, Category = "Cosmetics")
    USkeletalMesh SkeletalMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Cosmetics")
    EAnimationMode AnimationMode = EAnimationMode::AnimationBlueprint; ;

    UPROPERTY(EditDefaultsOnly, Category = "Cosmetics", Instanced, Meta = (EditCondition = "AnimationMode == EAnimationMode::AnimationBlueprint"))
    TSubclassOf<UAnimInstance> AnimInstanceClass;

    UPROPERTY(EditDefaultsOnly, Category = "Cosmetics", Meta = (EditCondition = "AnimationMode == EAnimationMode::AnimationSingleNode"))
    UAnimationAsset AnimationAsset;

    UPROPERTY(EditDefaultsOnly, Category = "Cosmetics", Meta = (EditCondition = "AnimationMode == EAnimationMode::AnimationSingleNode"))
    bool bLoopAnimation = true;
}