import gsap from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';
import { ScrollSmoother } from 'gsap/ScrollSmoother';
import { reducedMotion } from './preferences';
import { sendPreviewCommand, type PreviewState } from './controls';

gsap.registerPlugin(ScrollTrigger);

export async function initHardwareStory() {
  const root = document.querySelector<HTMLElement>('[data-hardware-story]');
  if (!root) return;
  const stage = root.querySelector<HTMLElement>('[data-hardware-stage]')!;
  const track = root.querySelector<HTMLElement>('[data-hardware-chapters]')!;
  const chapters = Array.from(root.querySelectorAll<HTMLElement>('[data-chapter]'));
  const buttons = Array.from(root.querySelectorAll<HTMLButtonElement>('[data-hardware-step]'));
  const count = chapters.length,
    last = count - 1;
  let chapter = 0,
    pinned = false;
  let settle: gsap.core.Tween | undefined;
  let userControlled = false;
  const inputEvents = new AbortController();
  // Explicit app input takes precedence over focus-induced/smoothed scrolling.
  root.addEventListener(
    'preview-change',
    (event) => {
      if ((event as CustomEvent<PreviewState>).detail.source !== 'scroll') userControlled = true;
    },
    { signal: inputEvents.signal },
  );
  root.addEventListener(
    'pointerdown',
    (event) => {
      if ((event.target as Element).closest('[data-control],[data-joy-action]'))
        userControlled = true;
    },
    { signal: inputEvents.signal },
  );
  root.addEventListener(
    'keydown',
    (event) => {
      if ((event.target as Element).closest('[data-control],[data-joy-action]'))
        userControlled = true;
    },
    { signal: inputEvents.signal },
  );
  root.addEventListener(
    'focusin',
    (event) => {
      if ((event.target as Element).closest('[data-control],[data-joy-action],[data-demo-receive]'))
        userControlled = true;
    },
    { signal: inputEvents.signal },
  );
  window.addEventListener(
    'wheel',
    () => {
      userControlled = false;
    },
    { passive: true, signal: inputEvents.signal },
  );
  window.addEventListener(
    'touchmove',
    () => {
      userControlled = false;
    },
    { passive: true, signal: inputEvents.signal },
  );
  window.addEventListener(
    'keydown',
    (event) => {
      if (['PageDown', 'PageUp', 'Home', 'End'].includes(event.key)) userControlled = false;
    },
    { signal: inputEvents.signal },
  );
  const state = { progress: 0 };
  function update(progress: number, fromScroll = false) {
    state.progress = progress;
    root!.dataset.progress = progress.toFixed(3);
    root!.dispatchEvent(new CustomEvent('hardware-view', { detail: { progress } }));
    const index = Math.min(last, Math.round(progress * last));
    if (index !== chapter) {
      chapter = index;
      gsap.set(chapters, { x: 0 });
      gsap.to(track, {
        x: (-index * track.clientWidth) / count,
        duration: reducedMotion() ? 0 : 0.28,
        ease: 'power3.out',
        overwrite: true,
      });
      chapters.forEach((element, i) => {
        element.inert = i !== index;
        element.setAttribute('aria-hidden', String(i !== index));
      });
      buttons.forEach((button, i) => {
        if (i === index) button.setAttribute('aria-current', 'step');
        else button.removeAttribute('aria-current');
      });
      if (fromScroll && !userControlled) sendPreviewCommand({ chapter: index, source: 'scroll' });
    }
    settle?.kill();
    if (pinned && !reducedMotion()) {
      // Follow the gesture by only a few pixels, then settle the text itself.
      // No page-scroll snapping: focus, app controls and ScrollSmoother stay independent.
      const drift = gsap.utils.clamp(-14, 14, -(progress * last - index) * 52);
      root!.dataset.textSnap = 'following';
      gsap.to(chapters[index], { x: drift, duration: 0.1, ease: 'power2.out', overwrite: true });
      settle = gsap.delayedCall(0.14, () => {
        gsap.to(chapters, {
          x: 0,
          duration: 0.22,
          ease: 'power3.out',
          overwrite: true,
          onComplete: () => {
            root!.dataset.textSnap = 'settled';
          },
        });
      });
    } else {
      gsap.set(chapters, { x: 0 });
      root!.dataset.textSnap = 'settled';
    }
  }
  // The controls remain usable while WebGL and the model load independently.
  const visibility = new IntersectionObserver(
    (entries) => {
      if (!entries[0].isIntersecting) return;
      visibility.disconnect();
      import('./hardware-scene')
        .then((module) => module.mountHardwareScene(root))
        .catch(() => {
          root.dataset.modelState = 'fallback';
        });
    },
    { rootMargin: '700px' },
  );
  visibility.observe(root);
  await document.fonts.ready;
  const media = gsap.matchMedia();
  media.add(
    {
      all: '(min-width: 0px)',
      desktop: '(min-width: 901px) and (min-height: 740px)',
      reduce: '(prefers-reduced-motion: reduce)',
    },
    (context) => {
      const signal = new AbortController();
      pinned = Boolean(context.conditions?.desktop) && !reducedMotion();
      let tween: gsap.core.Tween | undefined;
      if (pinned) {
        root.dataset.scrollMode = 'horizontal';
        gsap.set(track, { x: (-chapter * track.clientWidth) / count });
        tween = gsap.fromTo(
          state,
          { progress: 0 },
          {
            progress: 1,
            ease: 'none',
            duration: 1,
            onUpdate: () => update(state.progress, true),
            scrollTrigger: {
              id: 'hardware-story',
              trigger: root,
              pin: stage,
              start: 'top top',
              end: () => `+=${Math.max(3000, last * 680)}`,
              scrub: 0.35,
              invalidateOnRefresh: true,
              anticipatePin: 1,
              onRefresh: () => gsap.set(track, { x: (-chapter * track.clientWidth) / count }),
            },
          },
        );
      } else {
        root.dataset.scrollMode = 'steps';
        gsap.set(track, { x: () => (-chapter * track.clientWidth) / count });
      }
      buttons.forEach((button, index) => {
        button.disabled = false;
        button.addEventListener(
          'click',
          () => {
            userControlled = false;
            sendPreviewCommand({ chapter: index, source: 'scroll' });
            const trigger = tween?.scrollTrigger;
            if (trigger) {
              const y = trigger.start + (index / last) * (trigger.end - trigger.start);
              const smoother = ScrollSmoother.get();
              // Jump on an explicit step. A smoothed catch-up keeps the button
              // moving, so the next pointer action never becomes stable.
              if (smoother) smoother.scrollTo(y, false);
              else window.scrollTo({ top: y, behavior: 'auto' });
            } else {
              update(index / last, true);
              gsap.set(track, { x: (-index * track.clientWidth) / count });
            }
          },
          { signal: signal.signal },
        );
      });
      const resize = () => {
        if (!pinned) gsap.set(track, { x: (-chapter * track.clientWidth) / count });
      };
      window.addEventListener('resize', resize, { signal: signal.signal, passive: true });
      ScrollTrigger.sort();
      ScrollTrigger.refresh();
      return () => {
        signal.abort();
        settle?.kill();
        gsap.killTweensOf(track);
        gsap.killTweensOf(chapters);
        gsap.set(chapters, { x: 0 });
        delete root.dataset.scrollMode;
      };
    },
  );
  window.addEventListener(
    'pagehide',
    () => {
      media.revert();
      visibility.disconnect();
      inputEvents.abort();
    },
    { once: true },
  );
}
