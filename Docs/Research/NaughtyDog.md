### Naughty Dog (Last of Us, Uncharted)

Key Patterns from Naughty Dog:
- **Encounter System**: Self-contained gameplay segments
- **Message Bus**: Decoupled communication between systems
- **State Machines**: Central to all behavior, not just AI

```mermaid
flowchart TD
subgraph Core["Core Systems"]
	DC[Data Compiler]
	STATE[State Machines]
	MSG[Message Bus]
end
subgraph Gameplay["Gameplay"]
    PLAYER[Player Systems]
    WEAPON[Weapon System]
    COVER[Cover System]
end

subgraph AI["AI"]
    PLANNER[AI Planner]
    NAV[Navigation]
    PERCEPT[Perception]
end

subgraph Narrative["Narrative"]
    ENCOUNTER[Encounter System]
    SCRIPT[Scripting]
    CONV[Conversations]
end

Core --> Gameplay
Core --> AI
Core --> Narrative
Gameplay --> AI
AI --> Narrative
```