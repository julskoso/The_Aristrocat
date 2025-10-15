# Hex Grid Runtime Support

This project now includes native code support for hex tiles that mirrors the requested Blueprint-only workflow. You can create Blueprint assets derived from the provided classes to configure tiles and grid managers directly inside the editor.

## New runtime types

| Asset | Description |
| --- | --- |
| `FHexCoord` | A Blueprint-visible axial coordinate struct with integer `Q` and `R` members. |
| `AHexTile` | Actor with a `HexMesh` component, editable axial coordinates, and helper functions for grid registration and selection highlight logic. |
| `AHexGridManager` | Abstract actor exposing a `RegisterTile` Blueprint event so grid logic can stay in Blueprint. |

## Usage

1. **Create the struct asset**
   * In the Content Browser, add a *Blueprint Structure* asset named `ST_HexCoord` and choose `FHexCoord` as its base type. The struct is already Blueprint-ready, so you only need to create the asset wrapper if you want a content browser entry.
2. **Create the tile Blueprint**
   * Add a new *Blueprint Class* and pick `AHexTile` as the parent.
   * Assign your static mesh to the `HexMesh` component.
   * Optionally expose additional editor-only details, but the `AxialQ` and `AxialR` properties are already editable and spawn-exposed.
3. **Grid manager Blueprint**
   * Derive a Blueprint from `AHexGridManager` to keep grid logic in Blueprint.
   * Implement the `RegisterTile` event to receive the coordinate/actor pair from tiles.

The runtime code automatically:

* Registers each tile with the first grid manager found in the world on `BeginPlay`.
* Keeps track of selection state and toggles Custom Depth rendering plus stencil value for outline effects when `SetSelected` is called.

> **Note:** Because this implementation is written in C++, rebuilding project files is required (`Generate Visual Studio project files` or `./GenerateProjectFiles.sh`). After compiling once, the new types appear in the editor and can be used in Blueprints without further coding.
