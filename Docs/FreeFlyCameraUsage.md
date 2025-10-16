# Free-Fly Camera Setup & Usage

This guide covers how to use the Enhanced Input powered free-fly camera pawn inside a level.

## 1. Prerequisites

1. Build the project once after enabling the **Enhanced Input** plugin.
2. Ensure the input assets listed below exist in `Content/` and match the naming in `AFreeFlyCameraPawn`:
   - `IMC_FreeFly` mapping context
   - `IA_Move`, `IA_UpDown`, `IA_Look`, `IA_RMB`, `IA_Sprint`, `IA_Slow`, `IA_SpeedStep`
3. In the `BP_FreeFlyCameraPawn` blueprint (subclass of `AFreeFlyCameraPawn`), assign the mapping context and all input actions in the exposed class defaults.

## 2. Making the Pawn the Active Camera

You can make the pawn active either globally (via maps & modes) or per-level.

### Option A – Project Default
1. Open **Project Settings → Maps & Modes**.
2. Under *Default Modes*, set **Default Pawn Class** to `BP_FreeFlyCameraPawn`.
3. Keep your existing player controller blueprint (e.g. `BP_PlayerController_Hex`) assigned as **Player Controller Class**.
4. Press **Play** in the editor; the pawn will be possessed automatically.

### Option B – Level Override
1. Drop an instance of `BP_FreeFlyCameraPawn` into the level.
2. In the Details panel for the placed pawn, set **Auto Possess Player** to **Player 0**.
3. Optionally disable *Auto Possess AI* so only the human player controls it.
4. Start PIE (Play-In-Editor) to verify possession.

## 3. Verifying the Mapping Context

When PIE starts, `AFreeFlyCameraPawn` registers `IMC_FreeFly` with the `UEnhancedInputLocalPlayerSubsystem` in `BeginPlay` and whenever it is possessed. If movement does not respond:

1. Ensure the pawn instance references the correct mapping context asset.
2. Confirm the local player controller exists (the pawn attempts to cache it on begin play).
3. Check the output log for `Enhanced Input` warnings; missing assets or duplicate contexts will be reported there.

## 4. Controls In-Game

| Action | Input | Notes |
| --- | --- | --- |
| Move | `WASD` | Forward/backward on `Y`, strafe on `X`. |
| Rise / Lower | `E` / `Q` | Moves along world up. |
| Look | Hold Right Mouse Button | Locks cursor, rotates while held. |
| Sprint | Hold Left Shift | Multiplies base speed by `SprintMultiplier`. |
| Slow | Hold Left Ctrl | Multiplies base speed by `SlowMultiplier`. |
| Speed Step | Mouse Wheel Up/Down | Adjusts `BaseSpeed` within `[MinBaseSpeed, MaxBaseSpeed]`. |

## 5. Cursor Behaviour

- When the right mouse button is pressed, the pawn hides the cursor and switches the player controller to **Game Only** input mode so you can look around freely.
- Releasing RMB restores **Game and UI** input mode with the cursor visible, allowing designers to continue interacting with UI or hex tiles.

## 6. Tweaking Movement

In `BP_FreeFlyCameraPawn` you can adjust exposed variables:

- `BaseSpeed` for default travel speed (clamped between `MinBaseSpeed` and `MaxBaseSpeed`).
- `SprintMultiplier` / `SlowMultiplier` for speed modifiers.
- `MouseSensitivity` for look speed.
- `MinBaseSpeed` / `MaxBaseSpeed` to redefine the scroll-wheel range.

After adjusting values, compile and save the blueprint so the defaults propagate to new instances.

## 7. Testing Checklist

1. Play in editor and confirm the pawn spawns.
2. Move with `WASD` and `Q/E`; note acceleration is handled via `UFloatingPawnMovement`.
3. Hold RMB to rotate, release to regain cursor control.
4. Shift/Ctrl to vary speed, mouse wheel to re-scale base speed.
5. Verify left mouse button interactions (hex selection) still work when the cursor is unlocked.

Following these steps ensures the free-fly camera is fully functional in any level.
