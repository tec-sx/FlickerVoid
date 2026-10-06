# FVNavigationSystem setup

Maps and floors, map markers, discovery of places, the tracked marker and the player's waypoint, and the driving logic for the minimap, compass and world map. An editor tool captures map images from a top-down camera. The plugin has no widgets: your HUD asks the navigator for ready-made views and draws them.

Modules: `FVNavigationSystem`, `FVNavigationSystemDebug`, `FVNavigationSystemEditor` (Map Capture panel).

## 1. Quick start

1. Make a map definition with one layer (section 2) and add it to *FlickerVoid > Navigation > Maps*.
2. Make a capture camera Blueprint, place it over the area and capture the layer image (section 4).
3. Add **FV Navigator Component** to the player pawn.
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
| `DA_Marker_<Kind>` | `FVMarkerDefinition` | Icon, name and tint from **Display**, category from **Tags** (e.g. `Marker.Category.Shop`), **Priority** (draw order), **Visible When** (re-checked on every fact change), **Fragments** |

A marker shows only in the views whose fragment it has:

| Fragment | Fields |
|---|---|
| Minimap | **Clamp To Edge** (stay on the rim when out of range), **Max Distance** (0 = any), **Rotate With Actor** (vehicles) |
| World Map | **Min Zoom** (appears only when zoomed in that far), **Rotate With Actor** |
| Compass | **Max Distance** (0 = any) |
| Discovery | **Radius**, **Hidden Until Discovered** (otherwise shows with **Undiscovered Icon**), **On Discovered** effects |

Typical set: Shop, Inn, Quest Giver, Quest Target (minimap clamp, compass, world map; Visible When = quest active), Point of Interest (discovery), Fast Travel, and **Waypoint** (minimap clamp, compass, world map; set it in *FlickerVoid > Navigation > Waypoint Marker*).

Create category tags under your own root (`Marker.Category.*`) to drive legend filters.

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
| FV Navigator Component | Player pawn | Tracks the map underfoot (**On Active Map Changed**), discovers nearby places, builds the views. **Minimap** settings (radius, min/max radius, rotate with view), **Compass** (field of view), **Hidden Categories** (legend filters) |
| FV Map Marker Component | Anything shown on the map | **Definition**, **Label** (this place's own name), **Discovered Fact** (needed for Discovery markers so discoveries save), **Marker Enabled**. Scene component: place it where the icon should point. Event **On Discovered** |

Markers without an actor (quest areas, search zones): `UFVNavigationSubsystem` > `Add Marker At Location(Definition, Location, Label)` and `Remove Marker`.

## 6. Driving the HUD

The navigator builds plain data each time you ask; call it from your widget's tick or on a short timer.

**Minimap** (`Build Minimap View`): map texture of the layer, `Center UV` and `Extent UV` (the part of the texture to show), `Map Rotation` (rotate the image by this), `Player Rotation` (player arrow), and markers with `Position` in minimap radii (-1..1, +Y down), `Rotation`, `Distance`, `Elevation` (above/below arrows), `Clamped`, `Discovered`, `Tracked`.
A simple widget: a circular-masked material that samples the layer texture around `Center UV` with `Extent UV` and `Map Rotation`, plus a canvas placing marker icons at `Position * Radius` from the centre.

**Compass** (`Build Compass View`): `Heading` (0 = north, clockwise) for the strip, and markers with `Position.X` from -1 (left edge) to 1 (right edge).

**World map** (`Build World Map View(Map, Layer, Zoom)`): markers in texture UV, the player's UV and rotation, and whether the player is on that layer. Pan and zoom are your widget's state; convert clicks with `FVNavigation::MapUVToWorld` and place the waypoint with `Set Waypoint`. Pass the zoom so markers with **Min Zoom** appear as the player zooms in. Switch floors by passing another layer index.

**Waypoint and tracking** on `UFVNavigationSubsystem`: `Set Waypoint(Location)` (replaces the previous one and tracks it), `Clear Waypoint`, `Set Tracked Marker(Handle)` (e.g. the active quest's target). The tracked marker ignores distance and category filters and stays on the minimap and compass edges. Events: **On Marker Added / Removed / Discovered / Visibility Changed**, **On Tracked Marker Changed**, **On Waypoint Changed**.

**Discovery**: entering a Discovery fragment's radius sets the marker component's **Discovered Fact**, applies **On Discovered** effects (e.g. an XP fact, a notification) and fires **On Marker Discovered**.

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
