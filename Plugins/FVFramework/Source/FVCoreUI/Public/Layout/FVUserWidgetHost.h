#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "FVUserWidgetHost.generated.h"

class UOverlay;

UCLASS(NotBlueprintable)
class FVCOREUI_API UFVUserWidgetHost : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFVUserWidgetHost(const FObjectInitializer& ObjectInitializer);

	void SetContent(UUserWidget* InContent);

	UUserWidget* GetContent() const { return Content; }

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UOverlay> Root;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> Content;
};
