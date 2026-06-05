#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FVMaterialTypes.h"
#include "GameplayTagContainer.h"
#include "FVItemsTags.h"
#include "FVInventoryItemBase.generated.h"

UENUM(BlueprintType)
enum class EFVItemSize : uint8
{
	Tiny UMETA(DisplayName = "Tiny"),
	Small UMETA(DisplayName = "Small"),
	Medium UMETA(DisplayName = "Medium"),
	Large UMETA(DisplayName = "Large"),
	Giant UMETA(DisplayName = "Giant")
};

UCLASS()
class FLICKERVOIDITEMS_API UFVInventoryItemBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Identification
	// ----------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identification")
	int32 ID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identification")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identification")
	FString Description;

	// Visual
	// ----------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FFVMaterialOverride OverrideMaterial;

	// Data
	// ----------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	float BaseValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemData")
	EFVItemSize Size = EFVItemSize::Tiny;

	// Functions
	// ----------------------------------------------------------------------------------
	virtual FGameplayTag GetTag() const { return FGameplayTag(); };
};
