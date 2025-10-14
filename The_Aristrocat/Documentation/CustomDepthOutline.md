# Custom Depth Outline Setup (Blueprint Only)

This document describes how to implement a white outline post-process effect in Unreal Engine using Custom Depth/Stencil. The implementation relies entirely on Blueprint-accessible settings and assets.

## 1. Enable custom depth rendering

1. Open **Edit → Project Settings**.
2. Navigate to **Rendering → Postprocessing**.
3. Set **Custom Depth-Stencil Pass** to **Enabled with Stencil**.

## 2. Create the outline material

1. In the Content Browser, create a new **Material** named `M_PP_Outline`.
2. Set the following material properties:
   * **Material Domain**: `Post Process`
   * **Blendable Location**: `Before Tonemapping`
   * **Blendable Output Alpha**: Disabled (default)
3. Add the following nodes to the material graph:
   * **SceneTexture** node sampling **CustomDepth**. (In UE5 this may require setting the SceneTexture ID to `PostProcessInput0`.)
   * **SceneTexture** node sampling **CustomStencil**.
   * `ScalarParameter` named `Thickness` with a default value of `1.0`. This controls the size of the UV offsets.
   * Four `ScreenPosition`-based UV offsets: `(±Thickness, 0)` and `(0, ±Thickness)` in normalized screen UV space (divide Thickness by viewport size if necessary). Sample `CustomDepth` at each offset.
   * Subtract the center depth sample from each offset sample and take the absolute value. Accumulate the differences and compare against a small threshold (e.g., `0.001`).
   * Multiply the resulting edge mask by a comparison against the `CustomStencil` value equal to `1` (use `Equal` node) to ensure only meshes with the stencil value are outlined.
   * Feed the final mask into **Emissive Color** multiplied by a constant color (white: `(1,1,1)`).

The material outputs white emissive pixels on stencil transitions that have detectable depth or normal discontinuity.

## 3. Add the post process volume

1. Place a **PostProcessVolume** actor in the level.
2. Enable **Infinite Extent (Unbound)** so the effect applies everywhere.
3. Under the **Blendables** section, add the `M_PP_Outline` material instance.

## 4. Tag meshes for outlining

1. Select any static mesh actors that should receive an outline.
2. In the **Details** panel, under **Rendering**, enable **Render CustomDepth Pass**.
3. Set **CustomDepth Stencil Value** to `1`.

Meshes with the stencil value of `1` will now render with a thin white outline controlled by the `Thickness` parameter.

## 5. Optional: expose thickness control

Create a **Material Instance** from `M_PP_Outline` so that you can adjust the `Thickness` parameter without editing the base material.

Adjusting the `Thickness` value will widen or narrow the outline. Larger values increase the screen-space offset, producing a thicker outline.

## 6. Testing checklist

* Play the level and confirm that meshes with `CustomDepth` enabled display a white outline.
* Verify that disabling **Render CustomDepth Pass** or changing the stencil value removes the outline.
* Adjust the `Thickness` parameter in the material instance to confirm the outline width responds as expected.

