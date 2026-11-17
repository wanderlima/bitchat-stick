import * as THREE from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { reducedMotion } from './preferences';

const PAD = 0.42;
const REST_X = -0.05;
const REST_Y = -0.22;

/** Outline StickC Plus2 in the hero slot. It turns toward the pointer from its own place on the screen. */
export async function mountHeroDevice() {
  const hero = document.querySelector<HTMLElement>('.hero');
  const host = hero?.querySelector<HTMLElement>('[data-hero-device]');
  if (!hero || !host) return;
  const controller = new AbortController();
  const signal = controller.signal;
  let renderer: THREE.WebGLRenderer;
  try {
    renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true, powerPreference: 'low-power' });
  } catch {
    hero.dataset.device = 'fallback';
    return;
  }
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 1.75));
  renderer.setClearColor(0x000000, 0);
  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera(26, 1, 0.01, 2);
  const pose = new THREE.Group();
  scene.add(pose);
  const textures = new Map<string, THREE.Texture>();
  const materials = new Set<THREE.Material>();
  const outlines: THREE.LineBasicMaterial[] = [];
  const pointer = { x: 0, y: 0, active: false };
  const current = { x: 0, y: 0 };
  let slotEl: SVGRectElement | null = null;
  const media = window.matchMedia('(hover: hover) and (pointer: fine)');
  let frame = 0;
  let visible = false;
  let disposed = false;
  let lastFrame = 0;
  let modelHeight = 0.048;
  let shown = '';
  let litState = false;
  const screenMaterial = new THREE.MeshBasicMaterial({ toneMapped: false });
  materials.add(screenMaterial);
  const screen = new THREE.Mesh(new THREE.PlaneGeometry(1, 1), screenMaterial);
  screen.name = 'hero-firmware-screen';
  screen.renderOrder = 2;
  pose.add(screen);

  function requestRender() {
    if (disposed || !visible || document.hidden || frame) return;
    frame = requestAnimationFrame(render);
  }
  function activeMesh() {
    return [...hero!.querySelectorAll<SVGSVGElement>('[data-mesh]')].find(mesh => getComputedStyle(mesh).display !== 'none');
  }
  function syncFromMesh() {
    const mesh = activeMesh();
    if (!mesh) return;
    host!.style.opacity = getComputedStyle(mesh).opacity;
    const lit = Number(getComputedStyle(mesh.querySelector('[data-peer="stick"] .device-accent')!).opacity) > 0.5;
    if (lit !== litState) {
      litState = lit;
      hero!.dataset.stickLit = lit ? 'true' : 'false';
      for (const material of outlines) {
        material.color.setHex(lit ? 0x84ff84 : 0xebdcba);
        material.opacity = lit ? 0.95 : 0.72;
      }
      requestRender();
    }
    const active = [...mesh.querySelectorAll<SVGImageElement>('[data-screen]')].find(image => Number(getComputedStyle(image).opacity) > 0.5);
    const name = active?.dataset.screen ?? 'home';
    if (name !== shown) setScreen(name);
  }
  function render(time: number) {
    frame = 0;
    const delta = Math.min((time - lastFrame) / 1000 || 0.016, 0.05);
    lastFrame = time;
    const reduced = reducedMotion();
    const target = deviceOffset(pointer, slotEl, reduced);
    current.x = THREE.MathUtils.damp(current.x, target.x, 7, delta);
    current.y = THREE.MathUtils.damp(current.y, target.y, 7, delta);
    pose.rotation.set(REST_X + current.y * 0.16, REST_Y + current.x * 0.34, current.x * 0.04);
    host!.dataset.viewYaw = pose.rotation.y.toFixed(4);
    renderer.render(scene, camera);
    if (Math.abs(current.x - target.x) + Math.abs(current.y - target.y) > 0.001) requestRender();
  }
  function place() {
    const slot = [...hero!.querySelectorAll<SVGRectElement>('[data-device-slot]')].find(element => element.getClientRects().length);
    slotEl = slot ?? null;
    if (!slot) return;
    const heroBox = hero!.getBoundingClientRect();
    const box = slot.getBoundingClientRect();
    const padX = box.width * PAD;
    const padY = box.height * PAD;
    host!.style.left = `${box.left - heroBox.left - padX}px`;
    host!.style.top = `${box.top - heroBox.top - padY}px`;
    host!.style.width = `${box.width + padX * 2}px`;
    host!.style.height = `${box.height + padY * 2}px`;
    const width = host!.clientWidth;
    const height = host!.clientHeight;
    if (!width || !height) return;
    renderer.setSize(width, height, false);
    camera.aspect = width / height;
    const visibleHeight = modelHeight * (1 + PAD * 2);
    camera.position.z = (visibleHeight / 2) / Math.tan(THREE.MathUtils.degToRad(camera.fov / 2));
    camera.updateProjectionMatrix();
    requestRender();
  }
  const textureLoader = new THREE.TextureLoader();
  function loadTexture(name: string) {
    const cached = textures.get(name);
    if (cached) return Promise.resolve(cached);
    return textureLoader.loadAsync(`${host!.dataset.screenBase}${name}.png`).then(texture => {
      texture.colorSpace = THREE.SRGBColorSpace;
      texture.magFilter = THREE.NearestFilter;
      texture.minFilter = THREE.NearestFilter;
      texture.generateMipmaps = false;
      if (disposed) texture.dispose();
      else textures.set(name, texture);
      return texture;
    });
  }
  function setScreen(name: string) {
    shown = name;
    loadTexture(name).then(texture => {
      if (disposed || shown !== name) return;
      screenMaterial.map = texture;
      screenMaterial.needsUpdate = true;
      host!.dataset.renderedScreen = name;
      requestRender();
    }).catch(() => {});
  }
  function dispose() {
    if (disposed) return;
    disposed = true;
    controller.abort();
    observer.disconnect();
    presentation.disconnect();
    sizeObserver.disconnect();
    cancelAnimationFrame(frame);
    scene.traverse(object => {
      if (object instanceof THREE.Mesh) object.geometry.dispose();
    });
    textures.forEach(texture => texture.dispose());
    materials.forEach(material => material.dispose());
    renderer.dispose();
    renderer.domElement.remove();
    if (hero!.dataset.device === 'ready') hero!.dataset.device = 'fallback';
  }
  const presentation = new MutationObserver(() => syncFromMesh());
  const observer = new IntersectionObserver(entries => {
    visible = entries[0].isIntersecting;
    if (visible) {
      lastFrame = 0;
      requestRender();
    } else {
      cancelAnimationFrame(frame);
      frame = 0;
    }
  });
  const sizeObserver = new ResizeObserver(place);
  try {
    const loader = new GLTFLoader();
    const screens = ['home', 'writing', 'confirm', 'queued', 'received'];
    const [solid, contours] = await Promise.all([
      loader.loadAsync(host.dataset.modelUrl!),
      loader.loadAsync(host.dataset.contourUrl!),
      Promise.all(screens.map(loadTexture)),
    ]);
    if (disposed) return;
    const depth = new THREE.MeshBasicMaterial({ colorWrite: false, depthWrite: true, polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1 });
    materials.add(depth);
    for (const source of [solid.scene, contours.scene]) {
      source.traverse(object => {
        if (!(object instanceof THREE.Mesh || object instanceof THREE.LineSegments)) return;
        const previous = object.material;
        if (Array.isArray(previous)) previous.forEach(material => material.dispose());
        else previous.dispose();
        if (object instanceof THREE.Mesh) {
          object.material = depth;
          object.renderOrder = 0;
        } else {
          const material = new THREE.LineBasicMaterial({ color: 0xebdcba, transparent: true, opacity: 0.72, toneMapped: false });
          object.material = material;
          object.renderOrder = 1;
          materials.add(material);
          outlines.push(material);
        }
      });
      pose.add(source);
    }
    const bounds = new THREE.Box3().setFromObject(solid.scene);
    const center = bounds.getCenter(new THREE.Vector3());
    solid.scene.position.sub(center);
    contours.scene.position.sub(center);
    solid.scene.updateWorldMatrix(true, true);
    modelHeight = new THREE.Box3().setFromObject(solid.scene).getSize(new THREE.Vector3()).y;
    const glass = solid.scene.getObjectByName('display_glass');
    if (glass) {
      const glassBox = new THREE.Box3().setFromObject(glass);
      const size = glassBox.getSize(new THREE.Vector3());
      const glassCenter = glassBox.getCenter(new THREE.Vector3());
      screen.scale.set(size.x * 0.98, size.y * 0.98, 1);
      screen.position.set(glassCenter.x, glassCenter.y, glassBox.max.z + 0.00012);
    }
    host.append(renderer.domElement);
    place();
    sizeObserver.observe(hero);
    observer.observe(hero);
    hero.dataset.device = 'ready';
    host.dataset.modelState = 'ready';
    for (const mesh of hero.querySelectorAll<SVGSVGElement>('[data-mesh]')) {
      presentation.observe(mesh, { attributes: true, attributeFilter: ['style'] });
      const stick = mesh.querySelector('[data-peer="stick"]');
      if (stick) presentation.observe(stick, { attributes: true, attributeFilter: ['style', 'opacity'], subtree: true });
    }
    syncFromMesh();
    requestRender();
    const reset = () => { pointer.active = false; requestRender(); };
    window.addEventListener('pointermove', event => {
      if (!media.matches || !visible || reducedMotion() || event.pointerType !== 'mouse') return;
      pointer.active = true;
      pointer.x = event.clientX;
      pointer.y = event.clientY;
      requestRender();
    }, { signal, passive: true });
    document.documentElement.addEventListener('pointerleave', reset, { signal });
    const motion = new MutationObserver(reset);
    motion.observe(document.documentElement, { attributes: true, attributeFilter: ['data-reduce-motion'] });
    signal.addEventListener('abort', () => motion.disconnect());
    window.matchMedia('(prefers-reduced-motion: reduce)').addEventListener('change', reset, { signal });
    window.addEventListener('resize', place, { signal, passive: true });
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) {
        cancelAnimationFrame(frame);
        frame = 0;
      } else requestRender();
    }, { signal });
    renderer.domElement.addEventListener('webglcontextlost', event => {
      event.preventDefault();
      dispose();
    }, { signal });
    window.addEventListener('pagehide', dispose, { once: true, signal });
  } catch {
    dispose();
    hero.dataset.device = 'fallback';
  }
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
