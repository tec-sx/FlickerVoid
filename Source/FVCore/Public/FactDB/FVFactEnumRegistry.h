#pragma once

#include "CoreMinimal.h"
#include "Misc/DelayedAutoRegister.h"

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

// StaticEnum<EnumType>() must not run during static initialization (the reflection system may not be
// ready yet), so the actual registration is deferred until after the engine has finished initializing.
#define FV_REGISTER_FACT_ENUM(EnumType) \
	static FDelayedAutoRegisterHelper UE_JOIN(GFactEnumAutoRegister_, __LINE__)( \
		EDelayedRegisterRunPhase::EndOfEngineInit, \
		[]() { FFVFactEnumRegistry::Get().RegisterEnum(StaticEnum<EnumType>()); })
