# StickC-Plus2 — external 3D reconstruction

Model for Three.js / web visualization. No animation or website changes are included.

## Assets

- `stickc-plus2.glb`: solid meshes, with materials and 47 named parts.
- `stickc-plus2-contours.glb`: line segments for a transparent technical outline. These are feature edges, not every triangle edge. Some CSG junctions remain visible.
- `preview.png`: front, rear and contour render of the exported assets.
- `generate.mjs`: editable, procedural source using Three.js and CSG.

The solid contains about 17,675 triangles, no textures, no animations, and no electronics inside. Button A is a tongue in the front housing; its M5 mark is engraved geometry. Rear label is a plain inset, without printed text. Mount inserts have no helical thread geometry.

## Dimensions and confidence

Authoring uses millimetres; exported glTF uses **metres**. Overall bounds: **24 × 48 × 13.5 mm**. Origin is the centre; +X is right viewed from the front, +Y points towards the HAT header, +Z faces the screen. Pieces are named for later selection and animation.

Explicit drawing dimensions used: 24 mm width, 48 mm length, 13.5 mm depth, R3 outline corners, 16 mm between rear hole centres, hole rows 9.4 mm and 4.5 mm from the USB end. Header pitch is 2.54 mm per the official product image.

The lower rear shell includes the full-width step and inclined transition visible in the side elevation (estimated 2.4 mm depth, 6.8 mm lower flat and 2.5 mm transition). Lower screws follow the recessed surface.

Screen opening (~16 × 26 mm), controls, shell seam, wall depths, connector recesses, screw diameters, front/back bevels and the M5 mark were reconstructed from image proportions. Their positions/depths are **estimates**, not manufacturer CAD. Side feature shapes/positions and rear details are a visual approximation. The drawing is published under the older Plus path, but is also linked in the Plus2 documentation; photographs of Plus2 were used for the external appearance.

This is a visual reference model, not a manufacturing or fit-certified solid. It does not reproduce PCB, battery, hidden mounting structures, printed labels or every moulding detail.

## Rebuild

```sh
cd models/stickc-plus2
npm install
npm run build
```

The generator checks overall bounds and finite vertex coordinates before exporting. It uses the official GLTFExporter. Both GLBs passed the Khronos glTF validator with zero errors and warnings. Assets were also loaded and rendered with Three.js GLTFLoader in Chrome.

## Load in Three.js

```js
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
const { scene: device } = await new GLTFLoader().loadAsync('/models/stickc-plus2.glb');
scene.add(device);
// For outlines, load stickc-plus2-contours.glb instead.
// A camera distance around 0.12 m is appropriate for this metre-scaled model.
```

## Sources

- Product: https://docs.m5stack.com/en/core/StickC-Plus2
- Drawing: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/products/core/m5stickc_plus/%E5%B0%BA%E5%AF%B8%E5%9B%BE.jpg
- Official photos: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/products/core/M5StickC%20PLUS2/4.webp and `5.webp`, `6.webp` in the same directory.

Geometry was reconstructed for this project; no ownership of M5Stack trademarks or product design is implied.
