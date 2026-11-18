import gsap from 'gsap';
import { desktopRoutes, mobileRoutes, length, type Route } from '../data/mesh';
import { reducedMotion } from './preferences';

const PORTRAIT_RATIO = 327 / 420;

/** Sizes the portrait diagram to the column, without growing taller than the screen. */
function fitPortraitGraph(hero: HTMLElement) {
  const graph = hero.querySelector<HTMLElement>('.mobile-graph');
  if (!graph) {
    return;
  }
  if (getComputedStyle(graph).display === 'none') {
    graph.style.removeProperty('width');
    return;
  }
  const header = document.querySelector('.site-header');
  const lead = hero.querySelector('.hero-lead');
  if (!header || !lead) {
    return;
  }
  const top =
    header.getBoundingClientRect().height + lead.getBoundingClientRect().height + 24;
  const room = window.innerHeight - top;
  const column = graph.parentElement?.clientWidth ?? window.innerWidth;
  const fromHeight = Math.max(0, room) * PORTRAIT_RATIO;
  const width = Math.round(Math.min(column, Math.max(fromHeight, Math.min(column, 220))));
  graph.style.width = `${width}px`;
}

/** Two stories: the Stick sends one message, then receives one. The envelope glides, and peers stay lit until that message arrives. */
export async function initHero() {
  const hero = document.querySelector<HTMLElement>('.hero');
  if (!hero) {
    return;
  }
  fitPortraitGraph(hero);
  window.addEventListener('resize', () => fitPortraitGraph(hero), { passive: true });
  document.fonts.ready.then(() => fitPortraitGraph(hero));
  const sources = [
    ...new Set(
      Array.from(hero.querySelectorAll<SVGImageElement>('[data-screen]')).map((image) =>
        image.getAttribute('href')!,
      ),
    ),
  ];
  const ready = await Promise.all(
    sources.map((source) => {
      const image = new Image();
      image.src = source;
      return image.decode();
    }),
  ).then(
    () => true,
    () => false,
  );
  if (!ready) {
    hero.dataset.motion = 'static';
    return;
  }
  const media = gsap.matchMedia();
  let entrancePlayed = false;
  media.add(
    {
      desktop: '(min-width: 640px)',
      mobile: '(max-width: 639px)',
      reduce: '(prefers-reduced-motion: reduce)',
    },
    (context) => {
      if (reducedMotion()) {
        entrancePlayed = true;
        hero.dataset.entrance = 'static';
        hero.dataset.motion = 'reduced';
        return;
      }
      const desktop = context.conditions?.desktop;
      const scene = hero.querySelector<SVGSVGElement>(
        `[data-mesh="${desktop ? 'desktop' : 'mobile'}"]`,
      )!;
      const routes = desktop ? desktopRoutes : mobileRoutes;
      const packet = scene.querySelector<SVGGElement>('.packet')!;
      const rings = [...scene.querySelectorAll<SVGCircleElement>('.ripple')];
      let rippleCursor = 0;
      const ripple = (x: number, y: number, at: number) => {
        const ring = rings[rippleCursor++ % rings.length];
        const max = desktop ? 118 : 50;
        const state = { radius: max * 0.25, opacity: 0.85 };
        const draw = () => {
          ring.setAttribute('cx', String(x));
          ring.setAttribute('cy', String(y));
          ring.setAttribute('r', state.radius.toFixed(1));
          ring.setAttribute('opacity', state.opacity.toFixed(3));
        };
        timeline.set(state, { radius: max * 0.25, opacity: 0.85 }, at);
        timeline.call(draw, [], at);
        timeline.to(
          state,
          { radius: max, opacity: 0, duration: 0.85, ease: 'power2.out', onUpdate: draw },
          at,
        );
      };
      const screens = scene.querySelectorAll<SVGImageElement>('[data-screen]');
      const lit = scene.querySelectorAll<SVGGElement>('[data-route]');
      const accents = scene.querySelectorAll<SVGGElement>('.device-accent');
      const timeline = gsap.timeline({
        paused: true,
        repeat: -1,
        defaults: { ease: 'steps(1)' },
        onRepeat: () => {
          hero.dataset.loop = String(Number(hero.dataset.loop || 0) + 1);
        },
      });
      hero.dataset.loop = '0';
      const screen = (name: string, at: number) => {
        timeline.set(screens, { opacity: 0 }, at);
        timeline.set(scene.querySelector(`[data-screen="${name}"]`), { opacity: 1 }, at);
      };
      function travel(route: Route, index: number, at: number, pulseOrigin = true) {
        const points = route.points,
          distance = length(points);
        const edge = scene.querySelector<SVGGElement>(`[data-route="${index}"]`)!;
        const nodes = ['west', 'relay', 'stick', 'north', 'east'];
        const pair = [
          scene.querySelector(`[data-peer="${nodes[index]}"] .device-accent`),
          scene.querySelector(`[data-peer="${nodes[index + 1]}"] .device-accent`),
        ];
        const trail = edge.querySelector<SVGPolylineElement>('polyline')!;
        const head = { covered: 0 };
        const paint = () => {
          trail.style.removeProperty('stroke-dasharray');
          trail.style.removeProperty('stroke-dashoffset');
          const drawn = Math.max(0, Math.min(distance, head.covered));
          trail.setAttribute('stroke-dasharray', `${drawn.toFixed(2)} ${distance}`);
          trail.setAttribute('stroke-dashoffset', '0');
        };
        const [source, destination] = pair;
        timeline.call(paint, [], at);
        timeline.set(edge, { opacity: 1 }, at);
        timeline.set(source, { opacity: 1 }, at);
        timeline.set(packet, { x: points[0][0], y: points[0][1], opacity: 1 }, at);
        if (pulseOrigin) ripple(points[0][0], points[0][1], at);
        let cursor = at,
          covered = 0;
        for (let i = 1; i < points.length; i++) {
          const leg =
            Math.abs(points[i][0] - points[i - 1][0]) + Math.abs(points[i][1] - points[i - 1][1]);
          const duration = (route.duration * leg) / distance;
          const from = covered;
          covered += leg;
          timeline.fromTo(
            head,
            { covered: from },
            { covered, duration, ease: 'none', immediateRender: false, onUpdate: paint },
            cursor,
          );
          timeline.to(packet, { x: points[i][0], y: points[i][1], duration, ease: 'none' }, cursor);
          cursor += duration;
        }
        timeline.set(destination, { opacity: 1 }, at + route.duration);
        ripple(points[points.length - 1][0], points[points.length - 1][1], at + route.duration);
      }
      timeline.addLabel('idle', 0);
      screen('home', 0);
      timeline.set([lit, accents, packet], { opacity: 0 }, 0);
      timeline.addLabel('writing', 0.8);
      screen('writing', 0.8);
      timeline.addLabel('confirm', 2);
      screen('confirm', 2);
      timeline.addLabel('queued', 2.6);
      screen('queued', 2.6);
      timeline.addLabel('outbound-north', 3);
      travel(routes[2], 2, 3);
      timeline.addLabel('outbound-east', 3.9);
      travel(routes[3], 3, 3.9, false);
      timeline.set([lit, accents, packet], { opacity: 0 }, 6.2);
      timeline.addLabel('inbound-west', 6.4);
      screen('home', 6.4);
      travel(routes[0], 0, 6.4);
      timeline.addLabel('inbound-relay', 8.2);
      travel(routes[1], 1, 8.2, false);
      timeline.addLabel('received', 9.2);
      screen('received', 9.2);
      timeline.addLabel('rest', 11);
      timeline.set([lit, accents, packet], { opacity: 0 }, 11.2);
      timeline.to({}, { duration: 0.2 }, 11.2);
      timeline.pause(0);
      let visible = false;
      let entrance: gsap.core.Timeline | undefined;
      let entranceComplete = entrancePlayed;
      hero.dataset.entrance = entranceComplete ? 'complete' : 'pending';
      const sync = () => {
        const paused = !visible || document.hidden;
        if (!entranceComplete) {
          entrance?.paused(paused);
          timeline.pause();
          hero.dataset.motion = paused ? 'paused' : 'entering';
        } else {
          timeline.paused(paused);
          hero.dataset.motion = paused ? 'paused' : 'playing';
        }
      };
      if (!entrancePlayed) {
        entrancePlayed = true;
        entrance = gsap.timeline({
          paused: true,
          defaults: { ease: 'steps(3)' },
          onStart: () => {
            hero.dataset.entrance = 'playing';
          },
          onComplete: () => {
            entranceComplete = true;
            hero.dataset.entrance = 'complete';
            sync();
          },
        });
        entrance
          .from(
            hero.querySelector('.hero-lead > .eyebrow'),
            { opacity: 0, y: 4, duration: 0.16 },
            0,
          )
          .from(
            hero.querySelector('h1'),
            { opacity: 0, y: 8, duration: 0.28, ease: 'steps(4)' },
            0.04,
          )
          .from(hero.querySelector('.hero-description'), { opacity: 0, y: 8, duration: 0.24 }, 0.1)
          .from(hero.querySelector('.hero-actions'), { opacity: 0, y: 8, duration: 0.24 }, 0.18)
          .from(hero.querySelector('.hero-meta'), { opacity: 0, duration: 0.2 }, 0.24)
          // Opacity only. A y tween rewrites translateX(-50%) as pixels, so the mesh slides off on resize.
          .from(scene, { opacity: 0, duration: 0.32, ease: 'steps(4)' }, 0.12)
          .from(
            scene.querySelectorAll('[data-peer]'),
            { opacity: 0, duration: 0.16, stagger: 0.04 },
            0.16,
          );
        if (desktop)
          entrance.from(
            hero.querySelector('.hero-statement'),
            { opacity: 0, y: 8, duration: 0.22 },
            0.3,
          );
      }
      // Keyboard focus never lands on a transparent CTA while the entrance is running.
      const showFocusedAction = () => {
        if (!entranceComplete) entrance?.progress(1);
      };
      hero.addEventListener('focusin', showFocusedAction);
      const observer = new IntersectionObserver(
        ([entry]) => {
          visible = entry.isIntersecting;
          sync();
        },
        { threshold: 0 },
      );
      observer.observe(hero);
      document.addEventListener('visibilitychange', sync);
      return () => {
        observer.disconnect();
        document.removeEventListener('visibilitychange', sync);
        hero.removeEventListener('focusin', showFocusedAction);
      };
    },
  );
}
