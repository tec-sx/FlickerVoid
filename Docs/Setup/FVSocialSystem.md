# FVSocialSystem setup

Factions and the player's standing with them, one global fame and one global notoriety, titles the world gives the player, personal relationships, and disguises. Every value is a fact, so it saves and works in conditions. Nothing decays over time.

Modules: `FVSocialSystem`, `FVSocialSystemDebug`, `FVSocialSystemEditor`. Uses GameplayAbilities to read disguise tags.

## 1. Quick start

1. Create fact tags: `Fact.Social.Fame`, `Fact.Social.Notoriety`, and per faction `Fact.Faction.<Name>.Standing` and `Fact.Faction.<Name>.Notoriety`.
2. Declare them in *FlickerVoid > Facts* with clamps if you want (standing -100..100, notoriety 0..100).
3. Make a faction definition (section 2) and add it to *FlickerVoid > Social > Factions*.
4. Set *Fame Fact* and *Notoriety Fact* in *FlickerVoid > Social*.
5. Add the **Social** fragment to NPC character definitions to make them faction members.
6. Change standing with the **Modify Standing** effect (quest rewards, dialogue choices) and watch `FVCvar.Social.Debug.HUD 1`.

## 2. Data assets

| Asset | Class | Key fields |
|---|---|---|
| `DA_Faction_<Name>` | `FVFactionDefinition` | **Id** = standing fact. **Min/Max Standing**, **Tiers** (min standing, tier tag, label, attitude; sorted ascending), **Relations** (other faction and the fraction of each change passed on, e.g. Police -0.5 for Gangs), **Fame Contribution**, **Notoriety Contribution** (raise for criminal factions), **Notoriety Fact**, **Hostile Notoriety**, **Disguise Tag**, **Disguise Notoriety Limit** |
| `DA_Title_<Name>` | `FVTitleDefinition` | **Id** = fact set while held. **Conditions** (e.g. fame >= 60 and standing with the dockers >= 40), **On Earned** effects, **Replaces** (lesser titles it removes), **Transient** (lost again when the conditions stop holding) |

Tier tags: create them under your own root, e.g. `Faction.Tier.Hated`, `.Disliked`, `.Neutral`, `.Liked`, `.Trusted`.

### Example faction tiers
| Min Standing | Tier | Attitude |
|---|---|---|
| -100 | Hated | Hostile |
| -40 | Disliked | Unfriendly |
| -10 | Neutral | Neutral |
| 30 | Liked | Friendly |
| 70 | Trusted | Allied |

## 3. Characters

Add the **Social** fragment to an `FVCharacterDefinition`:
- **Faction**: the NPC's faction (used by attitude checks and AI).
- **Relationship Fact**: the fact holding the player's personal relationship (affinity) with this character, e.g. `Fact.Relationship.Mara`. Declare it in Facts settings to give it a range and default.

The NPC actor needs an **FV Identity Component** pointing at that definition.

## 4. Disguises

1. Create disguise tags under a root, e.g. `Disguise.Faction.Police`, and set *FlickerVoid > Social > Disguise Root* to `Disguise`.
2. Set the faction's **Disguise Tag** to `Disguise.Faction.Police`.
3. Give an outfit item an Equippable fragment granting that tag (FVInventorySystem). The equipment component adds it to the wearer's Ability System Component.
4. While worn, police treat the player as a member (Allied) until the notoriety they read reaches the faction's **Disguise Notoriety Limit**, and onlookers read only *Disguise Recognition* (default 25%) of the player's fame and notoriety.

The player needs an Ability System Component for disguises to be read.

## 5. Settings

*FlickerVoid > Social*:

| Setting | Meaning |
|---|---|
| Fame Fact | Fact holding fame |
| Notoriety Fact | Fact holding global notoriety |
| Titles | Every title evaluated on fact changes |
| Factions | Factions listed by the debug HUD and commands |
| Disguise Root | Tags under this root count as disguises |
| Disguise Recognition | Fraction of fame and notoriety onlookers read while the player is disguised |

## 6. Conditions and effects

| Conditions | Effects |
|---|---|
| Faction Standing, Faction Notoriety, Fame (as onlookers read it, or real), Global Notoriety, Has Title, Relationship, Attitude (observer's faction toward the other actor), Is Disguised As | Modify Standing (passes on to related factions once), Modify Notoriety, Modify Fame, Modify Global Notoriety, Modify Relationship |

In dialogue the observer for *Attitude* is the Target (the NPC); in an NPC's StateTree it is the Instigator (the NPC itself).

From Blueprint/AngelScript use `UFVSocialLibrary` (`FVSocial::` in AngelScript): `Get Standing`, `Get Tier`, `Get Attitude(Faction, Actor)`, `Get Recognized Fame`, `Is Disguised As`, `Get Actor Faction`, and the Modify functions. `UFVSocialSubsystem` fires **On Title Earned** and **On Title Lost** for notifications.

## 7. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Social.Debug.HUD 1` | Standing, fame, notoriety, attitudes and titles on screen |
| `FV.Social.Standing <FactionIdOrAsset> <Delta>` | Change standing |
| `FV.Social.Fame <Delta>` | Change fame |
| `FV.Social.Notoriety <Delta>` | Change global notoriety |
