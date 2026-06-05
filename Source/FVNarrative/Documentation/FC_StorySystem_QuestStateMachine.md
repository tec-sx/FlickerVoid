# Story System Flowcharts

## Quest State Machine

```mermaid
stateDiagram-v2
	[*] --> Unavailable

	Unavailable --> Available: Prerequisites Met
	Available --> Unavailable: Prerequisites Lost
	
	Available --> Active: StartQuest()
	Active --> Completed: All Objectives Done
	Active --> Failed: Fail Condition Met
	Active --> Abandoned: Player Abandons
	
	Abandoned --> Available: Can Retry
	Abandoned --> Unavailable: Blocked
	
	Completed --> [*]
	Failed --> [*]
```