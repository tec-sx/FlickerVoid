#pragma once

#include "CoreMinimal.h"

struct FLICKERVOIDCOREEDITOR_API FFVFactDebuggerStyle final : public FSlateStyleSet
{
	static void Register();
	static void Unregister();

	static FFVFactDebuggerStyle& Get();
	static FName GetStyleSetName();

	FFVFactDebuggerStyle();
	virtual ~FFVFactDebuggerStyle() override = default;

private:
	static TSharedPtr<FFVFactDebuggerStyle> StyleInstance;
};
