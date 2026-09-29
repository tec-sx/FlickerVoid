#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Widgets/Input/SSearchBox.h"

class FLICKERVOIDCOREEDITOR_API SFVFactSearchBox : public SSearchBox
{
public:
	DECLARE_DELEGATE_OneParam(FOnSaveSearchClicked, const FText&)

	SLATE_BEGIN_ARGS(SFVFactSearchBox)
			: _InitialText(),
			  _OnTextChanged()
		{ }

		SLATE_ATTRIBUTE(FText, InitialText)
		SLATE_EVENT(FOnTextChanged, OnTextChanged)
		SLATE_EVENT(FOnSaveSearchClicked, OnSaveSearchClicked)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FOnSaveSearchClicked OnSaveSearchClicked;
};
