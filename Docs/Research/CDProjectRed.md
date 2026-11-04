### CD Projekt RED

Based on GDC talks and technical analysis:

Key Patterns from CDPR:
- **Facts Database**: Central truth store for world state (similar to your WorldState)
- **Stats System**: Separate from entities, pure data processing
- **Scene System**: Orchestrates dialogue, animations, cameras independently
- **Communities**: NPC scheduling/behavior separate from individual AI

```mermaid
flowchart TD 
subgraph Engine["RED Engine Core"]
	CORE[Core/Foundation]
	RTTI[Reflection/RTTI]
	RES[Resource Management]
end
subgraph Systems["Game Systems"]
    STATS[Stats System]
    FACT[Facts Database]
    JRNL[Journal/Quest]
    INV[Inventory]
    CRAFT[Crafting]
end

subgraph World["World Layer"]
    STRM[Streaming]
    COMM[Communities]
    SPAWN[Spawning]
    ENV[Environment]
end

subgraph Entities["Entity Layer"]
    ENT[Entity Base]
    NPC[NPC System]
    PLAYER[Player]
    VEH[Vehicles]
end

subgraph AI["AI Layer"]
    BEH[Behavior Trees]
    REACT[Reaction System]
    SENSE[Senses]
end

subgraph Narrative["Narrative Layer"]
    SCENE[Scene System]
    DLG[Dialogue]
    CINE[Cinematics]
end

subgraph UI["UI Layer"]
    HUD[HUD]
    MENU[Menus]
    JOURNAL[Journal UI]
end

Engine --> Systems
Engine --> World
Systems --> Entities
World --> Entities
Entities --> AI
Entities --> Narrative
AI --> Narrative
Systems --> UI
Narrative --> UI
```