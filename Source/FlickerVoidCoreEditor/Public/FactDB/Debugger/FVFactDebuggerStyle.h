#pragma once

#include "CoreMinimal.h"

struct FLICKERVOIDCOREEDITOR_API FFVFactDebuggerStyle final : public FSlateStyleSet
{
	static void Register();
	static void Unregister();

	static FFVFactDebuggerStyle& Get();
	virtual FName& GetStyleSetName() const override;

	FFVFactDebuggerStyle();
	virtual ~FFVFactDebuggerStyle() override = default;

private:
	static TSharedPtr<FFVFactDebuggerStyle> StyleInstance;
};
