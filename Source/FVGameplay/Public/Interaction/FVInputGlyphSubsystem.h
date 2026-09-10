#pragma once

#include "Core/InteractionTypes.h"
#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"

#include "FVInputGlyphSubsystem.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EFVInputDevice : uint8
{
	KeyboardMouse,
	Gamepad,
};

USTRUCT(BlueprintType)
struct FInteractionKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Glyph;
};

USTRUCT(BlueprintType)
struct FFVInputGlyphEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "InputTag.Interaction"))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFVInputDevice Device = EFVInputDevice::KeyboardMouse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInteractionKeyBinding Binding;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputDeviceChanged, EFVInputDevice, NewDevice);

UCLASS(Config = Game, DefaultConfig)
class UFVInputGlyphSubsystem final : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Input|Glyph")
	bool ResolveGlyph(FGameplayTag InputTag, FInteractionKeyBinding& OutBinding) const;

	UFUNCTION(BlueprintCallable, Category = "Input|Glyph")
	void SetInputDevice(EFVInputDevice NewDevice);

	UFUNCTION(BlueprintPure, Category = "Input|Glyph")
	EFVInputDevice GetInputDevice() const { return CurrentDevice; }

	UPROPERTY(BlueprintAssignable, Category = "Input|Glyph")
	FOnInputDeviceChanged OnInputDeviceChanged;

private:
	UPROPERTY(Config, EditAnywhere, Category = "Input|Glyph")
	TArray<FFVInputGlyphEntry> Glyphs;

	EFVInputDevice CurrentDevice = EFVInputDevice::KeyboardMouse;
};
