#include "Traversal/FVTraversable.h"

#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Traversal/FVTraversalConfig.h"

AFVTraversableNative::AFVTraversableNative()
{
PrimaryActorTick.bCanEverTick = false;
RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
Mesh->SetupAttachment(RootComponent);
LedgeFront = CreateLedge(TEXT("LedgeFront"));
LedgeLeft = CreateLedge(TEXT("LedgeLeft"));
LedgeBack = CreateLedge(TEXT("LedgeBack"));
LedgeRight = CreateLedge(TEXT("LedgeRight"));
}

USplineComponent* AFVTraversableNative::CreateLedge(FName Name)
{
USplineComponent* Spline = CreateDefaultSubobject<USplineComponent>(Name);
Spline->SetupAttachment(RootComponent);
return Spline;
}

float AFVTraversableNative::GetMinLedgeWidth() const
{
return Config ? Config->MinLedgeWidth : 60.f;
}

float AFVTraversableNative::GetLedgeNormalOffset() const
{
return Config ? Config->LedgeNormalOffset : 10.f;
}

bool AFVTraversableNative::CanTraverse(AActor* Traverser) const
{
FFVConditionContext Context;
Context.WorldContext = const_cast<AFVTraversableNative*>(this);
Context.Instigator = Traverser;
Context.Target = const_cast<AFVTraversableNative*>(this);
return Requirements.Evaluate(Context);
}

USplineComponent* AFVTraversableNative::GetOpposite(const USplineComponent* Ledge) const
{
if (Ledge == LedgeFront) { return LedgeBack; }
if (Ledge == LedgeBack) { return LedgeFront; }
if (Ledge == LedgeLeft) { return LedgeRight; }
if (Ledge == LedgeRight) { return LedgeLeft; }
return nullptr;
}

USplineComponent* AFVTraversableNative::FindClosestLedge(const FVector& Location) const
{
USplineComponent* Best = nullptr;
float BestDistance = TNumericLimits<float>::Max();
for (USplineComponent* Ledge : { LedgeFront.Get(), LedgeLeft.Get(), LedgeBack.Get(), LedgeRight.Get() })
{
if (!Ledge)
{
continue;
}
const FVector Closest = Ledge->FindLocationClosestToWorldLocation(Location, ESplineCoordinateSpace::World);
const FVector Up = Ledge->FindUpVectorClosestToWorldLocation(Location, ESplineCoordinateSpace::World);
const float Distance = FVector::Dist(Closest + Up * GetLedgeNormalOffset(), Location);
if (Distance < BestDistance)
{
BestDistance = Distance;
Best = Ledge;
}
}
return Best;
}

FFVLedgeResult AFVTraversableNative::GetLedgeTransforms(const FVector& HitLocation, const FVector& ActorLocation) const
{
FFVLedgeResult Result;
const float MinWidth = GetMinLedgeWidth();
USplineComponent* Front = FindClosestLedge(ActorLocation);
if (!Front || Front->GetSplineLength() < MinWidth)
{
return Result;
}

const FVector LocalClosest = Front->FindLocationClosestToWorldLocation(HitLocation, ESplineCoordinateSpace::Local);
const float Distance = FMath::Clamp(
Front->GetDistanceAlongSplineAtLocation(LocalClosest, ESplineCoordinateSpace::Local),
MinWidth * 0.5f,
Front->GetSplineLength() - MinWidth * 0.5f);
const FTransform FrontTransform = Front->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

Result.bHasFrontLedge = true;
Result.FrontLocation = FrontTransform.GetLocation();
Result.FrontNormal = FrontTransform.GetRotation().GetUpVector();

const USplineComponent* Back = GetOpposite(Front);
if (!Back || Back->GetSplineLength() < MinWidth)
{
return Result;
}

const FTransform BackTransform = Back->FindTransformClosestToWorldLocation(Result.FrontLocation, ESplineCoordinateSpace::World);
Result.bHasBackLedge = true;
Result.BackLocation = BackTransform.GetLocation();
Result.BackNormal = BackTransform.GetRotation().GetUpVector();
return Result;
}