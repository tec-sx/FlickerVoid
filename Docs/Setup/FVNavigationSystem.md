# FVNavigationSystem setup

Maps and floors, map markers, discovery of places, the tracked marker and the player's waypoint, and the driving logic for the minimap, compass and world map. An editor tool captures map images from a top-down camera. The plugin has no widgets: your HUD asks the navigator for ready-made views and draws them.

Modules: `FVNavigationSystem`, `FVNavigationSystemDebug`, `FVNavigationSystemEditor` (Map Capture panel).

## 1. Quick start

1. Make a map definition with one layer (section 2) and add it to *FlickerVoid > Navigation > Maps*.
2. Make a capture camera Blueprint, place it over the area and capture the layer image (section 4).
3. Add **FV Navigator Component** to the player pawn or player controller (on a controller it follows the possessed pawn).
4. Make a marker definition with Minimap, Compass and World Map fragments, and add **FV Map Marker Component** to an actor with that definition.
5. Turn on `FVCvar.Navigation.Debug.HUD 1` and play; then build your HUD widgets on the views (section 6).

## 2. Map definitions

| Asset | Class | Key fields |
|---|---|---|
| `DA_Map_<Area>` | `FVMapDefinition` | **Priority** (higher wins where maps overlap: interior > district > world), **Available When** (e.g. the player bought the map), **Layers** |

Each **layer** is one image and the world area it covers: **Name** (e.g. `Ground`, `Floor1`), **Display Name**, **Texture**, **World Min / World Max** (world XY) and optional **Limit Height** with **Min Z / Max Z** for floors. The image top is north (+X) and its right edge east (+Y). The capture tool fills texture, area and height for you.

How the active map is chosen: the navigator asks for the highest-priority available map with a layer containing the player; a height-limited layer wins over an unlimited one. So walking into a building swaps to its interior map, and taking the stairs swaps the floor.

Typical set: `DA_Map_World` (priority 0, one large layer), `DA_Map_OldTown` (priority 10), `DA_Map_PoliceStation` (priority 20, layers `Basement`, `Ground`, `Floor1` with height limits).

## 3. Marker definitions

| Asset | Class | Key fields |
|---|---|---|
| `DA_Marker_<Kind>` | `FVMarkerDefinition` | Icon, name and tint from **Display**, category from **Tags** (e.g. `Marker.Category.Service`), **Priority** (draw order), **Visible When** (re-checked on every fact change), **Fragments** |

A marker shows only in the views whose fragment it has:

| Fragment | Fields |
|---|---|
| Minimap | **Clamp To Edge** (stay on the rim when out of range), **Max Distance** (0 = any), **Rotate With Actor** (vehicles) |
| World Map | **Min Zoom** (appears only when zoomed in that far), **Rotate With Actor** |
| Compass | **Max Distance** (0 = any) |
| Discovery | **Radius**, **Hidden Until Discovered** (otherwise shows with **Undiscovered Icon**), **On Discovered** effects |

Typical set: Shop, Inn, Quest Giver, Quest Target (minimap clamp, compass, world map; Visible When = quest active), Point of Interest (discovery), Fast Travel, and **Waypoint** (minimap clamp, compass, world map; set it in *FlickerVoid > Navigation > Waypoint Marker*).

## Gameplay tags

Defined natively; add children for your own:

| Root | Defaults | Use |
|---|---|---|
| `Marker.Category` | `Player`, `Quest`, `Exploration`, `Service`, `Travel` | Marker definition **Tags**; legend filters hide categories via the navigator's **Hidden Categories** |
| `Map` | `Map.World`, `Map.Region`, `Map.Interior` | Optional map **Id**, e.g. `Map.Interior.PoliceStation` |
| `Marker.Type` | `Waypoint`, `QuestTarget`, `QuestGiver`, `PointOfInterest`, `Shop`, `FastTravel` | Optional marker **Id** |

Map and marker definitions don't need an Id; set one when you want to find the definition by tag (debug commands) or refer to it from a fact. A display name is recommended (validation warns without one) and the icon is optional; a map's name and icon are what a world map shows in its map list or floor switcher.

## 4. Capturing map images

1. **Camera Blueprint**: make `BP_MapCapture` with parent class `FVMapCaptureActor`. It has a **Capture Box** (area and height) and a top-down orthographic **Capture Component**. Set post-process and show flags on the capture component to style the map, e.g. fixed exposure and no fog. Make several Blueprints for different looks if you like.
2. **Place and size**: drop it in the level and scale the box over the area. The camera sits at the top of the box looking down. For one floor of a building, fit the box between that floor and the ceiling.
3. **Fill in Map Capture**:

| Field | Meaning |
|---|---|
| Map | Map definition to write into |
| Layer | Layer name (dropdown of the map's layers); a missing layer is added |
| Resolution | Pixels along the longer side of the box |
| Texture Name | Asset name; empty uses `T_<Map>_<Layer>` |
| Clip Below Box | Stop rendering at the bottom of the box (unverified for orthographic captures; off by default) |
| Filter | Actors left out: **Ignored Classes**, **Ignored Actor Tags**, **Ignored Actors**, **Ignore Actors Above Box** (roofs over an interior), **Use Project Defaults** |

4. **Capture**: open *FlickerVoid menu > Navigation System > Map Capture* (or the level editor's *Window* menu). The panel lists every capture actor in the level with **Focus**, **Capture**, **Capture All** and the selected actor's details. The actor's Details panel also has a **Capture Map** button.

A capture saves the texture to the capture folder (or updates the layer's existing texture), then writes the texture, the exact captured area and the box's height into the map layer. It can be undone, and a notification links to the texture. Characters are left out by default (`APawn` and the `MapCaptureIgnore` actor tag in project settings); tag anything else you don't want on the map with `MapCaptureIgnore`.

Capture actors are editor only and are stripped from cooked builds.

## 5. Components

| Component | Goes on | Notes |
|---|---|---|
| FV Navigator Component | Player pawn or player controller | Tracks the map underfoot (**On Active Map Changed**), discovers nearby places, builds the views. **Minimap** settings (radius, min/max radius, rotate with view), **Compass** (field of view), **Hidden Categories** (legend filters) |
| FV Map Marker Component | Anything shown on the map | **Definition**, **Label** (this place's own name), **Discovered Fact** (needed for Discovery markers so discoveries save), **Marker Enabled**. Scene component: place it where the icon should point. Event **On Discovered** |

Markers without an actor (quest areas, search zones): `UFVNavigationSubsystem` > `Add Marker At Location(Definition, Location, Label)` and `Remove Marker`.

## 6. Building the navigation UI

The plugin gives you data, not widgets. The **FV Navigator Component** on the player pawn builds a fresh view every time you call it; your widgets draw that view. Read [FVFramework > UI layout](FVFramework.md#3-ui-layout-and-hud-fvcoreui) first for where widgets go.

### 6.1 Where the widgets live

| Widget | Parent class | Where | How it gets there |
|---|---|---|---|
| `WBP_Minimap` | User Widget | Inside `WBP_GameHUD` | Placed in the HUD's designer |
| `WBP_Compass` | User Widget | Inside `WBP_GameHUD` | Placed in the HUD's designer |
| `WBP_GameHUD` | Common Activatable Widget | `UI.Layer.HUD` | Pushed once when the layout is ready |
| `WBP_WorldMap` | Common Activatable Widget | `UI.Layer.GameMenu` | Pushed when the player opens the map, popped on close |
| `WBP_MapMarker` | User Widget | Created by the three widgets above | One per visible marker, reused |

You do **not** need a Navigation layer. The minimap and compass are always-on HUD elements, so they sit inside the one HUD widget next to the quest tracker and status bars. The world map is a full-screen menu, so it goes on the menu layer.

Every navigation widget starts the same way:

```
Event Construct (or when the owning pawn changes):
    Navigator = UFVNavigatorComponent::Get(GetOwningPlayerPawn())   // AngelScript; in Blueprint: Get Component by Class
    if Navigator is not valid: hide yourself and stop
```

Then update on a timer (every 0.03-0.05 s is plenty) or in Tick. Building a view is cheap; it only walks the registered markers.

### 6.2 Marker icon widget

`WBP_MapMarker` (User Widget), reused by minimap, compass and world map:
- An `Image` named **Icon**, and optional small images for **Above** / **Below** arrows and a **Tracked** highlight.
- A function `SetMarker(FFVMarkerView View, float IconSize)`:
  - Icon brush = `View.Definition.Display.Icon` (soft texture: load it once and cache it per definition), tint = `View.Definition.Display.Tint`.
  - When `View.Discovered` is false, show your "unknown place" icon instead (the definition's Discovery fragment holds an **Undiscovered Icon** if you want it per marker; read it with *Get Instanced Struct Value* on `Definition.Fragments`).
  - Above arrow visible when `View.Elevation == Above`, Below arrow when `Below`.
  - Render angle = `View.Rotation` (only non-zero for markers set to rotate with their actor).
  - Tooltip or label = `View.Label`, distance text = `View.Distance / 100` metres.

**Reuse icons instead of recreating them.** Keep a map `Handle -> WBP_MapMarker` in each parent widget. On each update: mark all as unused, then for every marker in the view, find or create its icon and mark it used; finally collapse (or remove) the icons still unused. `FFVMarkerHandle` is unique per marker for the whole session.

### 6.3 Minimap

The minimap is a plain `Image` of the layer texture inside a clipped frame. A `MapRoot` canvas holding the image and the marker icons is resized for zoom, rotated with the camera and translated so the player's spot sits in the centre. No material needed.

**`WBP_Minimap`** (User Widget), designer:
```
SizeBox "Frame"           e.g. 256 x 256, Clipping = Clip to Bounds
 └ CanvasPanel "Root"     fills the frame
    ├ CanvasPanel "MapRoot"   anchors Center, Alignment (0.5, 0.5), Position (0, 0)
    │   ├ Image "MapImage"    anchors Fill (0,0 to 1,1), offsets 0 → always the size of MapRoot
    │   └ marker icons        added at runtime, slot Alignment (0.5, 0.5)
    ├ CanvasPanel "EdgeMarkers"  anchors Fill; out-of-range markers pinned to the rim
    └ Image "PlayerArrow"     anchors Center, Alignment (0.5, 0.5); arrow texture pointing up
```
For a round minimap, put a circular mask image over it or use a retainer box; the maths below doesn't change.

All values come from the struct that `Navigator > Build Minimap View` returns (*Break FFVMinimapView*): `Map`, `LayerIndex`, `CenterUV` (the player's spot on the layer image, 0..1), `ExtentUV` (how much of the image the frame shows; this is the zoom), `MapRotation`, `PlayerRotation`, `Markers`.

Each update (timer of ~0.03 s, or Tick):
```
View = Navigator.BuildMinimapView()
if View.Map is not valid: show "no map", stop                        // player is outside every map
if map or layer changed: MapImage.SetBrushFromSoftTexture(View.Map.Layers[View.LayerIndex].Texture)

FrameSize = Frame size in pixels                                     // e.g. (256, 256)
Size      = FrameSize / (2 * View.ExtentUV)                          // per axis; MapRoot size for this zoom
P         = View.CenterUV

MapRoot slot:  SetSize(Size)
MapRoot:       SetRenderTransformPivot(P)                            // rotate around the player's spot
               SetRenderTransformAngle(View.MapRotation)             // 0 when the minimap is north-up
               SetRenderTranslation((0.5 - P) * Size)                // brings the player's spot to the centre
PlayerArrow:   SetRenderTransformAngle(View.PlayerRotation)
```

Example: a 2048 x 2048 image of a 1 km square layer and the navigator's default 50 m radius gives `ExtentUV = 0.05`, so `Size = 256 / 0.1 = 2560` px; with the player at `P = (0.7, 0.3)` the translation is `(-512, 512)`.

Why it is correct: UMG applies a widget's render transform about its pivot, in this order: scale, shear, rotation, then translation, with the translation in the parent's unscaled, unrotated space. `MapRoot` is centred in `Root`, so before the transform the player's spot is `(P - 0.5) * Size` from the frame centre. Rotation about a pivot on that spot leaves it in place, and the translation `(0.5 - P) * Size` moves it to the exact centre, whatever the angle. Zoom changes `Size` (layout size), not the render scale, so the image stays sharp and the same formula holds.

**Markers.** The minimap view lists every marker that should show. For one that isn't clamped, place it in map space inside `MapRoot`, so it pans and rotates with the map:
```
UV = FVNavigation::WorldToMapUV(View.Map, View.LayerIndex, Navigation.GetMarkerLocation(Marker.Handle))
Icon slot (in MapRoot):  Position = UV * Size, Alignment (0.5, 0.5)
Icon:                    SetRenderTransformAngle(Marker.Rotation - View.MapRotation)   // counter-rotate so icons stay upright
```
(`Navigation` = `UFVNavigationSubsystem::Get()`.) For a clamped marker (`Marker.Clamped`, e.g. the tracked quest target out of range), its map position is outside the frame, so put it in `EdgeMarkers` instead: `Position = FrameSize / 2 + Marker.Position * (FrameSize.X / 2 - EdgePadding)`, angle `atan2(Marker.Position.X, -Marker.Position.Y)` in degrees if you want it to point outward.

Shortcut: you can place *every* marker in `EdgeMarkers` with that same formula (`Marker.Position` is already rotated and zoomed for the minimap) and skip `WorldToMapUV`; icons then stay upright without counter-rotation. Markers inside `MapRoot` only matter when you want them to scroll smoothly between view updates.

**Zoom** with `Navigator.ZoomMinimap(0.8)` / `ZoomMinimap(1.25)` from an input action; the radius is clamped to **Min/Max Radius** and `ExtentUV` follows. **North-up** vs **rotating**: the navigator's **Rotate With View** setting; with it off `MapRotation` is 0. Swap the texture when **On Active Map Changed** fires (entering a building or changing floor).

#### Option: material instead of render transforms

If you'd rather sample the texture in a material (no huge image widget, built-in circular mask): make `M_Minimap` (Material Domain *User Interface*, Blend Mode *Masked* or *Translucent*) with parameters `MapTexture`, `CenterUV` (Vector, R and G), `ExtentUV` (Vector, R and G) and `MapRotation` (Scalar, degrees), and a *Custom* node (inputs `UV` = TexCoord 0, `CenterUV`, `ExtentUV`, `RotationDeg`; output *Float 2*):

```hlsl
float2 p = (UV - 0.5) * 2;                       // -1..1, +Y down
float a = radians(RotationDeg);
float2 q = float2(p.x * cos(a) + p.y * sin(a),   // undo the on-screen rotation
                  -p.x * sin(a) + p.y * cos(a));
return CenterUV + q * ExtentUV;
```
Sample `MapTexture` with that UV (sampler *Clamp*); opacity mask `length(p) <= 1` for a round map. Set the four parameters from the view on a dynamic material instance each update, use it as the brush of a fixed, centred `MapImage`, and place markers with the `EdgeMarkers` formula above.

### 6.4 Compass

**`WBP_Compass`** (User Widget), designer:
```
SizeBox (e.g. 600 x 48)
 └ Overlay
    ├ Image "Strip"          optional scrolling tick texture
    ├ CanvasPanel "Ticks"    cardinal letters and markers
    └ Image "CentreLine"     centred notch
```
Set the navigator's **Compass > Field Of View** to the degrees the strip shows (e.g. 180).

Update:
```
View = Navigator.BuildCompassView()
Half = Navigator.Compass.FieldOfView / 2
Width = strip width in pixels

for each (Letter, Bearing) in [(N,0), (NE,45), (E,90), (SE,135), (S,180), (SW,225), (W,270), (NW,315)]:
    Relative = NormalizeAxis(Bearing - View.Heading)        // -180..180
    visible when abs(Relative) <= Half
    X = Width / 2 + Relative / Half * Width / 2

for each Marker in View.Markers:
    X = Width / 2 + Marker.Position.X * Width / 2
    Icon.SetMarker(Marker, 20); show Marker.Distance under it; dim it when Marker.Clamped (the tracked target is behind you)
```

For a continuous tick strip instead of letters: a material with a horizontally tiling texture that covers 360°, U offset = `View.Heading / 360` and tiling = `FieldOfView / 360`.

### 6.5 World map

`WBP_WorldMap` (Common Activatable Widget; input mode *Menu*, shows the mouse cursor) pushed to `UI.Layer.GameMenu` by an *Open Map* input action and popped by *Back*.

Designer:
```
Overlay (fill)
 ├ Image "MapImage"        brush = MID of M_Minimap (reuse it, rotation 0, opacity mask 1)
 ├ CanvasPanel "Markers"
 ├ Image "PlayerIcon"
 └ VerticalBox "Floors"    one button per layer
```

The designer above uses the material from 6.3. With render transforms instead, reuse the minimap hierarchy (`Frame` > `Root` > `MapRoot` with `MapImage` and markers): set `MapRoot` size `Size = ViewSize / (2 * ExtentUV)`, pivot `PanUV`, angle 0 and translation `(0.5 - PanUV) * Size`; place markers inside `MapRoot` at `Marker.Position * Size` (world map positions are already texture UV), so dragging and zooming only change `PanUV` and `Size`.

Widget state: `Map`, `LayerIndex`, `PanUV` (centre of the view in texture UV) and `Zoom` (1 = whole layer fits).

On activation:
```
Map = Navigator.GetActiveMap(); LayerIndex = Navigator.GetActiveLayer()
if no active map: pick one from UFVNavigationSubsystem.GetMaps()
PanUV = player UV (from BuildWorldMapView) ; Zoom = 1
build one floor button per Map.Layers entry (DisplayName); clicking sets LayerIndex
```

Extent (the half-size of the view in texture UV), keeping the image's proportions:
```
Size     = Layer.WorldMax - Layer.WorldMin            // world size; image width runs along Size.Y
Aspect   = (ViewWidth / ViewHeight) / (Size.Y / Size.X) // view aspect / image aspect
ExtentUV = (0.5 / Zoom * max(Aspect, 1), 0.5 / Zoom * max(1 / Aspect, 1))
MID: CenterUV = PanUV, ExtentUV, MapRotation = 0, MapTexture = layer texture
```

Converting between the screen and the map:
```
UVToScreen(UV)     = ((UV - PanUV) / (2 * ExtentUV) + 0.5) * ViewSize
ScreenToUV(Pixel)  = PanUV + (Pixel / ViewSize - 0.5) * 2 * ExtentUV
```

Input (override *On Mouse Button Down/Up*, *On Mouse Move*, *On Mouse Wheel* in the widget, or use Enhanced Input actions for gamepad sticks):
- **Pan** (drag or right stick): `PanUV -= DeltaPixels / ViewSize * 2 * ExtentUV`, then clamp each axis of `PanUV` to `[ExtentUV, 1 - ExtentUV]`; on an axis where `ExtentUV >= 0.5` (the whole image already fits) keep it at 0.5.
- **Zoom** (wheel or triggers) around the cursor: `CursorUV = ScreenToUV(Cursor)`; `Zoom = clamp(Zoom * 1.15^WheelDelta, 1, 8)`; recompute `ExtentUV`; `PanUV = CursorUV - (Cursor / ViewSize - 0.5) * 2 * ExtentUV`.
- **Place waypoint** (click or a button): `UV = ScreenToUV(Cursor)`; `Location = FVNavigation::MapUVToWorld(Map, LayerIndex, UV, PlayerZ)`; `UFVNavigationSubsystem.SetWaypoint(Location)`. Clicking the waypoint again calls `ClearWaypoint`.
- **Hover a marker**: the marker whose screen position is closest to the cursor within ~20 px; show its `Label` and the definition's `Display.ShortDescription`.
- **Track a marker** (click on it): `SetTrackedMarker(Marker.Handle)`.

Each frame (or on any change):
```
View = Navigator.BuildWorldMapView(Map, LayerIndex, Zoom)
PlayerIcon visible when View.PlayerOnLayer; position = UVToScreen(View.PlayerUV); angle = View.PlayerRotation
for each Marker in View.Markers:
    Pos = UVToScreen(Marker.Position); skip when outside the view
    Icon.SetMarker(Marker, 32) at Pos
```
Markers with a World Map fragment **Min Zoom** only come back once `Zoom` reaches it, so towns can show only the inn and the shop until the player zooms in.

**Legend**: a list of category toggles (`Marker.Category.*`). Toggling calls `Navigator.SetCategoryHidden(Category, bHidden)`; the minimap, compass and world map all respect it, except the tracked marker.

### 6.6 Waypoint, tracking and discovery events

On `UFVNavigationSubsystem` (`UFVNavigationSubsystem::Get()` in AngelScript, *Get World Subsystem* in Blueprint):
- `SetWaypoint(Location)` replaces the previous waypoint and tracks it; `ClearWaypoint`; `SetTrackedMarker(Handle)` for the active quest target.
- The tracked marker ignores distance and category filters and stays pinned to the minimap and compass edges.
- Events: **On Marker Added / Removed / Discovered / Visibility Changed**, **On Tracked Marker Changed**, **On Waypoint Changed**. Use **On Marker Discovered** for a "New location discovered" toast (the label is on the marker's component: `GetMarkerActor` then its FV Map Marker Component `GetLabel`, or the definition's `Display.Name`).
- Entering a Discovery fragment's radius sets the marker component's **Discovered Fact**, applies **On Discovered** effects and fires **On Marker Discovered**.

## 7. Settings

*FlickerVoid > Navigation* (also *FlickerVoid menu > Navigation System > Global Settings*):

| Setting | Meaning |
|---|---|
| Maps | Every map the player can be on |
| Waypoint Marker | Marker definition for the player's waypoint |
| Elevation Threshold | Height difference that counts as above or below |
| Update Interval | Seconds between the navigator's map and discovery checks |
| Capture Folder | Where new map textures are saved |
| Capture Ignored Classes / Actor Tags | Left out of every capture (defaults: `APawn`, `MapCaptureIgnore`) |

## 8. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Navigation.Debug.HUD 1` | Active map and layer, map UV, heading, nearby markers |
| `FVCvar.Navigation.Debug.Range <cm>` | How far the HUD lists markers |
| `FV.Navigation.ListMarkers` | Log every marker with its id |
| `FV.Navigation.DiscoverAll` | Discover everything |
| `FV.Navigation.Waypoint <X> <Y>` | Set the waypoint; no arguments clears it |
| `FV.Navigation.Track <MarkerId>` | Track a marker; no argument stops tracking |

## Not yet done

The waypoint and the tracked marker are not saved; there is no fog of war over unexplored areas; HUD widgets are up to the game.
