#pragma once

#include "CoreMinimal.h"

/**
 * Manual opt-in registry of UEnum types allowed to back Fact values.
 * Enums register themselves via FV_REGISTER_FACT_ENUM in their own .cpp,
 * so pickers like UFVFlowNode_SwitchOnFact::ValueEnumName only list relevant enums.
 */
class FLICKERVOIDCORE_API FFVFactEnumRegistry
{
public:
	static FFVFactEnumRegistry& Get();

	bool RegisterEnum(UEnum* Enum);
	UEnum* FindEnum(FName EnumName) const;
	TArray<FName> GetRegisteredEnumNames() const;

private:
	TMap<FName, UEnum*> RegisteredEnums;
};

#define FV_REGISTER_FACT_ENUM(EnumType) \
	static const bool PREPROCESSOR_JOIN(bFactEnumRegistered_, __LINE__) = FFVFactEnumRegistry::Get().RegisterEnum(StaticEnum<EnumType>())
