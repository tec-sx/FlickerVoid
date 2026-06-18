enum EFoliageType
{
    Tree,
    Bush,
    Trunk
};

enum EFoliageElementType
{
    Trunk,
    Crown,
    Bush
};

struct FFoliageElementData
{
    UPROPERTY()
    UStaticMesh Mesh;

    UPROPERTY()
    EFoliageElementType Type;
}

class AFoliageActor : AActor
{
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;

    UPROPERTY()
    FVector Size = FVector::OneVector;

    UPROPERTY()
    EFoliageType FoliageType;

    UPROPERTY()
    TArray<FFoliageElementData> FoliageElements;

    UFUNCTION(BlueprintOverride)
	void ConstructionScript()
	{
		switch (FoliageType)
        {
            case EFoliageType::Tree:
                GenerateTree();
                break;
            case EFoliageType::Bush:
                GenerateBush();
                break;
            default:
                break;
        }
	}

    void GenerateTree()
	{
		auto TrunkMesh = UStaticMeshComponent::Create(this);
        auto CrownMesh = UStaticMeshComponent::Create(this);

        for(int i = 0; i < FoliageElements.Num(); i++)
        {
            FFoliageElementData Element = FoliageElements[i];
            
            switch (Element.Type)
            {
                case EFoliageElementType::Trunk:
                    TrunkMesh.SetStaticMesh(Element.Mesh);
                    break;
                case EFoliageElementType::Crown:
                    CrownMesh.SetStaticMesh(Element.Mesh);
                    break;
                default:
                    break;
            }
        }

        TrunkMesh.AttachToComponent(SceneRoot);
        CrownMesh.AttachToComponent(TrunkMesh);

        FVector CrownOrigin = FVector(0, 0, TrunkMesh.BoundsExtent.Z * 1.5);
        CrownMesh.SetRelativeLocation(CrownOrigin);
        CrownMesh.SetRelativeScale3D(Size);
	}

    void GenerateBush()
    {
        auto BushMesh = UStaticMeshComponent::Create(this);

        for(int i = 0; i < FoliageElements.Num(); i++)
        {
            FFoliageElementData Element = FoliageElements[i];
            
            switch (Element.Type)
            {
                case EFoliageElementType::Bush:
                    BushMesh.SetStaticMesh(Element.Mesh);
                    break;
                default:
                    break;
            }
        }

        BushMesh.AttachToComponent(SceneRoot);
        BushMesh.SetRelativeScale3D(Size);
    }
}