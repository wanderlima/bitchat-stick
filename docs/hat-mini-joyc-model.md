# Hat Mini JoyC — visual 3D reconstruction

M5Stack U156, modelled separately from the StickC. For Three.js and wireframe visualization; no animation or website changes.

## Files

- `hat-mini-joyc.glb`: 44 named meshes, materials, approximately 69,416 triangles, no textures.
- `hat-mini-joyc-contours.glb`: feature-edge line segments for wireframe-style rendering. Some CSG junctions can remain visible.
- `views/overview.png`: eight-view contact sheet; individual PNGs are alongside it.
- `generate.mjs`: editable procedural geometry. Run `npm install` then `npm run build` in this directory. The existing StickC GLB in the sibling directory is required.

Delivery copies are in `docs/hat-mini-joyc.glb`, `docs/hat-mini-joyc-contours.glb`, and `docs/hat-mini-joyc-views.png`.

## Geometry and dimensions

The positive lower seating step is derived from the actual StickC GLB in `../stickc-plus2/stickc-plus2.glb`. The prior visually lowered step has been replaced: imported rear-surface raycasts yield Z=-9.7 mm for the main seat and Z=-7.3 mm for the lower land, restoring the 2.4 mm advance. The ramp runs from Y=-31.55 to -34.05 mm. The central clearance is Boolean-subtracted using the fitted StickC geometry, including the Grove connector, rather than a hand-drawn blind recess. The upper receiving shoulder is also trimmed against the fitted StickC.

The StickC uses translation [0, -16.85, -2.95] mm in JoyC coordinates. Both lower mount centres and all eight pin/socket axes are checked in the generator. HAT pins were repositioned to match the imported sockets, and the exposed battery moved back 0.3 mm to clear the rear shell. These checks establish consistency between the two reconstructed models, not a manufacturing tolerance guarantee.

`stickc-plus2-joyc-assembled.glb` contains both models in the fitted position, with separate named roots. `assembled-views/` contains eight rendered views and their contact sheet.

The joystick base is a single continuous moulding with a smoothly blended U-shaped saddle, a rounded receiving lip and a circular crown. This shape was revised using the user's eight photos IMG_7337–IMG_7344. The black cap remains outside the housing: the previously inferred lower black sphere and matching internal bearing pocket were removed after the user's clarification. The cap underside is checked at Z=+1.6 mm in authoring coordinates. These photo-derived curves are visual estimates. These mating details are reconstructed visually and do not certify physical fit.

The model includes the open receiving tray, indexed side rails, mounting bosses, battery pouch and tape, two wires, eight HAT pins at 2.54 mm pitch, pedestal, joystick skirt, stem and thumb cap, four grip dimples, Grove connector, rear screw heads, light pipe and lanyard opening. Joystick, cap, pins and battery are named separately for future selection/animation. No StickC is included. Printed labels are plain surfaces; no PCB, battery internals, screw threads or full internal joystick mechanism are reconstructed. The joystick is in its neutral position.

Authoring coordinates are in millimetres, exported GLB coordinates in **metres**. +Y points towards the joystick / Grove end, +Z towards the joystick and StickC receiving face. The chassis extends from Y=-40.55 to +40.55 mm. Origin Z is at the midpoint between the back and thumb-cap extremes.

The official page lists **81.1 × 36 × 24 mm**. The drawing dimensions the chassis length approximately 81 mm, chassis width 24 mm, overall side depth 36 mm, mounting centres 16 mm apart, joystick axis to lower mounting row 60 mm, rear upper/lower screw-row spacing 52 mm and pin pitch 2.54 mm.

The drawing also shows the joystick skirt overhanging the 24 mm chassis, including above its end. This reconstruction follows that outline rather than squeezing the skirt into the nominal box: the exported **complete mesh bounds are approximately 83.6 mm long × 29 mm wide × 36 mm deep**. The skirt diameter (~29 mm) and 2.5 mm longitudinal overhang are inferred from image proportions, not explicit manufacturer dimensions. These choices are estimates and may need revision if manufacturer CAD or physical measurements become available.

Chassis depths, rail shapes, bevels, cap profile, dimple positions, battery dimensions, fastener diameters, connector recess depths and fine exterior details are estimated from the drawing and photographs. This is a visual reconstruction, not manufacturer CAD or a certified fit model.

## Validation

All three GLBs passed the Khronos glTF validator with **zero errors, warnings or informational findings**. Finite coordinates, full bounds, mating surface heights, both mounting centres, all eight HAT contact axes and the joystick side-relief surface are checked in the generator. The solid GLB was loaded through Three.js GLTFLoader and rendered from eight angles in Chrome.

## Load in Three.js

```js
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
const { scene: joyc } = await new GLTFLoader().loadAsync('/models/hat-mini-joyc.glb');
scene.add(joyc);
// Load hat-mini-joyc-contours.glb instead for feature-edge line segments.
```

## Sources

- Product: https://docs.m5stack.com/en/hat/Hat_Mini_JoyC
- Dimension drawing: https://static-cdn.m5stack.com/resource/docs/products/hat/MiniJoyC/img-14cbb54f-a375-4e6d-b73f-0d6bf0f53130.png
- Official front / open tray photograph: https://static-cdn.m5stack.com/resource/docs/products/hat/MiniJoyC/img-919f4187-6276-48ce-9a6f-6bbd9ec1ac19.webp
- Official rear photograph: https://static-cdn.m5stack.com/resource/docs/products/hat/MiniJoyC/img-a7071d84-fa98-4b70-9aa6-0edd8381f7b1.webp
- Official side perspective: https://static-cdn.m5stack.com/resource/docs/products/hat/MiniJoyC/img-0e907dcb-985c-48dc-9c26-b21339f576e0.webp

Geometry was reconstructed for this project. M5Stack product design and trademarks remain with their respective owners.
