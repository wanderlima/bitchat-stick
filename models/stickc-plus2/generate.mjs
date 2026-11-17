import * as T from 'three';
import { MeshBVH, acceleratedRaycast } from 'three-mesh-bvh';
import { Brush, Evaluator, SUBTRACTION } from 'three-bvh-csg';
import { GLTFExporter } from 'three/addons/exporters/GLTFExporter.js';
import { writeFile } from 'node:fs/promises';

// Authoring units: mm. Export units: metres (glTF convention).
// Front = +Z, top / 8-pin HAT = +Y, USB/Grove end = -Y.
const scene = new T.Scene();
const root = new T.Group();
root.name = 'StickC_Plus2';
root.userData = { dimensionsMm: [24,48,13.5], fidelity: 'External visual reconstruction; see README.md' };
scene.add(root);
const materials = Object.fromEntries(Object.entries({
  shell: [0xffbc19,.38,0], black: [0x17191b,.5,0], glass: [0x070d12,.15,.12],
  metal: [0xb2bbc2,.28,.85], white: [0xebe9dd,.6,0], gold: [0xa98539,.35,.7],
}).map(([name,[color,roughness,metalness]]) => [name,new T.MeshStandardMaterial({ name, color,roughness,metalness })]));
const evaluator = new Evaluator();
evaluator.attributes = ['position','normal'];
evaluator.useGroups = false;

function round(w,h,r) {
  const s = new T.Shape();
  const x=-w/2,y=-h/2;
  s.moveTo(x+r,y); s.lineTo(x+w-r,y); s.quadraticCurveTo(x+w,y,x+w,y+r);
  s.lineTo(x+w,y+h-r); s.quadraticCurveTo(x+w,y+h,x+w-r,y+h);
  s.lineTo(x+r,y+h); s.quadraticCurveTo(x,y+h,x,y+h-r);
  s.lineTo(x,y+r); s.quadraticCurveTo(x,y,x+r,y); s.closePath();
  return s;
}
function extrude(shape,depth,bevel=0) {
  const g=new T.ExtrudeGeometry(shape,{depth:depth-2*bevel,steps:1,curveSegments:12,
    bevelEnabled:bevel>0,bevelThickness:bevel,bevelSize:bevel,bevelSegments:3});
  g.translate(0,0,-depth/2+bevel); return g;
}
function rounded(w,h,d,r=.5,b=0) { return extrude(round(w-2*b,h-2*b,Math.max(.05,r-b)),d,b); }
function mesh(name,g,mat,pos=[0,0,0],rotation=[0,0,0]) {
  const m=new T.Mesh(g,materials[mat]);m.name=name;m.position.set(...pos);m.rotation.set(...rotation);root.add(m);return m;
}
function cut(m,g,pos,rot=[0,0,0]) {
  const a=new Brush(m.geometry,materials.shell),b=new Brush(g,materials.shell);
  a.position.copy(m.position); a.rotation.copy(m.rotation);b.position.set(...pos);b.rotation.set(...rot);
  a.updateMatrixWorld();b.updateMatrixWorld();
  const result=evaluator.evaluate(a,b,SUBTRACTION);
  m.geometry.dispose(); m.geometry=result.geometry;
  m.geometry.applyMatrix4(a.matrixWorld.clone().invert());
  // Evaluator emits world coordinates; restore mesh-local coordinates.
}
const front=mesh('housing_front',rounded(24,48,6.6,3,.3),'shell',[0,0,3.45]);
const back=mesh('housing_rear',rounded(24,48,6.7,3,.3),'shell',[0,0,-3.4]);
// Visible separation of the two mouldings: 0.2 mm seam.
cut(front,rounded(16,26,2,.8),[0,8,6.7]);
mesh('display_bezel',rounded(15.95,25.95,.4,.75),'black',[0,8,5.9]);
mesh('display_glass',rounded(14.85,24.8,.16,.3),'glass',[0,8,6.18]);
// Button A is an integral tongue, with a U-shaped clearance (not a separate floating cap).
const slot=new T.Shape();
slot.moveTo(-7.45,-17.7);slot.lineTo(-7.45,-11.2);slot.quadraticCurveTo(-7.45,-9.4,-5.65,-9.4);
slot.lineTo(5.65,-9.4);slot.quadraticCurveTo(7.45,-9.4,7.45,-11.2);slot.lineTo(7.45,-17.7);
slot.lineTo(7.12,-17.7);slot.lineTo(7.12,-11.2);slot.quadraticCurveTo(7.12,-9.73,5.65,-9.73);
slot.lineTo(-5.65,-9.73);slot.quadraticCurveTo(-7.12,-9.73,-7.12,-11.2);slot.lineTo(-7.12,-17.7);slot.closePath();
cut(front,extrude(slot,1.6),[0,0,6.5]);
function polygon(points) { const s=new T.Shape();points.forEach(([x,y],i)=>i?s.lineTo(x,y):s.moveTo(x,y));s.closePath();return s; }
// Lower rear step visible in the side elevation: approximately 2.4 mm deep,
// flat for the last 6.8 mm, with a 2.5 mm inclined transition above it.
// This recess spans the full width, independently of the central Grove opening.
const rearStep=extrude(polygon([[-26,-10],[-14.7,-10],[-14.7,-6.75],[-17.2,-4.35],[-26,-4.35]]),30);
rearStep.applyMatrix4(new T.Matrix4().makeBasis(
 new T.Vector3(0,1,0),new T.Vector3(0,0,1),new T.Vector3(1,0,0)));
cut(back,rearStep,[0,0,0]);
// Side apertures and inset controls, reconstructed from un-dimensioned views.
for(const [name,side,y,h] of [['B',1,8.2,5],['C',-1,-14.5,5]]) {
  const target=front;
  cut(target,rounded(4.9,h,4,.65),[side*11.7,y,1.1],[0,Math.PI/2,0]);
  cut(back,rounded(4.9,h,4,.65),[side*11.7,y,1.1],[0,Math.PI/2,0]);
  mesh(`button_${name}`,rounded(4.3,h-.55,.65,.45),'shell',[side*11.65,y,1.1],[0,Math.PI/2,0]);
}
for(const [name,y,h] of [['IR_window',16.1,3.9],['microphone',-3.8,1.25],['buzzer_slot',-8.3,3.5]]) {
  cut(front,rounded(1.7,h,3,.35),[-11.75,y,1.3],[0,Math.PI/2,0]);
  mesh(name,rounded(1.4,h-.3,.15,.25),'black',[-11.6,y,1.3],[0,Math.PI/2,0]);
}
// End connectors. Openings are cut through housing, not painted rectangles.
for(const m of [front,back]) {
  cut(m,rounded(9.3,3.5,6,1.35),[0,-23,3.7],[Math.PI/2,0,0]);
  cut(m,rounded(12.1,8.5,7,.45),[0,-23,-3.2],[Math.PI/2,0,0]);
}
function frame(name,w,h,r,depth,wall,mat,pos,rotation) {
  const s=round(w,h,r);const hole=round(w-2*wall,h-2*wall,Math.max(.1,r-wall));
  s.holes.push(new T.Path(hole.getPoints(12).reverse()));
  return mesh(name,extrude(s,depth),mat,pos,rotation);
}
frame('USB_C_shield',8.9,3.15,1.3,3.5,.3,'metal',[0,-22.25,3.7],[Math.PI/2,0,0]);
mesh('USB_C_tongue',rounded(6.6,.65,2,.2),'black',[0,-23,3.7],[Math.PI/2,0,0]);
for(let i=0;i<12;i++) mesh(`USB_contact_${i+1}`,new T.BoxGeometry(.22,1,.06),'gold',[-2.75+i*.5,-23,4.06]);
frame('Grove_socket',11.6,6.1,.35,5.2,.85,'white',[0,-21.3,-2.5],[Math.PI/2,0,0]);
mesh('Grove_inner',new T.BoxGeometry(9.8,.25,4.2),'black',[0,-18.85,-2.5]);
for(let i=0;i<4;i++) mesh(`Grove_pin_${i+1}`,new T.BoxGeometry(.5,3,.5),'metal',[-3+i*2,-21,-2.5]);
// Header: eight sockets at 2.54 mm pitch on the upper rear edge.
cut(back,rounded(21.9,4.2,5,.2),[0,23,-4.15],[Math.PI/2,0,0]);
const header=mesh('HAT_header',new T.BoxGeometry(21.6,3.4,3.7),'black',[0,22.3,-4.15]);
for(let i=0;i<8;i++) {
 const x=(i-3.5)*2.54;
 cut(header,new T.BoxGeometry(.95,5,.95),[x,23,-4.15]);
 frame(`HAT_socket_${i+1}`,1.25,1.25,.08,.8,.18,'gold',[x,23.5,-4.15],[Math.PI/2,0,0]);
}
// Rear label platform; typography is intentionally omitted from the geometry.
cut(back,rounded(20.4,35.5,.5,1),[0,5.7,-6.75]);
mesh('rear_label',rounded(20.3,35.4,.08,1),'white',[0,5.7,-6.52]);
function cylinder(radius,depth) {const g=new T.CylinderGeometry(radius,radius,depth,40);g.rotateX(Math.PI/2);return g;}
for(const x of [-8,8]) {
 for(const [y,type] of [[-14.6,'mount'],[-19.5,'screw']]) {
  cut(back,cylinder(type==='mount'?1.05:1.65,9),[x,y,-4]);
  frame(`${type}_recess_${x}`,3.9,3.9,1.95,.22,.45,'shell',[x,y,type==='screw'?-4.17:-6.57],[0,Math.PI,0]);
  if(type==='mount') {
   frame(`thread_insert_${x}`,2.2,2.2,1.1,2.5,.35,'gold',[x,y,-5.1],[0,0,0]);
  } else {
   const screw=mesh(`screw_${x}`,cylinder(1.45,.7),'metal',[x,y,-3.65]);
   cut(screw,new T.BoxGeometry(1.6,.4,.8),[x,y,-4]);
   cut(screw,new T.BoxGeometry(.4,1.6,.8),[x,y,-4]);
  }
 }
}
root.scale.setScalar(.001);
root.updateMatrixWorld(true);
const bounds=new T.Box3().setFromObject(root),size=bounds.getSize(new T.Vector3());
if(Math.abs(size.x-.024)>1e-6 || Math.abs(size.y-.048)>1e-6 || Math.abs(size.z-.0135)>1e-6)
 throw new Error(`Unexpected dimensions ${size.toArray()}`);
let meshes=0,triangles=0;
root.traverse(o=>{if(o.isMesh){o.geometry.deleteAttribute('uv');meshes++;const p=o.geometry.getAttribute('position');
 for(let i=0;i<p.array.length;i++) if(!Number.isFinite(p.array[i])) throw new Error(`Invalid vertex ${o.name}`);
 triangles+=(o.geometry.index?.count??p.count)/3;}});
// Minimal Node FileReader adapter for the official exporter (no textures).
globalThis.FileReader=class {
 readAsArrayBuffer(blob){blob.arrayBuffer().then(result=>{this.result=result;this.onloadend?.();});}
};
const exporter=new GLTFExporter();
await writeFile(new URL('stickc-plus2.glb',import.meta.url),Buffer.from(await exporter.parseAsync(scene,{binary:true})));
// Remove false feature edges caused by T-junctions in coplanar CSG triangles.
function contours(geometry) {
 const edges=new T.EdgesGeometry(geometry,25),p=edges.getAttribute('position'),out=[];
 const probe=new T.Mesh(geometry,new T.MeshBasicMaterial({side:T.DoubleSide}));
 geometry.boundsTree=new MeshBVH(geometry); probe.raycast=acceleratedRaycast;
 const ray=new T.Raycaster();ray.firstHitOnly=true;
 const axes=[new T.Vector3(1,0,0),new T.Vector3(0,1,0),new T.Vector3(0,0,1)];
 for(let i=0;i<p.count;i+=2) {
  const a=new T.Vector3().fromBufferAttribute(p,i),b=new T.Vector3().fromBufferAttribute(p,i+1);
  const mid=a.clone().add(b).multiplyScalar(.5),edge=b.clone().sub(a).normalize();
  let internal=false;
  for(const axis of axes) {
   const sideways=new T.Vector3().crossVectors(edge,axis);
   if(sideways.lengthSq()<.01)continue;sideways.normalize().multiplyScalar(.003);
   const hits=[];
   for(const sign of [-1,1]) {
    ray.set(mid.clone().addScaledVector(sideways,sign).addScaledVector(axis,100),axis.clone().negate());
    hits.push(ray.intersectObject(probe,false)[0]);
   }
   const [h,k]=hits;
   if(h&&k&&h.point.distanceTo(mid)<.012&&k.point.distanceTo(mid)<.012&&
     h.face.normal.dot(k.face.normal)>.99999 && Math.abs(h.distance-k.distance)<.0001) {internal=true;break;}
  }
  if(!internal)out.push(...a.toArray(),...b.toArray());
 }
 edges.dispose();probe.material.dispose();
 return new T.BufferGeometry().setAttribute('position',new T.Float32BufferAttribute(out,3));
}
// A second asset with feature edges, avoiding triangles across flat display/panel faces.
const wireScene=new T.Scene();const wire=new T.Group();wire.name='StickC_Plus2_contours';wireScene.add(wire);
root.traverse(o=>{if(o.isMesh){const lines=new T.LineSegments(contours(o.geometry),new T.LineBasicMaterial({color:0xffffff}));
 lines.name=o.name;lines.applyMatrix4(o.matrixWorld);wire.add(lines);}});
await writeFile(new URL('stickc-plus2-contours.glb',import.meta.url),Buffer.from(await exporter.parseAsync(wireScene,{binary:true})));
console.log(JSON.stringify({meshes,triangles,dimensionsMm:size.multiplyScalar(1000).toArray()},null,2));
