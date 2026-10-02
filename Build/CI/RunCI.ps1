param(
    [string]$EngineRoot = 'C:\src\repos\FVUnrealEngine\LocalBuilds\Engine\Windows',
    [string]$Configuration = 'Development'
)

$ErrorActionPreference = 'Stop'
$Project = Join-Path $PSScriptRoot '..\..\FlickerVoid.uproject' | Resolve-Path
$Build = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
$Editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Logs = Join-Path $PSScriptRoot '..\..\Saved\CI'
New-Item -ItemType Directory -Force $Logs | Out-Null

function Invoke-Step([string]$Name, [scriptblock]$Body) {
    Write-Host "== $Name"
    & $Body
    if ($LASTEXITCODE -ne 0) { throw "$Name failed ($LASTEXITCODE)" }
}

Invoke-Step 'Build editor' { & $Build FlickerVoidEditor Win64 $Configuration -Project="$Project" -WaitMutex }
Invoke-Step 'Validate data' { & $Editor "$Project" -run=DataValidation -unattended -nop4 -nosplash -NullRHI -abslog="$Logs\validation.log" }
Invoke-Step 'Automation tests' { & $Editor "$Project" -ExecCmds="Automation RunTests FlickerVoid; Quit" -unattended -nop4 -nosplash -NullRHI -TestExit="Automation Test Queue Empty" -abslog="$Logs\tests.log" }

Write-Host 'CI passed'