/**
 * Dresses a character from what it wears. Each worn item's garment, fitted to the body type, goes into the outfit's
 * Mutable instance under its slot tag; its clip tags hide the skin it covers on the body and turn on tucks.
 */
class UCharacterAppearance : UObject
{
    private USkeletalMeshComponent Body;
    private UCustomizableObjectInstance Instance;
    private UFVEquipmentComponent Equipment;
    private FGameplayTag BodyType;
    private TArray<FString> DressedSlots;
    private TArray<FGameplayTag> ShownClipTags;

    /** Copies the outfit's instance so runtime changes never touch the asset, then dresses from the equipment and follows its changes. */
    void Start(USkeletalMeshComponent InBody, UCustomizableSkeletalComponent Outfit, UFVEquipmentComponent InEquipment, FGameplayTag InBodyType)
    {
        Body = InBody;
        Equipment = InEquipment;
        BodyType = InBodyType;

        UCustomizableObjectInstance Source = nullptr;

        if (Outfit != nullptr)
        {
            Source = Outfit.GetCustomizableObjectInstance();
        }

        if (Source != nullptr)
        {
            Instance = Source.Clone();
            Outfit.SetCustomizableObjectInstance(Instance);
        }

        if (Equipment != nullptr)
        {
            Equipment.OnEquipmentChanged.AddUFunction(this, n"HandleEquipmentChanged");
            Equipment.OnEquipmentReset.AddUFunction(this, n"Dress");
        }

        Dress();
    }

    FGameplayTag GetBodyType() const
    {
        return BodyType;
    }

    /** Switches the body type and refits every worn garment. NewBody, when set, replaces the body mesh. */
    void SetBodyType(FGameplayTag NewBodyType, USkeletalMesh NewBody)
    {
        BodyType = NewBodyType;

        if (NewBody != nullptr && Body != nullptr)
        {
            Body.EmptyOverrideMaterials();
            Body.SetSkeletalMeshAsset(NewBody);
            ShownClipTags.Empty();
        }

        Dress();
    }

    /** Rebuilds the outfit from everything worn and sends one Mutable update. */
    UFUNCTION()
    void Dress()
    {
        TArray<FString> Slots;
        TArray<FGameplayTag> ClipTags;

        if (Equipment != nullptr)
        {
            for (FFVEquippedItem Worn : Equipment.GetAllEquipped())
            {
                FString Slot = Worn.Slot.ToString();

                if (Instance != nullptr && Instance.ContainsSkeletalMeshParameter(Slot))
                {
                    USkeletalMesh Garment = FVItem::LoadAppearanceMesh(Worn.Item, BodyType);

                    if (Garment != nullptr)
                    {
                        Instance.SetSkeletalMeshParameterSelectedOption(Slot, Garment, -1);
                        Slots.Add(Slot);
                    }
                }

                FFVItemFragment_Appearance Appearance;

                if (FVItem::GetAppearanceFragment(Worn.Item, Appearance))
                {
                    for (FGameplayTag Tag : Appearance.ClipTags)
                    {
                        ClipTags.AddUnique(Tag);
                    }
                }
            }
        }

        if (Instance != nullptr)
        {
            for (FString Slot : DressedSlots)
            {
                if (!Slots.Contains(Slot))
                {
                    Instance.SetDefaultValue(Slot);
                }
            }
        }

        for (FGameplayTag Tag : ShownClipTags)
        {
            if (!ClipTags.Contains(Tag))
            {
                SetClip(Tag, false);
            }
        }

        for (FGameplayTag Tag : ClipTags)
        {
            SetClip(Tag, true);
        }

        DressedSlots = Slots;
        ShownClipTags = ClipTags;

        if (Instance != nullptr)
        {
            Instance.UpdateSkeletalMeshAsync(false, true);
        }
    }

    UFUNCTION()
    private void HandleEquipmentChanged(FGameplayTag Slot, UFVItemDefinition OldItem, UFVItemDefinition NewItem)
    {
        Dress();
    }

    // A clip tag is a bool parameter in the instance (body clipping meshes, tucks) and a scalar on the body's materials (skin mask).
    private void SetClip(FGameplayTag Tag, bool bOn)
    {
        FString ParameterName = Tag.ToString();

        if (Instance != nullptr && Instance.ContainsBoolParameter(ParameterName))
        {
            Instance.SetBoolParameterSelectedOption(ParameterName, bOn);
        }

        if (Body != nullptr)
        {
            float Mask = bOn ? 1.0 : 0.0;
            Body.SetScalarParameterValueOnMaterials(Tag.GetTagName(), Mask);
        }
    }
}
