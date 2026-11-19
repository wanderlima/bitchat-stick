import gsap from 'gsap';
import { desktopScene, mobileScene, peerNames } from '../data/mesh';
import { reducedMotion } from './preferences';

/** Two stories: the Stick sends one message, then receives one. The envelope glides, and peers stay lit until that message arrives. */
export async function initHero() {
  const hero = document.querySelector<HTMLElement>('.hero');
  if (!hero) {
    return;
  }
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
      desktop: '(width >= 640px)',
      mobile: '(width < 640px)',
      reduce: '(prefers-reduced-motion: reduce)',
    },
    (context) => {
      if (reducedMotion()) {
        entrancePlayed = true;
        hero.dataset.entrance = 'static';
        hero.dataset.motion = 'reduced';
        return;
      }
      const desktop = Boolean(context.conditions?.desktop);
      const scene = hero.querySelector<SVGSVGElement>(
        `[data-mesh="${desktop ? 'desktop' : 'mobile'}"]`,
      )!;
      const { routes } = desktop ? desktopScene : mobileScene;
      const packet = scene.querySelector<SVGGElement>('.packet')!;
      const ripple = (name: (typeof peerNames)[number], at: number) => {
        const ring = scene.querySelector(`[data-ripple="${name}"]`);
        const radius = name === 'stick' ? (desktop ? 210 : 84) : desktop ? 110 : 48;
        timeline.fromTo(
          ring,
          { attr: { r: 0, opacity: 0.9 } },
          {
            attr: { r: radius, opacity: 0 },
            duration: 0.95,
            ease: 'power1.out',
            immediateRender: false,
          },
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
      function travel(index: number, at: number, pulseOrigin = true) {
        const { duration } = routes[index];
        const edge = scene.querySelector<SVGGElement>(`[data-route="${index}"]`)!;
        const trail = edge.querySelector('polyline')!;
        const distance = trail.getTotalLength();
        const source = peerNames[index];
        const destination = peerNames[index + 1];
        const accent = (name: string) =>
          scene.querySelector(`[data-peer="${name}"] .device-accent`);
        const head = { covered: 0 };
        const paint = () => {
          const { x, y } = trail.getPointAtLength(head.covered);
          packet.setAttribute('transform', `translate(${x} ${y})`);
          trail.setAttribute('stroke-dasharray', `${head.covered} ${distance}`);
        };

        timeline.set(edge, { opacity: 1 }, at);
        timeline.set(accent(source), { opacity: 1 }, at);
        timeline.set(packet, { opacity: 1 }, at);
        timeline.fromTo(
          head,
          { covered: 0 },
          { covered: distance, duration, ease: 'none', immediateRender: false, onUpdate: paint },
          at,
        );
        timeline.set(packet, { opacity: 0 }, at + duration);
        timeline.set(accent(destination), { opacity: 1 }, at + duration);
        if (pulseOrigin) ripple(source, at);
        ripple(destination, at + duration);
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
      travel(2, 3);
      timeline.addLabel('outbound-east', 3.9);
      travel(3, 3.9, false);
      timeline.set([lit, accents, packet], { opacity: 0 }, 6.2);
      timeline.addLabel('inbound-west', 6.4);
      screen('home', 6.4);
      travel(0, 6.4);
      timeline.addLabel('inbound-relay', 8.2);
      travel(1, 8.2, false);
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
