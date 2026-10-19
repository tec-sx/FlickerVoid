# Character movement (Mover) and animation (UAF)

Characters move with Epic's **Mover** (`UCharacterMoverComponent`) instead of the Character Movement Component, and animation moves to the **Unreal Animation Framework** (UAF). Both plugins are Experimental in 5.8. NPCs (`AFVAICharacter`) stay on the Character Movement Component until Mover's NavWalking is set up for AI paths.

## What is in code

| Piece | Where | Does |
|---|---|---|
| `AFVCharacter` | FlickerVoidCharacter | Pawn (no longer `ACharacter`): capsule, skeletal mesh, `CharacterMover`, `Locomotion`, Flow component, owned tags. It is Mover's input producer and hands the job to the locomotion component, so Mover's interface never reaches AngelScript. The capsule and mesh keep `ACharacter`'s subobject names (`CollisionCylinder`, `CharacterMesh0`), so existing Blueprints keep their settings. |
| `UFVLocomotionComponent` | FlickerVoidCharacter | Movement intent (move, walk, sprint, crouch, jump, aim, traversing), turned into Mover input each step. Writes the active gait into Mover's Common Legacy Movement Settings (max speed, acceleration, deceleration, friction, turning rate). Exposes the state animation reads and the events `OnLanded`, `OnStartedFalling`, `OnGaitChanged`. |
| `UFVLocomotionData` | FlickerVoidCharacter | Data asset: Walk, Run, Sprint and Crouch speeds (forward, strafe, backward), acceleration, deceleration, friction, sprint angle, turning rate, and an optional speed multiplier attribute on the ability system. |
| `UFVTraversalComponent` | FlickerVoidCharacter | Unchanged checks; now reads the capsule from the root and the movement state from the locomotion component. |
| `UFVCharacterAnimInstance` | FlickerVoidCharacter | Kept for the current Anim Blueprint while the UAF graph is built; reads the locomotion component. |

Removed: `UFVCharacterMovementComponent`, the movement handler framework (`UFVMovementHandlerBase`, `UFVMovementHandlerData`, `FFVMovementHandlerInfo`, `UFVMovementHandlerConfigBase`) and `Script/Core/Movement` (ground, in-air and traverse handlers, `UFVGroundMovementConfig`).

Behaviour kept from the handlers: the character turns towards its movement, and faces the camera while aiming or with sideways input; sideways and backward speeds apply only while facing the camera; sprint only counts within the sprint angle of the facing direction; sprint and jump stand a crouching character up; walking off a ledge stands up (`PlayerCharacter.as`); movement stops on the blocking tags (Status.Movement.Blocked, plus Status.Death and Status.Interacting on the player) when they are on the ability system.

## Editor steps

1. **Build** with the new plugins on: `Mover`, `MoverAnimNext`, `UAF`, `UAFAnimGraph`, `UAFPoseSearch`, `UAFChooser`, `UAFWarping`, `UAFControlRig`, `UAFMirroring`, `UAFStateTree`. `MoverExamples` and `MoverIntegrations` stay off; turn `MoverExamples` on if you want its sample pawns as a reference.
2. **Player Blueprint** (the one on `APlayerCharacter`), *CharacterMover* component (the `Mover` property):
   - *Movement Modes*: Walking (`Walking Mode`, or `Smooth Walking Mode` as in the Game Animation Sample), Falling (`Falling Mode`), Flying (`Flying Mode`, used for traversal), Swimming if needed. Skip this if the component already lists them.
   - *Starting Movement Mode*: Walking.
   - *Shared Settings* must contain a **Common Legacy Movement Settings** object (Mover adds it for the default modes). The locomotion component writes the gait into it every frame, so values typed there for speed, acceleration, deceleration, friction and turning rate are overwritten; set them in the locomotion data instead.
   - *Backend Class*: leave the default unless you want a standalone backend.
   - Compile: Mover's own validation reports missing modes or settings.
3. **Locomotion data**: create a `UFVLocomotionData` asset (for example `DA_Locomotion_Player`; defaults match the old handler values) and set it on the Blueprint's *Locomotion* component. Leave *Speed Multiplier Attribute* empty until the GAS attributes land, then pick the movement speed attribute.
4. **Old assets to delete** (no longer load): any `UFVMovementHandlerData` asset, and on the player Blueprint the old *Character Movement* component settings. The traversal config stays (same class, now based on `UObject`).
5. **Traversal montages**: the old traverse handler switched the Character Movement Component to flying. Under Mover, play the traversal montage with root motion through Mover's animation root motion layered move, as the Game Animation Sample's Mover character does, and call `Locomotion.SetTraversing(false)` when it ends. This part is not written in code yet.
6. **Animation now**: the current Anim Blueprint keeps working; it reads `UFVLocomotionComponent` through `UFVCharacterAnimInstance`.
7. **Animation on UAF**, following the Game Animation Sample's Mover and UAF characters:
   - Create a UAF Workspace, a System and an Animation Graph (*Animation > Animation Framework*).
   - System: reference pose on initialize; on update, build the motion matching trajectory from the Mover component (MoverAnimNext), run the graph, write the pose.
   - Graph: chooser, then motion matching with the pose search databases and the trajectory.
   - Bind graph variables to the locomotion component: `GetGait`, `GetStance`, `GetMovementMode`, `IsAiming`, `IsTraversing`, `JustLanded`, `GetGroundSpeed`, `GetRuntimeState`.
   - On the player Blueprint add the UAF (AnimNext) component with the System, and turn animation off on the skeletal mesh (no Anim Blueprint), or UAF won't run.
   - Once it works, `UFVCharacterAnimInstance`, its proxy, `UFVAnimationConfigData` and `Script/Animation/FVPlayerAnimInstance.as` can go.

## AngelScript

```angelscript
// Movement state and events on any character
Locomotion.IsOnGround();
Locomotion.GetGait();
Locomotion.OnLanded.AddUFunction(this, n"OnLanded");

// Mover itself, for modes, layered moves and modifiers
Mover.QueueNextMode(n"Flying");
```

Input still arrives through `AFVPlayerController`, which now calls the locomotion component on the possessed pawn.
