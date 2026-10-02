# Copilot Instructions

## Project Guidelines
- Keep code clean and simple; do not add explanatory comments in code changes unless absolutely necessary; at most two comments total.
- When requesting a "simple" implementation, provide the most minimal direct code (a few lines, no extra helper functions, properties, or abstractions).
- Delegate naming: delegate types and members have no "On" prefix (e.g. FOffersChanged, OffersChanged), while handler functions use the "On" prefix (e.g. OnOffersChanged).

## Build Instructions
- Use the installed engine build located at `C:\src\repos\FVUnrealEngine\LocalBuilds\Engine\Windows`.
- Build using `LocalBuilds\Engine\Windows\Engine\Build\BatchFiles\Build.bat` (never use the source `Engine\Build\BatchFiles\Build.bat`, which rebuilds the whole engine).