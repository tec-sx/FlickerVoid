#include "Scanner/FVScannerDebug.h"

#include "FVDebugUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Scanner/FVScannableComponent.h"
#include "Scanner/FVScannerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScannerDebug)

static TAutoConsoleVariable<bool> CVarScannerDebugHUD(
	TEXT("FVCvar.Scanner.Debug.HUD"),
	false,
	TEXT("Show scan mode, the focused scannable and scan progress on screen."));

bool UFVScannerDebugSubsystem::IsEnabled() const
{
	return CVarScannerDebugHUD.GetValueOnGameThread();
}

void UFVScannerDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const APawn* Pawn = FVDebug::GetPlayerPawn(GetWorld());
	const UFVScannerComponent* Scanner = UFVScannerComponent::Find(Pawn);
	if (Scanner == nullptr)
	{
		return;
	}

	const UFVScannableComponent* Focus = Scanner->GetFocus();
	const TCHAR* ScanMode = Scanner->IsScanModeActive() ? TEXT("on") : TEXT("off");
	OutLines.Add(FString::Printf(TEXT("scan mode %s, progress %.0f%%"), ScanMode, Scanner->GetProgress() * 100.f));

	const FString FocusName = Focus != nullptr ? GetNameSafe(Focus->GetOwner()) : TEXT("none");
	const TCHAR* Scanned = Focus != nullptr && Focus->WasScanned() ? TEXT(" (scanned)") : TEXT("");
	OutLines.Add(FString::Printf(TEXT("focus %s%s"), *FocusName, Scanned));
}

static FAutoConsoleCommandWithWorldAndArgs CmdScanMode(
	TEXT("FV.Scanner.Mode"),
	TEXT("FV.Scanner.Mode [0/1] - toggle the player's scan mode."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		APawn* Pawn = FVDebug::GetPlayerPawn(World);
		UFVScannerComponent* Scanner = UFVScannerComponent::Find(Pawn);
		if (!Scanner)
		{
			return;
		}

		if (Args.IsEmpty())
		{
			Scanner->ToggleScanMode();
			return;
		}
		Scanner->SetScanMode(FCString::Atoi(*Args[0]) != 0);
	}));
