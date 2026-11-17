import * as THREE from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { GLTFExporter } from 'three/addons/exporters/GLTFExporter.js';
import { readFile, writeFile } from 'node:fs/promises';

// Export the approved outer geometry, in metres, with a shared joystick pivot.
const output = new THREE.Scene();
const body = new THREE.Group(); body.name = 'wire-device'; output.add(body);
const joystick = new THREE.Group(); joystick.name = 'joystick-pivot';
joystick.position.set(0, .02855, .002); body.add(joystick);
const depth = new THREE.MeshBasicMaterial({ color: 0x000000 });
const line = new THREE.LineBasicMaterial({ color: 0xebdcba });
const loader = new GLTFLoader();
const external = /^(housing_|display_|button_[ABC]|joystick_|chassis_|tray_|Grove|USB_C|mount_boss|rear_label|rear_screw|screw_)/;
for (const [prefix, offset] of [['hat-mini-joyc', [0, 0, 0]], ['stickc-plus2', [0, -.01685, -.00295]]]) {
  for (const contours of [false, true]) {
    const bytes = await readFile(new URL(`../../../../m5stickc-plus2-models/${prefix}/${prefix}${contours ? '-contours' : ''}.glb`, import.meta.url));
    const source = (await loader.parseAsync(bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength), '')).scene;
    source.position.set(...offset); source.updateMatrixWorld(true);
    source.traverse(part => {
      if (!(part.isMesh || part.isLineSegments) || !external.test(part.name)) return;
      const geometry = part.geometry.clone().applyMatrix4(part.matrixWorld);
      geometry.deleteAttribute('uv'); geometry.deleteAttribute('normal');
      const moving = /^joystick_(skirt|stem|thumb_cap)$/.test(part.name);
      if (moving) geometry.translate(0, -.02855, -.002);
      const object = contours ? new THREE.LineSegments(geometry, line) : new THREE.Mesh(geometry, depth);
      object.name = `${prefix}_${part.name}_${contours ? 'lines' : 'depth'}`;
      object.userData = { role: contours ? 'lines' : 'depth', part: part.name };
      (moving ? joystick : body).add(object);
    });
  }
}
globalThis.FileReader = class {
  readAsArrayBuffer(blob) { blob.arrayBuffer().then(result => { this.result = result; this.onloadend?.(); }); }
};
const glb = await new GLTFExporter().parseAsync(output, { binary: true });
await writeFile(new URL('../../public/models/bitchat-stick-wire.glb', import.meta.url), Buffer.from(glb));
console.log(`Wireframe scene: ${(glb.byteLength / 1024).toFixed(0)} KiB`);
