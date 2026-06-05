struct FFVCheckLedgeResult
{
    bool bHasFrontLedge;
    FVector FrontLocation;
    FVector FrontLedgeNormal;

    bool bHasBackLedge;
    FVector BackLocation;
    FVector BackLedgeNormal;

    FFVResult ValidationResult;
};

class AFVTraversable : AActor
{
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent Root;

    UPROPERTY(DefaultComponent)
    UStaticMeshComponent Mesh;

    UPROPERTY(DefaultComponent)
    USplineComponent LedgeFront;

    UPROPERTY(DefaultComponent)
    USplineComponent LedgeLeft;

    UPROPERTY(DefaultComponent)
    USplineComponent LedgeBack;

    UPROPERTY(DefaultComponent)
    USplineComponent LedgeRight;

    UPROPERTY(Category = "Configuration")
    UFVTraversableConfig Config;

    TArray<USplineComponent> Ledges;
    TMap<USplineComponent, USplineComponent> OppositeLedges;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        Ledges.Empty();
        OppositeLedges.Empty();
    
        // Validate configuration
        if (Config == nullptr)
        {
            Warning("UFVTraversableConfig is null on " + GetName() + ". Using default values.");
            Config = NewObject(this, UFVTraversableConfig);
        }

        Ledges.Add(LedgeFront);
        Ledges.Add(LedgeLeft);
        Ledges.Add(LedgeBack);
        Ledges.Add(LedgeRight);

        OppositeLedges.Add(LedgeFront, LedgeBack);
        OppositeLedges.Add(LedgeLeft, LedgeRight);
        OppositeLedges.Add(LedgeBack, LedgeFront);
        OppositeLedges.Add(LedgeRight, LedgeLeft);
    }

    FFVCheckLedgeResult GetLedgeTransforms(FVector HitLocation, FVector PlayerLocation)
    {
        FFVCheckLedgeResult Result;
        Result.ValidationResult = FVResultHelpers::Success();

        // Validate we have ledges configured
        Result.ValidationResult = FVValidation::ValidateArrayNotEmpty(Ledges.Num(), "Ledges");
        if (!Result.ValidationResult.bSuccess)
        {
            Result.ValidationResult.ErrorMessage += " on " + GetName();
            Warning(Result.ValidationResult.ErrorMessage);
            return Result;
        }

        USplineComponent ClosestLedge = FindClosestLedge(PlayerLocation);
        
        // Validate closest ledge found
        Result.ValidationResult = FVValidation::ValidateComponent(ClosestLedge, "ClosestLedge");
        if (!Result.ValidationResult.bSuccess)
        {
            Result.ValidationResult.ErrorMessage = "Failed to find closest ledge on " + GetName();
            Warning(Result.ValidationResult.ErrorMessage);
            return Result;
        }

        // Validate ledge length
        if (ClosestLedge.GetSplineLength() < Config.MinLedgeWidth)
        {
            Result.ValidationResult = FVResultHelpers::Failure("Closest ledge is too short: " + ClosestLedge.GetSplineLength() + " < " + Config.MinLedgeWidth);
            return Result;
        }

        FVector ClosestLocation = ClosestLedge.FindLocationClosestToWorldLocation(HitLocation, ESplineCoordinateSpace::Local);
        float DistanceToLocation = ClosestLedge.GetDistanceAlongSplineAtLocation(ClosestLocation, ESplineCoordinateSpace::Local);
        DistanceToLocation = Math::Clamp(
			DistanceToLocation,
			Config.MinLedgeWidth / 2,
			ClosestLedge.GetSplineLength() - Config.MinLedgeWidth / 2);

        FTransform ClosestLocationTransform = ClosestLedge.GetTransformAtDistanceAlongSpline(DistanceToLocation, ESplineCoordinateSpace::World);
        
        Result.bHasFrontLedge = true;
        Result.FrontLocation = ClosestLocationTransform.GetLocation();
        Result.FrontLedgeNormal = ClosestLocationTransform.GetRotation().GetUpVector();

        USplineComponent OppositeLedge = OppositeLedges[ClosestLedge];

        // Validate opposite ledge
        Result.ValidationResult = FVValidation::ValidateComponent(OppositeLedge, "OppositeLedge");
        if (!Result.ValidationResult.bSuccess)
        {
            Result.ValidationResult.ErrorMessage = "No opposite ledge found for " + ClosestLedge.GetName();
            return Result;
        }

        // Validate opposite ledge length
        if (OppositeLedge.GetSplineLength() < Config.MinLedgeWidth)
        {
            Result.ValidationResult = FVResultHelpers::Failure("Opposite ledge is too short: " + OppositeLedge.GetSplineLength() + " < " + Config.MinLedgeWidth);
            return Result;
        }

        FTransform OppositeLocationTransform = OppositeLedge.FindTransformClosestToWorldLocation(Result.FrontLocation, ESplineCoordinateSpace::World);

        Result.bHasBackLedge = true;
        Result.BackLocation = OppositeLocationTransform.GetLocation();
        Result.BackLedgeNormal = OppositeLocationTransform.GetRotation().GetUpVector();

        return Result;
    }

    USplineComponent FindClosestLedge(FVector PlayerLocation)
    {
        if (Ledges.Num() == 0)
        {
            Warning("No ledges available on " + GetName());
            return nullptr;
        }

        float ClosestDistance = 0.f;
        int ClosestIndex = 0;

        for (int i = 0; i < Ledges.Num(); i++)
        {
            USplineComponent Ledge = Ledges[i];

            if (Ledge == nullptr)
            {
                Warning("Ledge at index " + i + " is null on " + GetName());
                continue;
            }

            FVector ClosestLocation = Ledge.FindLocationClosestToWorldLocation(PlayerLocation, ESplineCoordinateSpace::World);
            FVector UpVector = Ledge.FindUpVectorClosestToWorldLocation(PlayerLocation, ESplineCoordinateSpace::World);
            FVector FinalLocation = ClosestLocation + UpVector * Config.LedgeNormalOffset;
            float Distance = FinalLocation.Distance(PlayerLocation);

            if(Distance < ClosestDistance || i == 0)
            {
                ClosestDistance = Distance;
                ClosestIndex = i;
            }
        }

        return Ledges[ClosestIndex];
    }
}