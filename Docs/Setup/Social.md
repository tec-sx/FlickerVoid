# Social setup

Factions and the player's standing with them, one global fame and one global notoriety, titles the world gives the player, personal relationships, gifts and disguises. Social is a game rule, not a plugin: the C++ shapes live in the game's gameplay module (`Source/FVGameplay/.../Social`) and the conditions and effects in AngelScript (`Script/Gameplay/Protagonist/Identity/Social`). Nothing decays over time.

Every value is a **scale** (`FVScaleDefinition`, FVFramework): a fact with optional bands and links. Its range is the fact's clamp in *FlickerVoid > Facts*, so it saves, shows in the facts debugger and works with the generic **Scale** condition and **Modify Scale** effect.

## 1. Quick start

1. Create fact tags: `Fact.Social.Fame`, `Fact.Social.Notoriety`, and per faction `Fact.Faction.<Name>.Standing` and `Fact.Faction.<Name>.Notoriety`.
2. Declare them in *FlickerVoid > Facts* with a clamp (standing -100..100, notoriety and fame 0..100). The clamp is the scale's range and fills the menus' bars.
3. Make a scale for each (section 2): `DA_Scale_Fame`, `DA_Scale_Notoriety`, `DA_Scale_<Faction>Standing`, `DA_Scale_<Faction>Notoriety`.
4. Make a faction definition and add it to *FlickerVoid > Social > Factions*.
5. Set *Fame* and *Notoriety* in *FlickerVoid > Social*.
6. Add the **Social** fragment to NPC character definitions to make them faction members.
7. Change standing with the **Modify Scale** effect (quest rewards, dialogue choices) and watch `FVCvar.Social.Debug.HUD 1`.

## 2. Data assets

| Asset | Class | Key fields |
|---|---|---|
| `DA_Scale_<Name>` | `FVScaleDefinition` | **Id** = the fact holding the value. **Bands** (up to 3: threshold, tag, label; the highest threshold reached applies). **Links** (up to 3: another scale and the factor of each applied change passed on, once; **Magnitude** passes the size of the change whatever its sign) |
| `DA_Faction_<Name>` | `FVFactionDefinition` | **Id** = the faction (stolen goods use it). **Standing** scale, **Band Attitudes** (standing band tag to attitude; no match is Neutral), **Notoriety** scale, **Hostile Notoriety**, **Disguise Tag**, **Disguise Notoriety Limit** |
| `DA_Title_<Name>` | `FVTitleDefinition` | **Id** = fact holding the title's rank (0 = not earned; the highest rank is the current title). **Conditions** (e.g. Scale fame >= 60 and Scale docker standing >= 40), **On Earned** effects (applied once). Earned titles are never lost. Among titles whose conditions hold, the hardest leads: the highest at-least thresholds (summed over the conditions), then how far past them the values are. The leader becomes current whenever it changes. The **Promote Title** effect makes a title current until the leader changes again |

What used to be faction fields are now links on the standing scale:
- **Relations**: a link to the other faction's standing scale, e.g. Police standing -0.5 to Gangs standing.
- **Fame contribution**: a link to the fame scale with **Magnitude** on, so any change in standing makes the player better known.
- **Notoriety contribution**: a link to the global notoriety scale (raise it for criminal factions).

### Example standing scale

Scales have at most three bands, so use the attitude extremes:

| Threshold | Band tag | Label | Attitude (faction's Band Attitudes) |
|---|---|---|---|
| -100 | `Faction.Band.Hated` | Hated | Hostile |
| -20 | `Faction.Band.Neutral` | Neutral | Neutral |
| 50 | `Faction.Band.Trusted` | Trusted | Allied |

Members of the faction always treat each other as Allied, and anyone whose notoriety (the higher of the faction's and the global one, as recognized) reaches **Hostile Notoriety** is Hostile.

## 3. Characters

Add the **Social** fragment to an `FVCharacterDefinition`:
- **Faction**: the NPC's faction (used by attitude checks and AI).
- **Relationship**: the scale holding the player's relationship (affinity) with this character, e.g. `DA_Scale_RelationshipMara` with Id `Fact.Relationship.Mara`. Declare its fact in Facts settings to give it a range and starting value.

The NPC actor needs an **FV Identity Component** pointing at that definition.

### Gifts
Add the **Gifts** fragment to make a character care what they are given:
- **Preferences**: up to three, each a **Tag** matched against the gift's definition Tags and Category (a parent tag matches every child, so `Item.Consumable.Drink` covers wine and beer) and a **Relationship Change** (negative for gifts they dislike).
- **Default Relationship Change**: used when no preference matches.

Matching preferences add up. `Get Gift Relationship Change(Receiver, Gift Tags)` previews the change; `Receive Gift(Receiver, Gift Tags)` applies it to the receiver's **Relationship** scale and returns the change that applied (0 without a relationship scale).

Items and social don't call each other: the give ability and the Give Item Flow node broadcast `Message.Item.GiveFinished` with **As Gift** set when a dialogue gives an item as a gift, and the social subsystem listens and calls `Receive Gift` with the item's Tags plus its Category.

## 4. Disguises

1. Create disguise tags under a root, e.g. `Disguise.Faction.Police`, and set *FlickerVoid > Social > Disguise Root* to `Disguise`.
2. Set the faction's **Disguise Tag** to `Disguise.Faction.Police`.
3. Give an outfit item an Equippable fragment granting that tag. The equipment component adds it to the wearer's Ability System Component.
4. While worn, police treat the player as a member (Allied) until the notoriety they read reaches the faction's **Disguise Notoriety Limit**, and onlookers read only *Disguise Recognition* (default 25%) of the player's fame and notoriety.

The player needs an Ability System Component for disguises to be read.

## 5. Settings

*FlickerVoid > Social* (Project Settings):

| Setting | Meaning |
|---|---|
| Fame | Scale holding fame |
| Notoriety | Scale holding global notoriety |
| Titles | Every title evaluated on fact changes |
| Factions | Factions listed by the debug HUD and commands, the menus (`Get Factions`) and disguise lookups (`Get Disguise Faction`) |
| Relationships | Character definitions the menus list once their relationship scale has been set (`Get Known Characters`) |
| Disguise Root | Tags under this root count as disguises |
| Disguise Recognition | Fraction of fame and notoriety onlookers read while the player is disguised |

Ranges come from *FlickerVoid > Facts*; there is no separate max notoriety.

## 6. Conditions and effects

Generic (FVFramework): **Scale** (compare a scale's value; at-least comparisons are scored so titles and other rules rank by them) and **Modify Scale** (changes a scale within its range and passes the change on through its links once). Use them for standing, faction notoriety, fame, global notoriety and relationships with a fixed character.

Game rules (AngelScript `UFVCondition`/`UFVEffect` subclasses; until the condition sets hold objects directly, pick **Object** in a condition set or effect list and choose the class inside it):

| Conditions | Effects |
|---|---|
| Attitude Condition (observer's faction, or an override, toward the other actor is at least an attitude), Disguised As Condition, Relationship Condition (a character's, or the context target's, relationship), Recognized Fame Condition (fame as onlookers read it on the instigator), Has Title Condition | Modify Relationship Effect (a character's, or the target's), Promote Title Effect (makes a title current, earning it first if needed) |

In dialogue the observer for *Attitude* is the Target (the NPC); in an NPC's StateTree it is the Instigator (the NPC itself).

From Blueprint/AngelScript use `UFVSocialLibrary` (`FVSocial::` in AngelScript): `Get Fame Scale`, `Get Notoriety Scale`, `Get Relationship Scale`, `Get Attitude(Faction, Actor)`, `Get Recognized Fame`, `Get Recognized Notoriety`, `Is Disguised As`, `Get Actor Faction`, `Get Gift Relationship Change`, `Receive Gift`. Read and change the scales with `UFVScaleLibrary` (`FVScale::GetValue`, `GetBand`, `GetRange`, `ModifyScale`). `UFVSocialSubsystem` keeps the titles: `Get Current Title`, `Get Held Titles` (current first), `Promote Title`, `Evaluate Titles` (call it when an attribute a title checks changes, since titles re-check only on fact changes), and fires **On Title Earned** (first time) and **On Current Title Changed** for notifications.

## 7. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Social.Debug.HUD 1` | Standing, fame, notoriety, attitudes and titles on screen |
| `FV.Social.Standing <FactionIdOrAsset> <Delta>` | Change standing (passes on through links) |
| `FV.Social.Fame <Delta>` | Change fame |
| `FV.Social.Notoriety <Delta>` | Change global notoriety |
