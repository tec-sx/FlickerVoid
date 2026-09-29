#include "FactDB/FVFactEnumRegistry.h"

FFVFactEnumRegistry& FFVFactEnumRegistry::Get()
{
	static FFVFactEnumRegistry Instance;
	return Instance;
}

bool FFVFactEnumRegistry::RegisterEnum(UEnum* Enum)
{
	if (!Enum)
	{
		return false;
	}

	RegisteredEnums.Add(Enum->GetFName(), Enum);
	return true;
}

UEnum* FFVFactEnumRegistry::FindEnum(FName EnumName) const
{
	UEnum* const* Found = RegisteredEnums.Find(EnumName);
	return Found ? *Found : nullptr;
}

TArray<FName> FFVFactEnumRegistry::GetRegisteredEnumNames() const
{
	TArray<FName> Names;
	RegisteredEnums.GenerateKeyArray(Names);
	return Names;
}
