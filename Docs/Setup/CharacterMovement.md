# Character movement (Mover) and animation (UAF)

Characters move with Epic's **Mover** (`UCharacterMoverComponent`) instead of the Character Movement Component, and animation moves to the **Unreal Animation Framework** (UAF). Both plugins are Experimental in 5.8. NPCs (`AFVAICharacter`) stay on the Character Movement Component until Mover's NavWalking is set up for AI paths.

## How it fits together

Mover's own extension points replace the old movement handlers:

| Old handler system | Mover |
|---|---|
| Handler (`GenerateMovement`) | **Movement mode** (`UBaseMovementMode`): `Generate Move` proposes the move, `Simulation Tick` runs it. Modes are Blueprintable, so AngelScript subclasses them. |
| `Resolve()` deciding which handler runs | **Transition** (`UBaseMovementModeTransition`): `Evaluate` returns the mode to switch to, `Trigger` runs on the switch. On a mode, or global (checked in every mode). |
| Handler data asset per character | **Movement set** (`UFVMovementSet`): named modes plus global transitions. A character lists its sets; sets can be added and removed at runtime. |

Input is separate from movement: the locomotion component sends the intent, and the active mode decides what it means.

| Piece | Does |
|---|---|
| `AFVCharacter` | Pawn (no longer `ACharacter`): capsule, skeletal mesh, `CharacterMover`, `Locomotion`, `MovementSets`, Flow component, owned tags. Installs the starting movement sets before Mover initializes (game worlds only). The capsule and mesh keep `ACharacter`'s subobject names (`CollisionCylinder`, `CharacterMesh0`), so existing Blueprints keep their settings. |
| `UFVLocomotionComponent` | Mover input producer (C++ interface; Mover gathers input from producer components). Holds the intent (move, walk, sprint, crouch, jump, aim, traversing) and sends it as Mover's default character inputs plus `FFVLocomotionInputs`. Exposes the state animation, UAF and traversal read, and the events `OnLanded`, `OnStartedFalling`, `OnGaitChanged`. |
| `FFVLocomotionInputs` | Custom Mover input: gait, local move direction, facing the camera, aiming, traversing. Native because Mover needs custom input types in C++. |
| `UFVLocomotionLibrary` | `GetLocomotionInputs(StartState)` and `GetLocomotionInputsFromTick(Params)` for modes and transitions, AngelScript included. |
| `UFVGroundMovementMode` | Walking mode with FlickerVoid gaits: Walk, Run, Sprint, Crouch (forward, strafe and backward speeds, acceleration, deceleration, friction), turning rate and an optional speed multiplier attribute. Puts them into Mover's Common Legacy Movement Settings, then runs Mover's walking logic. |
| `UFVMovementSet` | Data asset: modes by name (instanced, each with its own settings and transitions) and global transitions. |
| `UFVMovementSetComponent` | Starting sets and starting mode for the character; `AddMovementSet`, `RemoveMovementSet`, `HasMovementSet` at runtime. Each character gets its own copies of the set's modes and transitions. |
| `UFVMovementSetTransition` | One global Mover transition the movement set component adds before Mover initializes. It runs the transitions of every installed set in order. Mover reads its own global transitions only once, so this is how sets added later bring theirs. |
| `UFVTraversalComponent` | Unchanged checks; reads the capsule from the root and the state from the locomotion component. |
| `UFVCharacterAnimInstance` | Kept for the current Anim Blueprint while the UAF graph is built; reads the locomotion component. |

Removed: `UFVCharacterMovementComponent`, the handler framework (`UFVMovementHandlerBase`, `UFVMovementHandlerData`, `FFVMovementHandlerInfo`, `UFVMovementHandlerConfigBase`) and `Script/Core/Movement`.

Behaviour kept from the handlers: the character turns towards its movement, and faces the camera while aiming or with sideways input; strafe and backward speeds apply only while facing the camera; sprint only counts within the sprint angle of the facing direction (*Sprint Angle Threshold* on the locomotion component); sprint and jump stand a crouching character up; walking off a ledge stands up (`PlayerCharacter.as`); movement input stops while the ability system has a blocking tag (Status.Movement.Blocked, plus Status.Death and Status.Interacting on the player).

## Editor steps

1. **Build** with the new plugins on: `Mover`, `MoverAnimNext`, `UAF`, `UAFAnimGraph`, `UAFPoseSearch`, `UAFChooser`, `UAFWarping`, `UAFControlRig`, `UAFMirroring`, `UAFStateTree`. `MoverExamples` (sample pawns, zipline and wall-run modes) is a useful reference; turn it on if you want it.
2. **Base movement set** `DA_Moves_Base` (`UFVMovementSet`):
   - `Walking`: *FV Ground Movement Mode*. Its defaults match the old handler values.
   - `Falling`: *Falling Mode*.
   - `Flying`: *Flying Mode* (traversal used flying before).
   Mover's own validation reports modes that need settings or names it can't find, for example the ground and air mode names in Common Legacy Movement Settings (`Walking`, `Falling` by default).
3. **Player Blueprint** (on `APlayerCharacter`):
   - *MovementSets* component: *Starting Sets* = `DA_Moves_Base`; *Starting Movement Mode* = `Walking`.
   - *CharacterMover* component: leave *Movement Modes* as it is, or empty. Sets add their modes by name and replace modes with the same name. Leave *Transitions* for Mover's own; set transitions go in the sets. *Shared Settings* must contain a **Common Legacy Movement Settings** object; the ground mode overwrites its speed, acceleration, deceleration, friction and turning rate.
   - *Locomotion* component: *Sprint Angle Threshold*, blocking tags.
4. **Old assets to delete** (no longer load): any `UFVMovementHandlerData` asset and the old *Character Movement* component settings on the player Blueprint. The traversal config stays (same class, now based on `UObject`).
5. **Traversal montages**: the old traverse handler switched to flying. Under Mover, play the traversal montage with root motion through Mover's animation root motion layered move, as the Game Animation Sample's Mover character does, and call `Locomotion.SetTraversing(false)` when it ends. A dedicated traversal mode can replace this later. Not written in code yet.
6. **Animation now**: the current Anim Blueprint keeps working; it reads `UFVLocomotionComponent` through `UFVCharacterAnimInstance`.
7. **Animation on UAF**, following the Game Animation Sample's Mover and UAF characters:
   - Create a UAF Workspace, a System and an Animation Graph (*Animation > Animation Framework*).
   - System: reference pose on initialize; on update, build the motion matching trajectory from the Mover component (MoverAnimNext), run the graph, write the pose.
   - Graph: chooser, then motion matching with the pose search databases and the trajectory.
   - Bind graph variables to the locomotion component: `GetGait`, `GetStance`, `GetMovementMode`, `IsAiming`, `IsTraversing`, `JustLanded`, `GetGroundSpeed`, `GetRuntimeState`.
   - On the player Blueprint add the UAF (AnimNext) component with the System, and turn animation off on the skeletal mesh (no Anim Blueprint), or UAF won't run.
   - Once it works, `UFVCharacterAnimInstance`, its proxy, `UFVAnimationConfigData` and `Script/Animation/FVPlayerAnimInstance.as` can go.

## Adding a movement ability in AngelScript

Example: swimming for some characters only.

1. Write the mode, for example `USwimMovementMode : UBaseMovementMode` (or subclass an engine mode), overriding `GenerateMove` and `SimulationTick`. Read the intent with `FVLocomotion::GetLocomotionInputs(StartState, Inputs)`.
2. Write its transitions, for example `UEnterWaterTransition : UBaseMovementModeTransition` with `Evaluate` returning the swim mode's name when the character is in water, and one back to `Walking` on the swim mode.
3. Make `DA_Moves_Swim` with `Swimming` = the swim mode (its exit transition on the mode) and the enter transition under *Transitions*.
4. Add `DA_Moves_Swim` to the *Starting Sets* of the characters that swim, or call `MovementSets.AddMovementSet(DA_Moves_Swim)` when one learns it.

Script modes and transitions run on the game thread (leave *Supports Async* off). A set transition's *First Sub Step Only* is not applied; *Allow Mode Reentry* is. To check on the fork first: AngelScript overriding Mover's `GenerateMove`/`SimulationTick`/`Evaluate` with their struct parameters (`FMoverTickStartData`, `FProposedMove`, `FSimulationTickParams`). Layered moves (dash, launch) need C++ for their data, because Mover's Blueprint nodes for move data use custom thunks.

Other useful calls: `Mover.QueueNextMode(n"Flying")`, `Locomotion.IsOnGround()`, `Locomotion.OnLanded.AddUFunction(this, n"OnLanded")`.
