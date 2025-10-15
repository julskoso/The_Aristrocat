# Custom Depth Post Process Outline Setup

This project uses a post process material that renders white outlines around meshes that are using the Custom Depth pass with Stencil value `1`. The material samples the Custom Depth and Custom Stencil buffers, performs a four tap edge detection with a configurable thickness parameter, and outputs the outline colour via the emissive channel.

## Engine configuration

Project rendering is configured for Custom Depth with Stencil support by enabling the `r.CustomDepth=3` console variable in `DefaultEngine.ini`. Temporal jitter for the Custom Depth buffer is also disabled so that the post process comparison remains stable frame-to-frame.

## Post process material

The material graph (created as `M_PP_Outline`) performs the following high-level steps:

1. Read Custom Stencil to use as a binary mask (`CustomStencil == 1`).
2. Sample Custom Depth in a cross pattern (current pixel, ±X, ±Y offsets derived from the `Thickness` parameter).
3. Compare the sampled depth values to detect where the mask transitions, outputting white when an edge is found and black otherwise.
4. Write the result to the emissive channel so it contributes additively on top of the scene before tonemapping.

The material exposes a scalar parameter named `Thickness` that defaults to `1.0`. Increasing this value widens the offset used for edge taps to thicken the outline.

## Level setup

A global Post Process Volume is placed in the persistent level with *Infinite Extent (Unbound)* enabled. The outline material is added to the volume's blendables so it affects the entire level.

To highlight a mesh, enable **Render CustomDepth Pass** on the mesh component and set **CustomDepth Stencil Value** to `1`.

## Testing notes

To validate the effect, enable the Custom Depth pass on a static mesh in the level. The mesh should display a thin white outline that respects the `Thickness` parameter.
