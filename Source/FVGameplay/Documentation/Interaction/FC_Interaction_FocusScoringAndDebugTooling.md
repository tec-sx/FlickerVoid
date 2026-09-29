# Focus Scoring, Zone Gating & Debug Tooling

This document covers the interaction-focus rework: weighted focus scoring,
zone-based activation gating, and the debug tooling used to inspect both.

## Focus Scoring

`UFVInteractionInstigatorComponent::FindBestTarget()` no longer uses a hard
angle/distance gate. Each registered candidate is scored continuously:

```
AngularQuality  = saturate((Dot - ConeCosine) / (1 - ConeCosine))
DistanceQuality = saturate(1 - Distance / FocusRadius)
Score = AngularWeight * AngularQuality + DistanceWeight * DistanceQuality
```

A candidate is rejected outright only if `Dot < ConeCosine` (outside the cone)
or `Distance > FocusRadius` (outside the target's focus radius). Everything
else contributes a graded, comparable score.

`Dot` and `Distance` are measured from the possessing `APlayerController`'s
camera viewpoint (`GetPlayerViewPoint`), not the pawn's actor location, so
third-person boom offsets don't desync the score from what the player
actually sees.

The currently focused target receives `StickyFocusBonus` (default `0.1`) added
to its score, to prevent flicker between similarly-scored candidates (e.g. a
cluster of NPCs).

### Focus Profiles

`FFVInteractionTargetConfig::FocusProfile` selects one of:

| Profile | ConeCosine | AngularWeight | DistanceWeight | Intent |
|---|---|---|---|---|
| `Precise` | 0.85 | 1.0 | 0.2 | Items — aim direction dominates. |
| `Loose`   | 0.35 | 0.7 | 0.5 | Characters — broader cone, distance matters more. |
| `Custom`  | — | — | — | Uses `CustomFocusProfile` on the config. |

### Aim Probe

`UFVInteractionTargetComponent::GetAimProbeLocation()` replaces the raw actor
origin as the point being aimed at. If `Config->AimProbeSocket` is set and the
owner has a matching skeletal mesh socket, that socket's world location is
used; otherwise it falls back to `ActorLocation + Config->AimProbeOffset`.
This keeps aiming stable for animated characters (whose capsule origin drifts
and sits below the torso).

## Zone Gating

Interaction detection is not a per-tick sweep. `UFVInteractionZoneComponent`
(a `UBoxComponent`) is placed on, or auto-provisioned onto, every interactable
actor. While a pawn's instigator overlaps the zone, the target is registered
as a scoring candidate via `RegisterCandidate`/`UnregisterCandidate`; outside
it, the instigator does no work for that target at all. The instigator only
ticks while it has at least one candidate (`SetComponentTickEnabled`), so idle
cost is zero.

### Auto-Sizing Heuristics

`UFVInteractionTargetComponent::EnsureZoneComponent` creates a zone if none
exists, sized from `UFVInteractionTargetConfig`'s zone knobs
(`ZonePadding`, `ZoneWidthScale`, `ZoneDepthScale`, `MinZoneExtent`):

- **Character-like** (owner has a `UCapsuleComponent`): an asymmetric box —
  `HalfHeight = CapsuleHalfHeight`, `HalfWidth (Y) = CapsuleRadius * ZoneWidthScale`,
  `HalfDepth (X) = CapsuleRadius * ZoneDepthScale`, shifted forward by
  `HalfDepth - CapsuleRadius` so a constant one-capsule-radius of coverage
  remains behind the character regardless of how `ZoneDepthScale` is tuned.
- **Prop-like** (no capsule): the actor's bounds, padded by `ZonePadding` on
  every axis.

Both are clamped to `MinZoneExtent` on every axis.

## Debug Tooling

`UFVInteractionDebugComponent` lives on `AFVPlayerController` (compiled out
entirely in shipping builds) and caches the possessed pawn's instigator/offer
components as a single source of truth for the visualizer and HUD.

- `FVCvar.Interaction.Debug.Draw` — draws each candidate's zone box (green),
  aim-probe point (cyan/yellow if focused/magenta if backing the active
  offer), its focus cone from the camera, a line to the focused target, and a
  highlight on the offer-backing actor.
- `FVCvar.Interaction.Debug.HUD` — an on-screen readout (via
  `UDebugDrawService::Register("Game", ...)`) listing each candidate's
  angular/distance quality and score, the active offer (id, source, priority,
  target, time remaining), each bound slot's availability/reason, and current
  engagement state.
