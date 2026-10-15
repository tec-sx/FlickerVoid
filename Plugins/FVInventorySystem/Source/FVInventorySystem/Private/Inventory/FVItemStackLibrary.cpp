#include "Inventory/FVItemStackLibrary.h"

#include "Inventory/FVItemInstanceData.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemStackLibrary)

bool UFVItemStackLibrary::IsStolen(const FFVItemStack& Stack, FGameplayTag& OutOwner)
{
	const FFVItemInstance_Stolen* Stolen = Stack.FindData<FFVItemInstance_Stolen>();
	OutOwner = Stolen ? Stolen->Owner : FGameplayTag();
	return Stolen != nullptr;
}

int32 UFVItemStackLibrary::GetChargesLeft(const FFVItemStack& Stack)
{
	if (Stack.Item == nullptr)
	{
		return 0;
	}

	const FFVItemInstance_Charges* Left = Stack.FindData<FFVItemInstance_Charges>();
	if (Left && Left->Remaining > 0)
	{
		return Left->Remaining;
	}

	if (const FFVItemFragment_Charges* Charges = Stack.Item->FindFragment<FFVItemFragment_Charges>())
	{
		return FMath::Max(1, Charges->Charges);
	}

	return Stack.Item->IsUsable() ? 1 : 0;
}

int32 UFVItemStackLibrary::GetBasePrice(const UFVItemDefinition* Item)
{
	const FFVItemFragment_Value* Value = Item ? Item->FindFragment<FFVItemFragment_Value>() : nullptr;
	return Value ? Value->BasePrice : 0;
}

bool UFVItemStackLibrary::IsSellable(const UFVItemDefinition* Item)
{
	const FFVItemFragment_Value* Value = Item ? Item->FindFragment<FFVItemFragment_Value>() : nullptr;
	return Value && Value->bSellable;
}
