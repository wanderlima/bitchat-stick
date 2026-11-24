import * as THREE from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import gsap from 'gsap';
import { reducedMotion } from './preferences';
import type { PreviewState } from './controls';
import { initialPreviewScreens } from '../data/hardware';

export async function mountHardwareScene(root: HTMLElement) {
  const host = root.querySelector<HTMLElement>('[data-hardware-canvas]')!;
  const note = root.querySelector<HTMLElement>('[data-hardware-note]')!;
  const controller = new AbortController();
  const signal = controller.signal;
  root.dataset.modelState = 'loading';
  let renderer: THREE.WebGLRenderer;
  try {
    renderer = new THREE.WebGLRenderer({
      alpha: true,
      antialias: true,
      powerPreference: 'low-power',
    });
  } catch {
    root.dataset.modelState = 'fallback';
    note.textContent = '2D preview · Controls still work';
    return;
  }
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 1.75));
  renderer.setClearColor(0x000000, 0);
  const scene = new THREE.Scene();
  const camera = new THREE.OrthographicCamera(-0.05, 0.05, 0.052, -0.052, 0.001, 2);
  camera.position.set(0, 0, 0.3);
  const pose = new THREE.Group();
  scene.add(pose);
  pose.position.y = -0.0011;
  const textures = new Map<string, THREE.Texture>();
  const materials = new Set<THREE.Material>();
  let frame = 0,
    visible = false,
    disposed = false;
  let progress = Number(root.dataset.progress ?? 0);
  const pointer = { x: 0, y: 0 },
    current = { x: 0, y: 0 };
  let pulse: gsap.core.Timeline | undefined;
  let resetTimer: ReturnType<typeof setTimeout> | undefined;
  const media = window.matchMedia('(hover: hover) and (pointer: fine)');
  const syncMotion = () => {
    pointer.x = 0;
    pointer.y = 0;
    if (reducedMotion()) {
      floatTime = 0;
      root.dataset.floatY = '0.000000';
    }
    requestRender();
  };
  const motion = new MutationObserver(syncMotion);
  motion.observe(document.documentElement, {
    attributes: true,
    attributeFilter: ['data-reduce-motion'],
  });
  const systemMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
  systemMotion.addEventListener('change', syncMotion, { signal });
  let joystick: THREE.Object3D | undefined;
  let lastFrame = 0,
    floatTime = 0;
  const basePivotZ = 0.002;

  function requestRender() {
    if (disposed || !visible || document.hidden || frame) return;
    frame = requestAnimationFrame(render);
  }
  function render(time: number) {
    frame = 0;
    const delta = Math.min((time - lastFrame) / 1000 || 0.016, 0.05);
    lastFrame = time;
    const reduced = reducedMotion();
    const tx = reduced ? 0 : pointer.x,
      ty = reduced ? 0 : pointer.y;
    current.x = THREE.MathUtils.damp(current.x, tx, 9, delta);
    current.y = THREE.MathUtils.damp(current.y, ty, 9, delta);
    if (!reduced) floatTime += delta;
    const floatY = reduced ? 0 : Math.sin((floatTime * Math.PI * 2) / 4.8) * 0.0016;
    pose.position.y = -0.0011 + floatY;
    pose.position.x = reduced ? 0 : current.x * 0.0012;
    pose.rotation.set(
      reduced ? -0.09 : -0.09 + current.y * 0.085,
      reduced ? -0.18 : -0.42 + progress * 0.62 + current.x * 0.12,
      reduced ? -0.08 : -0.15 + progress * 0.18 + current.x * 0.015,
    );
    root.dataset.floatY = floatY.toFixed(6);
    renderer.render(scene, camera);
    root.dataset.renderFrame = String(renderer.info.render.frame);
    root.dataset.viewYaw = pose.rotation.y.toFixed(4);
    if (joystick)
      root.dataset.joystickTilt = Math.hypot(joystick.rotation.x, joystick.rotation.y).toFixed(3);
    if (!reduced || Math.abs(current.x - tx) + Math.abs(current.y - ty) > 0.001) requestRender();
  }
  function resize() {
    const { width, height } = host.getBoundingClientRect();
    if (!width || !height) return;
    renderer.setSize(width, height, false);
    const margin = 1.22;
    const halfHeight =
      (Math.max(deviceHeight, (deviceWidth * height) / Math.max(width, 1)) * margin) / 2;
    camera.top = halfHeight;
    camera.bottom = -halfHeight;
    camera.left = (-halfHeight * width) / height;
    camera.right = -camera.left;
    camera.updateProjectionMatrix();
    requestRender();
  }
  const textureLoader = new THREE.TextureLoader();
  const pendingTextures = new Map<string, Promise<THREE.Texture>>();
  let desiredScreen = root.dataset.screen ?? 'home-nearby';
  function loadTexture(name: string): Promise<THREE.Texture> {
    const cached = textures.get(name);
    if (cached) {
      textures.delete(name);
      textures.set(name, cached);
      return Promise.resolve(cached);
    }
    const pending = pendingTextures.get(name);
    if (pending) return pending;
    const loading = textureLoader
      .loadAsync(`${host.dataset.screenBase}${name}.png`)
      .then((texture) => {
        texture.colorSpace = THREE.SRGBColorSpace;
        texture.magFilter = THREE.NearestFilter;
        texture.minFilter = THREE.NearestFilter;
        pendingTextures.delete(name);
        if (disposed) {
          texture.dispose();
          return texture;
        }
        textures.set(name, texture);
        return texture;
      })
      .catch((error) => {
        pendingTextures.delete(name);
        throw error;
      });
    pendingTextures.set(name, loading);
    return loading;
  }
  function setScreen(name: string) {
    desiredScreen = name;
    loadTexture(name)
      .then((texture) => {
        if (disposed || desiredScreen !== name) return;
        screenMaterial.map = texture;
        screenMaterial.needsUpdate = true;
        root.dataset.renderedScreen = name;
        // Cache a small working set; the 32-preset browser should not accumulate GPU textures.
        for (const [key, value] of textures) {
          if (textures.size <= 18) break;
          if (value !== screenMaterial.map) {
            textures.delete(key);
            value.dispose();
          }
        }
        requestRender();
      })
      .catch(() => {
        if (!disposed && desiredScreen === name)
          note.textContent = 'Screen unavailable · Controls still work';
      });
  }
  function feedback(state: PreviewState) {
    setScreen(state.screen);
    if (!joystick || state.direction === 'neutral') return;
    root.dataset.joystickAction = state.direction;
    pulse?.kill();
    clearTimeout(resetTimer);
    const direction = state.direction;
    // The body is turned 90° so the 240×135 UI sits horizontal. Hat local
    // axes still match the original vertical model, so tilt is remapped
    // into screen space: up is +Y and right is +X.
    const x = direction === 'left' ? -0.32 : direction === 'right' ? 0.32 : 0;
    const y = direction === 'up' ? 0.32 : direction === 'down' ? -0.32 : 0;
    const pressed = direction === 'confirm';
    // Publish the commanded deflection immediately. A slow frame can finish the
    // whole pulse before the canvas samples it, which would leave the peak at 0.
    root.dataset.joystickTilt = Math.hypot(x, y).toFixed(3);
    root
      .querySelectorAll<HTMLElement>('[data-joy-action]')
      .forEach((button) =>
        button.toggleAttribute('data-active', button.dataset.joyAction === direction),
      );
    const clear = () => {
      root
        .querySelectorAll('[data-active]')
        .forEach((button) => button.removeAttribute('data-active'));
      root.dataset.joystickAction = 'neutral';
    };
    if (reducedMotion() || state.source === 'keyboard') {
      joystick.rotation.set(x, y, 0);
      joystick.position.z = basePivotZ - (pressed ? 0.0006 : 0);
      requestRender();
      resetTimer = setTimeout(() => {
        joystick!.rotation.set(0, 0, 0);
        joystick!.position.z = basePivotZ;
        clear();
        requestRender();
      }, 160);
      return;
    }
    pulse = gsap.timeline({ onUpdate: requestRender, onComplete: clear });
    pulse
      .to(joystick.rotation, { x, y, duration: 0.1, ease: 'power3.out' }, 0)
      .to(
        joystick.position,
        { z: basePivotZ - (pressed ? 0.0006 : 0), duration: 0.1, ease: 'power3.out' },
        0,
      )
      .to(joystick.rotation, { x: 0, y: 0, duration: 0.18, ease: 'power2.inOut' }, 0.1)
      .to(joystick.position, { z: basePivotZ, duration: 0.18, ease: 'power2.inOut' }, 0.1);
  }
  const screenMaterial = new THREE.MeshBasicMaterial({ toneMapped: false });
  materials.add(screenMaterial);
  const screen = new THREE.Mesh(new THREE.PlaneGeometry(1, 1), screenMaterial);
  screen.name = 'live-firmware-screen';
  screen.renderOrder = 2;
  pose.add(screen);
  let deviceWidth = 0.04;
  let deviceHeight = 0.022;
  const observer = new IntersectionObserver((entries) => {
    const rect = host.getBoundingClientRect();
    visible =
      entries[0].isIntersecting || (rect.bottom > 0 && rect.top < innerHeight && rect.width > 0);
    if (visible) {
      lastFrame = 0;
      requestRender();
    } else {
      cancelAnimationFrame(frame);
      frame = 0;
    }
  });
  const sizeObserver = new ResizeObserver(resize);
  function dispose() {
    if (disposed) return;
    disposed = true;
    controller.abort();
    observer.disconnect();
    sizeObserver.disconnect();
    motion.disconnect();
    cancelAnimationFrame(frame);
    pulse?.kill();
    clearTimeout(resetTimer);
    scene.traverse((object) => {
      if (object instanceof THREE.Mesh || object instanceof THREE.LineSegments)
        object.geometry.dispose();
    });
    textures.forEach((texture) => texture.dispose());
    materials.forEach((material) => material.dispose());
    renderer.dispose();
    renderer.domElement.remove();
  }
  try {
    const loader = new GLTFLoader();
    const [asset] = await Promise.all([
      loader.loadAsync(host.dataset.modelUrl!),
      Promise.all(initialPreviewScreens.map(loadTexture)),
    ]);
    if (disposed) return;
    asset.scene.traverse((object) => {
      if (!(object instanceof THREE.Mesh || object instanceof THREE.LineSegments)) return;
      const previous = object.material;
      if (Array.isArray(previous)) previous.forEach((material) => material.dispose());
      else previous.dispose();
      if (object instanceof THREE.Mesh) {
        const material = new THREE.MeshBasicMaterial({
          colorWrite: false,
          depthWrite: true,
          polygonOffset: true,
          polygonOffsetFactor: 1,
          polygonOffsetUnits: 1,
        });
        object.material = material;
        object.renderOrder = 0;
        materials.add(material);
      } else {
        const moving = /joystick_(skirt|stem|thumb_cap)/.test(object.userData.part ?? '');
        const material = new THREE.LineBasicMaterial({
          color: moving ? 0x84ff84 : 0xebdcba,
          transparent: true,
          opacity: moving ? 0.95 : 0.72,
          toneMapped: false,
        });
        object.material = material;
        object.renderOrder = 1;
        materials.add(material);
      }
    });
    const turned = new THREE.Group();
    turned.rotation.z = Math.PI / 2;
    turned.add(asset.scene);
    pose.add(turned);
    turned.updateWorldMatrix(true, true);
    const bounds = new THREE.Box3().setFromObject(turned);
    turned.position.sub(bounds.getCenter(new THREE.Vector3()));
    turned.updateWorldMatrix(true, true);
    const fitted = new THREE.Box3().setFromObject(turned).getSize(new THREE.Vector3());
    turned.scale.setScalar(0.046 / Math.max(fitted.x, fitted.y));
    turned.updateWorldMatrix(true, true);
    const placed = new THREE.Box3().setFromObject(turned).getSize(new THREE.Vector3());
    deviceWidth = placed.x;
    deviceHeight = placed.y;
    const glass = asset.scene.getObjectByName('stickc-plus2_display_glass_depth');
    if (glass) {
      const glassBox = new THREE.Box3().setFromObject(glass);
      const size = glassBox.getSize(new THREE.Vector3());
      const glassCenter = glassBox.getCenter(new THREE.Vector3());
      // Frames are 240×135. The glass is a little taller, so fit the picture
      // inside the opening instead of stretching it to the glass.
      const frameAspect = 240 / 135;
      const openingWidth = size.x * 0.98;
      const openingHeight = size.y * 0.98;
      const frameHeight = Math.min(openingHeight, openingWidth / frameAspect);
      const frameWidth = frameHeight * frameAspect;
      screen.scale.set(frameWidth, frameHeight, 1);
      screen.position.set(glassCenter.x, glassCenter.y, glassBox.max.z + 0.00012);
    }
    joystick = asset.scene.getObjectByName('joystick-pivot');
    if (!joystick) throw new Error('Missing joystick articulation');
    host.append(renderer.domElement);
    setScreen(root.dataset.screen ?? 'home-nearby');
    sizeObserver.observe(host);
    observer.observe(root);
    resize();
    root.dataset.modelState = 'ready';
    root.addEventListener(
      'preview-change',
      (event) => feedback((event as CustomEvent<PreviewState>).detail),
      { signal },
    );
    root.addEventListener(
      'hardware-view',
      (event) => {
        progress = (event as CustomEvent<{ progress: number }>).detail.progress;
        requestRender();
      },
      { signal },
    );
    window.addEventListener(
      'pointermove',
      (event) => {
        if (!media.matches || !visible || reducedMotion() || event.pointerType !== 'mouse') return;
        pointer.x = THREE.MathUtils.clamp((event.clientX / innerWidth) * 2 - 1, -1, 1);
        pointer.y = THREE.MathUtils.clamp((event.clientY / innerHeight) * 2 - 1, -1, 1);
        requestRender();
      },
      { signal, passive: true },
    );
    document.documentElement.addEventListener(
      'pointerleave',
      () => {
        pointer.x = 0;
        pointer.y = 0;
        requestRender();
      },
      { signal },
    );
    document.addEventListener(
      'visibilitychange',
      () => {
        if (document.hidden) {
          cancelAnimationFrame(frame);
          frame = 0;
        } else requestRender();
      },
      { signal },
    );
    renderer.domElement.addEventListener(
      'webglcontextlost',
      (event) => {
        event.preventDefault();
        dispose();
        root.dataset.modelState = 'fallback';
        note.textContent = '2D preview · Controls still work';
      },
      { signal },
    );
    window.addEventListener('pagehide', dispose, { once: true, signal });
  } catch {
    dispose();
    root.dataset.modelState = 'fallback';
    note.textContent = '2D preview · Controls still work';
  }
}
