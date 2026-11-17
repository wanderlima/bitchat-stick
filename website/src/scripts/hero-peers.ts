import * as THREE from 'three';
import { reducedMotion } from './preferences';

const PAD = 0.22;
const REST_X = -0.08;
const REST_Y = -0.28;

/** Outline phones over the hero peers. Each one turns toward the pointer from its own place on the screen. */
export function mountHeroPeers() {
  const hero = document.querySelector<HTMLElement>('.hero');
  const host = hero?.querySelector<HTMLElement>('[data-hero-peers]');
  if (!hero || !host) return;
  let renderer: THREE.WebGLRenderer;
  try {
    renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true, powerPreference: 'low-power' });
  } catch {
    return;
  }
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 1.75));
  renderer.setClearColor(0x000000, 0);
  renderer.autoClear = false;
  const phones = Array.from({ length: 4 }, () => createPhone());
  const pointer = { x: 0, y: 0, active: false };
  const media = window.matchMedia('(hover: hover) and (pointer: fine)');
  const controller = new AbortController();
  const signal = controller.signal;
  let frame = 0;
  let visible = false;
  let disposed = false;
  let lastFrame = 0;
  const viewports: { x: number; y: number; w: number; h: number }[] = [];

  function requestRender() {
    if (disposed || !visible || document.hidden || frame) return;
    frame = requestAnimationFrame(render);
  }
  function activeMesh() {
    return [...hero!.querySelectorAll<SVGSVGElement>('[data-mesh]')].find(mesh => getComputedStyle(mesh).display !== 'none');
  }
  function render(time: number) {
    frame = 0;
    const delta = Math.min((time - lastFrame) / 1000 || 0.016, 0.05);
    lastFrame = time;
    const reduced = reducedMotion();
    let settling = false;
    for (const phone of phones) {
      const target = deviceOffset(pointer, phone.slot, reduced);
      phone.x = THREE.MathUtils.damp(phone.x, target.x, 7, delta);
      phone.y = THREE.MathUtils.damp(phone.y, target.y, 7, delta);
      phone.pose.rotation.set(REST_X + phone.y * 0.1, REST_Y + phone.x * 0.22, phone.x * 0.03);
      if (Math.abs(phone.x - target.x) + Math.abs(phone.y - target.y) > 0.001) settling = true;
    }
    renderer.setScissorTest(false);
    renderer.clear();
    renderer.setScissorTest(true);
    viewports.forEach((viewport, index) => {
      if (viewport.w < 2 || viewport.h < 2) return;
      renderer.setViewport(viewport.x, viewport.y, viewport.w, viewport.h);
      renderer.setScissor(viewport.x, viewport.y, viewport.w, viewport.h);
      renderer.render(phones[index].scene, phones[index].camera);
    });
    if (settling) requestRender();
  }
  function place() {
    const mesh = activeMesh();
    if (!mesh) return;
    const heroBox = hero!.getBoundingClientRect();
    const meshBox = mesh.getBoundingClientRect();
    host!.style.left = `${meshBox.left - heroBox.left}px`;
    host!.style.top = `${meshBox.top - heroBox.top}px`;
    host!.style.width = `${meshBox.width}px`;
    host!.style.height = `${meshBox.height}px`;
    host!.style.opacity = getComputedStyle(mesh).opacity;
    const width = host!.clientWidth;
    const height = host!.clientHeight;
    if (!width || !height) return;
    renderer.setSize(width, height, false);
    const ratio = renderer.getPixelRatio();
    const slots = [...mesh.querySelectorAll<SVGRectElement>('[data-peer-slot]')].filter(slot => slot.getClientRects().length);
    viewports.length = 0;
    slots.slice(0, 4).forEach((slot, index) => {
      const box = slot.getBoundingClientRect();
      const padX = box.width * PAD;
      const padY = box.height * PAD;
      const pixel = {
        x: (box.left - meshBox.left - padX) * ratio,
        y: (meshBox.bottom - box.bottom - padY) * ratio,
        w: (box.width + padX * 2) * ratio,
        h: (box.height + padY * 2) * ratio,
      };
      viewports.push(pixel);
      phones[index].slot = slot;
      const camera = phones[index].camera;
      camera.aspect = pixel.w / pixel.h;
      const visibleHeight = phones[index].height * (1 + PAD * 2);
      camera.position.z = (visibleHeight / 2) / Math.tan(THREE.MathUtils.degToRad(camera.fov / 2));
      camera.updateProjectionMatrix();
    });
    const peers = [...mesh.querySelectorAll<SVGGElement>('[data-peer]')].filter(peer => peer.querySelector('[data-peer-slot]'));
    peers.slice(0, 4).forEach((peer, index) => {
      const lit = Number(getComputedStyle(peer.querySelector('.device-accent')!).opacity) > 0.5;
      const material = phones[index].material;
      material.color.setHex(lit ? 0x84ff84 : 0xebdcba);
      material.opacity = lit ? 0.95 : 0.78;
    });
    requestRender();
  }
  function dispose() {
    if (disposed) return;
    disposed = true;
    controller.abort();
    cancelAnimationFrame(frame);
    phones.forEach(phone => phone.dispose());
    renderer.dispose();
    renderer.domElement.remove();
    if (hero!.dataset.peers === 'ready') delete hero!.dataset.peers;
  }

  host.append(renderer.domElement);
  place();
  hero.dataset.peers = 'ready';
  const observer = new ResizeObserver(place);
  observer.observe(hero);
  const presentation = new MutationObserver(place);
  for (const mesh of hero.querySelectorAll<SVGSVGElement>('[data-mesh]')) {
    presentation.observe(mesh, { attributes: true, attributeFilter: ['style'] });
    for (const accent of mesh.querySelectorAll('.device-accent')) presentation.observe(accent, { attributes: true, attributeFilter: ['style', 'opacity'] });
  }
  const reset = () => { pointer.active = false; requestRender(); };
  window.addEventListener('pointermove', event => {
    if (!media.matches || !visible || reducedMotion() || event.pointerType !== 'mouse') return;
    pointer.active = true;
    pointer.x = event.clientX;
    pointer.y = event.clientY;
    requestRender();
  }, { signal, passive: true });
  document.documentElement.addEventListener('pointerleave', reset, { signal });
  window.matchMedia('(prefers-reduced-motion: reduce)').addEventListener('change', reset, { signal });
  window.addEventListener('resize', place, { signal, passive: true });
  document.addEventListener('visibilitychange', () => { if (!document.hidden) requestRender(); }, { signal });
  renderer.domElement.addEventListener('webglcontextlost', event => { event.preventDefault(); dispose(); }, { signal });
  window.addEventListener('pagehide', dispose, { once: true, signal });
  const intersection = new IntersectionObserver(([entry]) => { visible = entry.isIntersecting; if (visible) requestRender(); });
  intersection.observe(hero);
  signal.addEventListener('abort', () => { observer.disconnect(); presentation.disconnect(); intersection.disconnect(); });
  requestRender();
}

function deviceOffset(pointer: { x: number; y: number; active: boolean }, slot: Element | null, reduced: boolean) {
  if (!pointer.active || reduced || !slot) return { x: 0, y: 0 };
  const box = slot.getBoundingClientRect();
  if (!box.width) return { x: 0, y: 0 };
  return {
    x: THREE.MathUtils.clamp((pointer.x - (box.left + box.width / 2)) / (innerWidth * 0.45), -1, 1),
    y: THREE.MathUtils.clamp((pointer.y - (box.top + box.height / 2)) / (innerHeight * 0.45), -1, 1),
  };
}

function createPhone() {
  const width = 0.92;
  const height = 1.32;
  const radius = 0.12;
  const depth = 0.18;
  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera(26, width / height, 0.01, 10);
  const pose = new THREE.Group();
  scene.add(pose);
  const material = new THREE.LineBasicMaterial({ color: 0xebdcba, transparent: true, opacity: 0.78, toneMapped: false, depthTest: false });
  const shape = roundedShape(width, height, radius);
  const body = new THREE.Mesh(
    new THREE.ExtrudeGeometry(shape, { depth, bevelEnabled: false, curveSegments: 10 }).translate(0, 0, -depth / 2),
    new THREE.MeshBasicMaterial({ colorWrite: false, depthWrite: true }),
  );
  pose.add(body);
  const shell = new THREE.LineSegments(new THREE.EdgesGeometry(body.geometry, 1), material);
  shell.renderOrder = 1;
  pose.add(shell);
  const face = depth / 2 + 0.02;
  pose.add(loop(roundedPoints(width * 0.78, height * 0.68, 0.05, face), material));
  pose.add(loop([new THREE.Vector3(-0.1, height * 0.4, face), new THREE.Vector3(0.1, height * 0.4, face)], material));
  pose.add(loop([new THREE.Vector3(-0.16, 0.02, face), new THREE.Vector3(0.16, 0.02, face)], material));
  pose.add(loop([new THREE.Vector3(-0.16, -0.12, face), new THREE.Vector3(0.08, -0.12, face)], material));
  pose.add(loop([new THREE.Vector3(-0.08, -height * 0.4, face), new THREE.Vector3(0.08, -height * 0.4, face)], material));
  return {
    scene, camera, pose, material, height, x: 0, y: 0, slot: null as SVGRectElement | null,
    dispose() {
      body.geometry.dispose();
      (body.material as THREE.Material).dispose();
      material.dispose();
      pose.traverse(object => {
        if (object instanceof THREE.Line || object instanceof THREE.LineSegments) object.geometry.dispose();
      });
    },
  };
}

function loop(points: THREE.Vector3[], material: THREE.LineBasicMaterial) {
  const line = new THREE.Line(new THREE.BufferGeometry().setFromPoints(points), material);
  line.renderOrder = 1;
  return line;
}

function roundedShape(width: number, height: number, radius: number) {
  const shape = new THREE.Shape();
  const x = -width / 2;
  const y = -height / 2;
  shape.moveTo(x + radius, y);
  shape.lineTo(x + width - radius, y);
  shape.absarc(x + width - radius, y + radius, radius, -Math.PI / 2, 0, false);
  shape.lineTo(x + width, y + height - radius);
  shape.absarc(x + width - radius, y + height - radius, radius, 0, Math.PI / 2, false);
  shape.lineTo(x + radius, y + height);
  shape.absarc(x + radius, y + height - radius, radius, Math.PI / 2, Math.PI, false);
  shape.lineTo(x, y + radius);
  shape.absarc(x + radius, y + radius, radius, Math.PI, Math.PI * 1.5, false);
  return shape;
}

function roundedPoints(width: number, height: number, radius: number, z: number) {
  const points: THREE.Vector3[] = [];
  const corners: [number, number, number, number][] = [
    [width / 2 - radius, -height / 2 + radius, -Math.PI / 2, 0],
    [width / 2 - radius, height / 2 - radius, 0, Math.PI / 2],
    [-width / 2 + radius, height / 2 - radius, Math.PI / 2, Math.PI],
    [-width / 2 + radius, -height / 2 + radius, Math.PI, Math.PI * 1.5],
  ];
  for (const [cx, cy, start, end] of corners) {
    for (let step = 0; step <= 8; step++) {
      const angle = start + (end - start) * (step / 8);
      points.push(new THREE.Vector3(cx + Math.cos(angle) * radius, cy + Math.sin(angle) * radius, z));
    }
  }
  points.push(points[0].clone());
  return points;
}
