# Scripts to remove

The traversal scripts were restored from `116dee2^` so the existing assets load again. To avoid name clashes with Angelscript, the C++ copies temporarily have a `Native` suffix. Every C++ use of them is in `Source/FVCharacter/*/Traversal/`.

| Script | Angelscript type | Temporary C++ type (FlickerVoidCharacter) |
|---|---|---|
| Script/Environment/FVTraversable.as | AFVTraversable | AFVTraversableNative |
| Script/Core/Movement/FVTraversalComponent.as | UFVTraversalComponent | UFVTraversalComponentNative |
| Script/Core/Movement/FVMovementConfig.as (traversal part only) | UFVTraversalConfig | UFVTraversalConfigNative |
| Script/Core/Movement/FVMovementConfig.as (traversal part only) | UFVTraversableConfig | UFVTraversableConfigNative |
| Script/Core/Movement/TraversalTypes.as | EFVTraversalActionType | EFVTraversalActionTypeNative |
| Script/Core/Movement/TraversalTypes.as | FFVTraversalCheckResult | FFVTraversalCheckResultNative |
| Script/Core/Movement/TraversalTypes.as | FFVTraversalChooserInput | FFVTraversalChooserInputNative |
| Script/Core/Movement/TraversalTypes.as | FFVTraversalChooserOutput | FFVTraversalChooserOutputNative |
| Script/Core/Movement/TraversalTypes.as | FFVTraversalCharacterData | none (script-only) |
| Script/Core/Movement/FVTraversalComponent.as | FFVTraceParameters | none (script-only) |
| Script/Environment/FVTraversable.as | FFVCheckLedgeResult | none (C++ uses FFVLedgeResult) |

`Script/Core/Player/PlayerCharacter.as` was also reverted. Its `RequestTraverse` builds `FFVTraversalCharacterData` again for the script component.

## Migration
1. In the editor, reparent the BPs and reassign the data assets to the `*Native` types. Examples: `BP_BasicTraversableBlock` goes to `FVTraversableNative`, and the traversal config assets go to `FVTraversalConfigNative` / `FVTraversableConfigNative`. Re-check the property values, because the fields differ (see below).
2. Switch `PlayerCharacter.as` to `UFVTraversalComponentNative` and `TryTraversalAction(DrawDebugType)`.
3. Delete the scripts above. In `FVMovementConfig.as`, remove only the two traversal config classes.
4. Rename the C++ types back by dropping `Native`, and add to `[CoreRedirects]`:
   - `+ClassRedirects=(OldName="/Script/FlickerVoidCharacter.FVTraversableNative",NewName="/Script/FlickerVoidCharacter.FVTraversable")`
   - one for each of FVTraversalComponentNative, FVTraversalConfigNative and FVTraversableConfigNative
   - `+StructRedirects` for FVTraversalCheckResultNative, FVTraversalChooserInputNative and FVTraversalChooserOutputNative
   - `+EnumRedirects` for EFVTraversalActionTypeNative
5. Resave the assets, then remove those redirects.

## Field differences (C++ vs script)
- FFVTraversalCheckResult: the ledge fields are nested in `Ledges` (FFVLedgeResult). MontageToPlay, StartTime, PlayRate and ValidationResult were removed.
- FFVTraversalChooserInput: `PoseHistory` was removed.
- UFVTraversalComponent: no `bIsTraversing`. `TryTraversalAction` takes no CharacterData.
- AFVTraversable: the C++ version adds `Requirements` (FFVConditionSet).