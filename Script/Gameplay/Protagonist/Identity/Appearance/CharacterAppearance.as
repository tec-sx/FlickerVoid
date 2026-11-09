/**
 * Dresses a character from what it wears. One Mutable instance builds the body and the outfit as two components: the body
 * type is an option of the body, each worn item's garment, fitted to the body type, goes into the outfit under its slot
 * tag, and its clip tags switch on the clipping meshes that cut the skin it covers out of the body and the tucks it needs.
 */
class UCharacterAppearance : UObject
{
    private UCustomizableObjectInstance Instance;
    private UFVEquipmentComponent Equipment;
    private FGameplayTag BodyType;
    private TArray<FString> DressedSlots;
    private TArray<FGameplayTag> ShownClipTags;

    /** Copies the body's instance so runtime changes never touch the asset, gives the copy to the outfit too, then dresses from the equipment and follows its changes. */
    void Start(UCustomizableSkeletalComponent Body, UCustomizableSkeletalComponent Outfit, UFVEquipmentComponent InEquipment, FGameplayTag InBodyType)
    {
        Equipment = InEquipment;
        BodyType = InBodyType;

        UCustomizableObjectInstance Source = nullptr;

        if (Body != nullptr)
        {
            Source = Body.GetCustomizableObjectInstance();
        }

        if (Source != nullptr)
        {
            Instance = Source.Clone();
            Body.SetCustomizableObjectInstance(Instance);

            if (Outfit != nullptr)
            {
                Outfit.SetCustomizableObjectInstance(Instance);
            }
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

    /** Switches the body type and refits every worn garment. */
    void SetBodyType(FGameplayTag NewBodyType)
    {
        BodyType = NewBodyType;
        Dress();
    }

    /** Sets the body type, everything worn and the clip tags it switches on, then sends one Mutable update. */
    UFUNCTION()
    void Dress()
    {
        if (Instance == nullptr)
        {
            return;
        }

        FString BodyParameter = GameplayTags::Appearance_Body.ToString();

        if (BodyType.IsValid() && Instance.ContainsEnumParameter(BodyParameter))
        {
            FString BodyOption = BodyType.ToString();
            Instance.SetEnumParameterSelectedOption(BodyParameter, BodyOption, -1);
        }

        TArray<FString> Slots;
        TArray<FGameplayTag> ClipTags;

        if (Equipment != nullptr)
        {
            for (FFVEquippedItem Worn : Equipment.GetAllEquipped())
            {
                FString Slot = Worn.Slot.ToString();

                if (Instance.ContainsSkeletalMeshParameter(Slot))
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

        for (FString Slot : DressedSlots)
        {
            if (!Slots.Contains(Slot))
            {
                Instance.SetDefaultValue(Slot);
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

        Instance.UpdateSkeletalMeshAsync(false, true);
    }

    UFUNCTION()
    private void HandleEquipmentChanged(FGameplayTag Slot, UFVItemDefinition OldItem, UFVItemDefinition NewItem)
    {
        Dress();
    }

    // A clip tag is a toggle of the same name in the instance: its clipping meshes cut the covered skin out of the body, and its tucks clip garments.
    private void SetClip(FGameplayTag Tag, bool bOn)
    {
        FString ParameterName = Tag.ToString();

        if (Instance.ContainsBoolParameter(ParameterName))
        {
            Instance.SetBoolParameterSelectedOption(ParameterName, bOn);
        }
    }
}
