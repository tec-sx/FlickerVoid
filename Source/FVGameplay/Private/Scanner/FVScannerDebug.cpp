#include "FVDebugHUDSubsystem.h"
#include "FVDebugUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Scanner/FVScannableComponent.h"
#include "Scanner/FVScannerComponent.h"
#include "Scanner/FVScannerSubsystem.h"

#include "FVScannerDebug.generated.h"

static TAutoConsoleVariable<bool> CVarScannerDebugHUD(
	TEXT("FVCvar.Scanner.Debug.HUD"),
	false,
	TEXT("Show scan mode, the focused scannable and scan progress on screen."));

UCLASS()
class UFVScannerDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override { return CVarScannerDebugHUD.GetValueOnGameThread(); }
	virtual FString GetTitle() const override { return TEXT("Scanner"); }

	virtual void CollectLines(TArray<FString>& OutLines) const override
	{
		const UFVScannerComponent* Scanner = UFVScannerComponent::Find(FVDebug::GetPlayerPawn(GetWorld()));
		if (Scanner == nullptr)
		{
			return;
		}

		const UFVScannableComponent* Focus = Scanner->GetFocus();

		OutLines.Add(FString::Printf(TEXT("scan mode %s, progress %.0f%%"),
			Scanner->IsScanModeActive() ? TEXT("on") : TEXT("off"), Scanner->GetProgress() * 100.f));

		OutLines.Add(FString::Printf(TEXT("focus %s%s"),
			Focus != nullptr ? *GetNameSafe(Focus->GetOwner()) : TEXT("none"),
			Focus != nullptr && Focus->WasScanned() ? TEXT(" (scanned)") : TEXT("")));
	}
};

static FAutoConsoleCommandWithWorldAndArgs CmdScanMode(
	TEXT("FV.Scanner.Mode"),
	TEXT("FV.Scanner.Mode [0/1] - toggle the player's scan mode."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (UFVScannerComponent* Scanner = UFVScannerComponent::Find(FVDebug::GetPlayerPawn(World)))
		{
			if (Args.IsEmpty())
			{
				Scanner->ToggleScanMode();
			}
			else
			{
				Scanner->SetScanMode(FCString::Atoi(*Args[0]) != 0);
			}
		}
	}));
