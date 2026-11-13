import * as T from 'three';
import { MeshBVH, acceleratedRaycast } from 'three-mesh-bvh';
import { Brush, Evaluator, SUBTRACTION, ADDITION } from 'three-bvh-csg';
import { GLTFExporter } from 'three/addons/exporters/GLTFExporter.js';
import { writeFile, readFile } from 'node:fs/promises';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';

// Authoring units: mm. Export units: metres (glTF convention).
// Front = +Z, top / 8-pin HAT = +Y, USB/Grove end = -Y.
const scene = new T.Scene();
const root = new T.Group();
root.name = 'Hat_Mini_JoyC';
root.userData = { nominalDimensionsMm: [24,81.1,36], fidelity: 'External visual reconstruction; see README.md' };
scene.add(root);
const materials = Object.fromEntries(Object.entries({
  shell: [0x444b4c,.48,0], black: [0x0b0d0e,.7,0], glass: [0x070d12,.15,.12],
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
  const g=new T.ExtrudeGeometry(shape,{depth:depth-2*bevel,steps:1,curveSegments:24,
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
function frame(name,w,h,r,depth,wall,mat,pos,rotation) {
  const s=round(w,h,r);const hole=round(w-2*wall,h-2*wall,Math.max(.1,r-wall));
  s.holes.push(new T.Path(hole.getPoints(12).reverse()));
  return mesh(name,extrude(s,depth),mat,pos,rotation);
}
function polygon(points) { const s=new T.Shape();points.forEach(([x,y],i)=>i?s.lineTo(x,y):s.moveTo(x,y));s.closePath();return s; }
function cylinder(radius,depth,segments=48) {const g=new T.CylinderGeometry(radius,radius,depth,segments);g.rotateX(Math.PI/2);return g;}
function lathe(name,points,mat,y,z=0) {
 const g=new T.LatheGeometry(points.map(p=>new T.Vector2(...p)),80);g.rotateX(Math.PI/2);
 return mesh(name,g,mat,[0,y,z]);
}
// Chassis: asymmetric outline, semicircular joystick end and rounded foot.
function chassisOutline(inset=0) {
 const w=12-inset,top=28.55,bottom=-40.55+inset,r=3-inset;
 const s=new T.Shape();s.moveTo(-w,bottom+r);s.quadraticCurveTo(-w,bottom,-w+r,bottom);
 s.lineTo(w-r,bottom);s.quadraticCurveTo(w,bottom,w,bottom+r);s.lineTo(w,top);
 s.absarc(0,top,w,0,Math.PI,false);s.lineTo(-w,bottom+r);s.closePath();return s;
}
const rear=mesh('chassis_back',extrude(chassisOutline(.3),2.2,.3),'shell',[0,0,-16.6]);
// Rearmost face -18 mm, frontmost cap +18 mm. A thin perimeter lip closes the back.
const backRim=mesh('chassis_back_rim',extrude(chassisOutline(.1),.6,.1),'shell',[0,0,-17.7]);
const trayFloor=mesh('tray_floor',rounded(21.2,48.4,1.5,1),'shell',[0,-15.65,-14.75]);
for(const x of [-11.1,11.1]){
 mesh(`tray_side_${x}`,rounded(1.8,48.8,4.1,.55,.15),'shell',[x,-15.5,-13.1]);
 // Short indexing steps on both side rails are visible in the technical side view.
 for(const y of [-29.5,-19.5,-9.5]) cut(root.children.find(o=>o.name===`tray_side_${x}`),new T.BoxGeometry(3,4,1.2),[x,y,-11.1]);
}
mesh('tray_foot',rounded(22,3.4,5.4,1,.2),'shell',[0,-38.7,-12.9]);
// Derive the mating surface from the actual exported StickC, not photo proportions.
const stickBytes=await readFile(new URL('../stickc-plus2/stickc-plus2.glb',import.meta.url));
const stickAsset=await new GLTFLoader().parseAsync(stickBytes.buffer.slice(stickBytes.byteOffset,stickBytes.byteOffset+stickBytes.byteLength),'');
const stick=stickAsset.scene;stick.name='StickC_Plus2_fitted';
// Mount rows: StickC -14.6 -> JoyC -31.45; back plane -6.75 -> boss seat -9.7.
const fitTranslationMm=new T.Vector3(0,-16.85,-2.95);
stick.scale.setScalar(1000);stick.position.copy(fitTranslationMm);stick.updateMatrixWorld(true);
const stickRear=stick.getObjectByName('housing_rear');
function rearSurface(x,y) {
 const ray=new T.Raycaster(new T.Vector3(x,y,-50),new T.Vector3(0,0,1));
 const hit=ray.intersectObject(stickRear,false)[0];
 if(!hit)throw new Error(`Missing StickC mating surface ${x},${y}`);
 return hit.point.z;
}
const upperSeatZ=rearSurface(10,-29);
const lowerSeatZ=rearSurface(10,-37);
if(Math.abs(lowerSeatZ-upperSeatZ-2.4)>.01)throw new Error('StickC rear recess does not match expected 2.4 mm step');
const rampStart=-14.7+fitTranslationMm.y,rampEnd=-17.2+fitTranslationMm.y;
const locatingStepGeometry=extrude(polygon([
 [-40.4,-14.1],[rampStart,-14.1],[rampStart,upperSeatZ],
 [rampEnd,lowerSeatZ],[-40.4,lowerSeatZ]
]),20.6);
locatingStepGeometry.applyMatrix4(new T.Matrix4().makeBasis(
 new T.Vector3(0,1,0),new T.Vector3(0,0,1),new T.Vector3(1,0,0)));
const locatingStep=mesh('lower_positive_locating_step',locatingStepGeometry,'shell');
// Remove only the space occupied by the fitted StickC geometry: this produces
// the true central connector clearance instead of inventing a slot by eye.
function subtractFittedStick(target) {
 target.updateMatrixWorld(true);
 const bounds=new T.Box3().setFromObject(target);
 stick.traverse(part=>{if(part.isMesh&&bounds.intersectsBox(new T.Box3().setFromObject(part))){
  const tool=part.geometry.clone().applyMatrix4(part.matrixWorld);
  cut(target,tool,[0,0,0]);tool.dispose();
 }});
}
subtractFittedStick(locatingStep);
root.userData.fit={translationMm:fitTranslationMm.toArray(),upperSeatZ,lowerSeatZ,rampStart,rampEnd,source:'../stickc-plus2/stickc-plus2.glb'};
// Fuse the positive feature into the tray floor, rather than leaving a floating part.
const floorBrush=new Brush(trayFloor.geometry,materials.shell);
floorBrush.position.copy(trayFloor.position);floorBrush.updateMatrixWorld();
const stepBrush=new Brush(locatingStep.geometry,materials.shell);stepBrush.updateMatrixWorld();
const fusedFloor=evaluator.evaluate(floorBrush,stepBrush,ADDITION);
trayFloor.geometry.dispose();trayFloor.geometry=fusedFloor.geometry;
trayFloor.geometry.applyMatrix4(floorBrush.matrixWorld.clone().invert());
trayFloor.name='tray_floor_with_positive_locating_step';
root.remove(locatingStep);locatingStep.geometry.dispose();

// Lanyard opening at the lower end.
for(const m of [rear,backRim]) cut(m,rounded(4.4,1.8,7,.8),[0,-37.8,-16]);
cut(root.children.find(o=>o.name==='tray_foot'),rounded(4.4,2,9,.8),[0,-39,-13],[Math.PI/2,0,0]);
// Photo-guided continuous moulding: circular crown, raised receiving lip,
// and a smooth U-shaped saddle. No separate bridge or through-cut cavity.
const saddleKnots=[[7,1.6],[10.4,1.6],[13,-1.4],[17.5,-5.8],
 [20.5,-5.9],[24.8,-2.2],[28.55,1.6],[40.55,1.6]];
function saddleProfile(y) {
 let i=0;while(i<saddleKnots.length-2&&y>saddleKnots[i+1][0])i++;
 const [a,b]=[saddleKnots[i],saddleKnots[i+1]];
 const prev=saddleKnots[Math.max(0,i-1)],next=saddleKnots[Math.min(saddleKnots.length-1,i+2)];
 const span=b[0]-a[0],t=T.MathUtils.clamp((y-a[0])/span,0,1);
 const m0=(b[1]-prev[1])/(b[0]-prev[0]),m1=(next[1]-a[1])/(next[0]-a[0]);
 return (2*t**3-3*t*t+1)*a[1]+(t**3-2*t*t+t)*span*m0+
  (-2*t**3+3*t*t)*b[1]+(t**3-t*t)*span*m1;
}
function deckHeight(x,y) {
 const radius=Math.hypot(x,y-28.55);
 const seat=1.6-.65*Math.max(0,radius-8.8)**2;
 const valley=saddleProfile(y);
 // Smooth maximum blends the circular bearing support into the saddle.
 const blend=.55,high=Math.max(seat,valley);
 return Math.min(1.6,high+blend*Math.log(Math.exp((seat-high)/blend)+Math.exp((valley-high)/blend)));
}
function mouldingGeometry() {
 const outline=new T.Shape();outline.moveTo(-10.7,7);outline.lineTo(10.7,7);
 outline.quadraticCurveTo(12,7,12,8.3);outline.lineTo(12,28.55);
 outline.absarc(0,28.55,12,0,Math.PI,false);outline.lineTo(-12,8.3);
 outline.quadraticCurveTo(-12,7,-10.7,7);outline.closePath();
 const edge=outline.getSpacedPoints(160);edge.pop();const n=edge.length,rings=36;
 const positions=[0,23,deckHeight(0,23)],indices=[];
 for(let k=1;k<=rings;k++)for(const p of edge){
  const r=k/rings,x=p.x*r,y=23+(p.y-23)*r;
  // Small rounded roll at the deck perimeter, visible around the raised lip.
  const roll=.28*T.MathUtils.smoothstep(r,.965,1);
  positions.push(x,y,deckHeight(x,y)-roll);
 }
 for(let j=0;j<n;j++)indices.push(0,1+j,1+(j+1)%n);
 for(let k=1;k<rings;k++)for(let j=0;j<n;j++){
  const a=1+(k-1)*n+j,b=1+k*n+j,c=1+k*n+(j+1)%n,d=1+(k-1)*n+(j+1)%n;
  indices.push(a,b,c,a,c,d);
 }
 const bottom=positions.length/3;positions.push(0,23,-15.4);
 for(const p of edge)positions.push(p.x,p.y,-15.4);
 for(let j=0;j<n;j++){
  const next=(j+1)%n,a=1+(rings-1)*n+j,b=bottom+1+j,c=bottom+1+next,d=1+(rings-1)*n+next;
  indices.push(a,b,c,a,c,d,bottom,c,b);
 }
 const g=new T.BufferGeometry();g.setAttribute('position',new T.Float32BufferAttribute(positions,3));
 g.setIndex(indices);g.computeVertexNormals();return g;
}
const pedestal=mesh('joystick_continuous_moulding',mouldingGeometry(),'shell');

// Rubberized cap and curved lower skirt, revolved as real geometry.
// The black joystick cap remains outside the grey housing. Do not infer
// a lower spherical extension or cut a matching internal pocket from photos.
const joystickSkirt=lathe('joystick_skirt',[[0,1.6],[11.6,1.6],[14.5,2.0],
 [14.2,3.0],[13.2,4.8],[11.4,6.8],[8.8,8.8],[5.1,10.4],[0,10.4]],'glass',28.55);
mesh('joystick_stem',cylinder(4.8,4.5),'black',[0,28.55,12.5]);
const cap=lathe('joystick_thumb_cap',[[0,14.4],[7.8,14.4],[8.8,14.8],[9.1,15.5],[9.1,16.5],[8.7,17.4],[7.7,18],[5.7,17.8],[3,17.4],[0,17.2]],'black',28.55);
// Four shallow grip dimples on the thumb surface.
for(let i=0;i<4;i++){
 const angle=Math.PI/4+i*Math.PI/2;
 const g=new T.SphereGeometry(.6,12,8);
 cut(cap,g,[Math.cos(angle)*5.7,28.55+Math.sin(angle)*5.7,17.75]);
}
// Eight HAT pins, 2.54 mm pitch, bent towards the receiving StickC.
mesh('HAT_pin_carrier',new T.BoxGeometry(21,2.0,2.5),'black',[0,8.3,-8.2]);
for(let i=0;i<8;i++){
 const x=(i-3.5)*2.54;
 mesh(`HAT_pin_${i+1}_leg`,new T.BoxGeometry(.64,4.5,.64),'metal',[x,6.4,-7.1]);
 mesh(`HAT_pin_${i+1}_elbow`,new T.BoxGeometry(.64,.64,3.3),'metal',[x,8.38,-8.4]);
}
// Exposed pouch cell and its yellow insulating tape inside the tray.
const battery=mesh('battery_pouch_200mAh',rounded(17.5,29,3.9,1.7,.3),'metal',[0,-13,-11.6]);
battery.material=new T.MeshStandardMaterial({name:'pouch_silver',color:0xc9cbcc,roughness:.55,metalness:.25});
const tape=new T.MeshStandardMaterial({name:'battery_tape',color:0xe7bd12,roughness:.45});
const tapeMesh=mesh('battery_tape_top',rounded(17.7,4.6,.25,.7),'gold',[0,-.6,-9.5]);tapeMesh.material=tape;
mesh('battery_tape_bottom',rounded(17.7,1.7,.2,.5),'gold',[0,-26.4,-9.55]);
for(const [name,color,x] of [['positive',0xc62b25,2.3],['negative',0x161616,-2.3]]){
 const curve=new T.CatmullRomCurve3([new T.Vector3(x,-.1,-10),new T.Vector3(x*1.6,2,-10),new T.Vector3(x*1.6,4.3,-12)]);
 const wire=mesh(`battery_wire_${name}`,new T.TubeGeometry(curve,12,.35,8,false),'black');wire.material=new T.MeshStandardMaterial({color,roughness:.65});
}
// Front mounting bosses: lower row 60 mm from joystick axis, spacing 16 mm.
for(const x of [-8,8]){
 const boss=mesh(`mount_boss_${x}`,cylinder(2.6,5),'shell',[x,-31.45,-12.2]);
 cut(boss,cylinder(1.05,9),[x,-31.45,-12]);
 for(const part of [rear,backRim]) cut(part,cylinder(1.9,8),[x,-31.45,-16]);
}
// Rear plate: a plain recessed label and upper 32 mm-spaced fastener row.
for(const part of [rear,backRim]) cut(part,rounded(18.7,40.7,1.3,.8),[0,-7,-18]);
mesh('rear_label',rounded(18.3,40.3,.12,.7),'white',[0,-7,-17.32]);
for(const x of [-8,8]){
 for(const part of [rear,backRim]){
  cut(part,cylinder(1.2,6),[x,20.55,-16]);
  cut(part,cylinder(2.2,1.4),[x,16,-18]);
 }
 const screw=mesh(`rear_screw_${x}`,cylinder(1.7,.65),'metal',[x,16,-17.4]);
 const hex=new T.CylinderGeometry(.72,.72,.8,6);hex.rotateX(Math.PI/2);cut(screw,hex,[x,16,-17.7]);
}
for(const part of [rear,backRim,pedestal])cut(part,rounded(1.3,5.1,5,.6),[0,20.55,-17]);
mesh('RGB_light_pipe',rounded(.9,4.7,.2,.4),'white',[0,20.55,-16.5]);
// Rear/top Grove connector, recessed into the crown rather than floating outside it.
for(const part of [rear,backRim,pedestal])cut(part,rounded(10.8,9.3,7,.4),[0,36.9,-15.5]);
const grove=frame('Grove_HY2_socket',10.1,6.2,.4,5.5,.8,'white',[0,35,-14.4],[Math.PI/2,0,0]);
cut(grove,rounded(6.5,4.5,2,.2),[0,36.5,-17.2]);
mesh('Grove_latch',rounded(6.3,2.3,1.2,.3),'shell',[0,37.2,-16.1]);
for(let i=0;i<4;i++)mesh(`Grove_contact_${i+1}`,new T.BoxGeometry(.5,3.5,.5),'metal',[-3+i*2,35,-14.5]);
// Engraved M5 mark near the foot; no printed sticker text is baked into geometry.
const M=polygon([[-3.7,-35.2],[-3.7,-32.7],[-3.2,-32.7],[-2.3,-33.8],[-1.4,-32.7],[-.9,-32.7],[-.9,-35.2],[-1.4,-35.2],[-1.4,-33.6],[-2.3,-34.6],[-3.2,-33.6],[-3.2,-35.2]]);
for(const part of [rear,backRim])cut(part,extrude(M,.5),[0,0,-17.9]);
const five=polygon([[.1,-32.7],[2.7,-32.7],[2.7,-33.15],[.6,-33.15],[.6,-33.7],[2,-33.7],[2.7,-34.15],[2.7,-34.8],[2.2,-35.2],[.1,-35.2],[.1,-34.75],[2,-34.75],[2.2,-34.6],[2.2,-34.35],[2,-34.15],[.1,-34.15]]);
for(const part of [rear,backRim])cut(part,extrude(five,.5),[0,0,-17.9]);
subtractFittedStick(pedestal);
battery.position.z-=.3;
tapeMesh.position.z-=.3;
root.getObjectByName('battery_tape_bottom').position.z-=.3;
// Verify mounting alignment and every HAT contact against the imported model.
root.updateMatrixWorld(true);
for(const x of [-8,8]) {
 const source=stick.getObjectByName(`thread_insert_${x}`).getWorldPosition(new T.Vector3());
 const target=root.getObjectByName(`mount_boss_${x}`).getWorldPosition(new T.Vector3());
 if(Math.hypot(source.x-target.x,source.y-target.y)>.001)throw new Error('Mount centres do not align');
}
for(let i=0;i<8;i++) {
 const socket=stick.getObjectByName(`HAT_socket_${i+1}`).getWorldPosition(new T.Vector3());
 const pin=root.getObjectByName(`HAT_pin_${i+1}_leg`).getWorldPosition(new T.Vector3());
 if(Math.hypot(socket.x-pin.x,socket.z-pin.z)>.001)throw new Error(`HAT contact ${i+1} does not align`);
}
// Verify the two reviewed features on the generated surfaces, in authoring mm.
root.updateMatrixWorld(true);
const fitRay=new T.Raycaster(new T.Vector3(8,-37,0),new T.Vector3(0,0,-1));
const fitHit=fitRay.intersectObject(trayFloor,false)[0];
if(!fitHit||Math.abs(fitHit.point.z-lowerSeatZ)>.01)throw new Error('Lower positive seating land missing');
const reliefRay=new T.Raycaster(new T.Vector3(11.5,20,10),new T.Vector3(0,0,-1));
const reliefHit=reliefRay.intersectObject(pedestal,false)[0];
if(!reliefHit||Math.abs(reliefHit.point.z-deckHeight(11.5,20))>.12)throw new Error('Joystick continuous saddle surface missing');
joystickSkirt.geometry.computeBoundingBox();
if(joystickSkirt.geometry.boundingBox.min.z<1.599)throw new Error('Black cap extends inside the housing');
root.scale.setScalar(.001);
root.updateMatrixWorld(true);
const bounds=new T.Box3().setFromObject(root),size=bounds.getSize(new T.Vector3());
// Drawing dimensions refer to the chassis; the joystick skirt overhangs its width.
if(Math.abs(size.x-.029)>1e-6 || Math.abs(size.y-.0836)>1e-6 || Math.abs(size.z-.036)>1e-6)
 throw new Error(`Unexpected external bounds ${size.toArray()}`);
let meshes=0,triangles=0;
root.traverse(o=>{if(o.isMesh){o.geometry.deleteAttribute('uv');meshes++;const p=o.geometry.getAttribute('position');
 for(let i=0;i<p.array.length;i++) if(!Number.isFinite(p.array[i])) throw new Error(`Invalid vertex ${o.name}`);
 triangles+=(o.geometry.index?.count??p.count)/3;}});
// Minimal Node FileReader adapter for the official exporter (no textures).
globalThis.FileReader=class {
 readAsArrayBuffer(blob){blob.arrayBuffer().then(result=>{this.result=result;this.onloadend?.();});}
};
const exporter=new GLTFExporter();
await writeFile(new URL('hat-mini-joyc.glb',import.meta.url),Buffer.from(await exporter.parseAsync(scene,{binary:true})));
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
const wireScene=new T.Scene();const wire=new T.Group();wire.name='Hat_Mini_JoyC_contours';wireScene.add(wire);
root.traverse(o=>{if(o.isMesh){const lines=new T.LineSegments(contours(o.geometry),new T.LineBasicMaterial({color:0xffffff}));
 lines.name=o.name;lines.applyMatrix4(o.matrixWorld);wire.add(lines);}});
await writeFile(new URL('hat-mini-joyc-contours.glb',import.meta.url),Buffer.from(await exporter.parseAsync(wireScene,{binary:true})));
console.log(JSON.stringify({meshes,triangles,dimensionsMm:size.multiplyScalar(1000).toArray()},null,2));

const assembly=new T.Scene();assembly.name='StickC_Plus2_with_Hat_Mini_JoyC';
assembly.add(root.clone(true));
stick.scale.setScalar(1);stick.position.copy(fitTranslationMm).multiplyScalar(.001);stick.updateMatrixWorld(true);
assembly.add(stick);
await writeFile(new URL('stickc-plus2-joyc-assembled.glb',import.meta.url),Buffer.from(await exporter.parseAsync(assembly,{binary:true})));
console.log('Mating surfaces from StickC GLB:',JSON.stringify(root.userData.fit));
