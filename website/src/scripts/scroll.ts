import gsap from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';
import { ScrollSmoother } from 'gsap/ScrollSmoother';
import { reducedMotion } from './preferences';

gsap.registerPlugin(ScrollTrigger, ScrollSmoother);
export async function initSmoothScroll() {
  await document.fonts.ready;
  const media = gsap.matchMedia();
  media.add('(min-width: 901px) and (pointer: fine)', () => {
    if (reducedMotion()) return;
    const root = document.documentElement;
    root.setAttribute('data-smooth-scroll', '');
    let anchorFocus = false;
    const wrapper = document.querySelector<HTMLElement>('#smooth-wrapper')!;
    const smoother = ScrollSmoother.create({
      wrapper,
      content: '#smooth-content',
      smooth: 0.65,
      smoothTouch: 0,
      effects: false,
      normalizeScroll: false,
      onFocusIn: () => (anchorFocus ? false : undefined),
    });
    const controller = new AbortController();
    let folding = false;
    // overflow:hidden still scrolls. scrollIntoView then offsets this wrapper on top
    // of the smoother transform, and the next focus snaps it back under the pointer.
    wrapper.addEventListener(
      'scroll',
      () => {
        if (folding) return;
        const delta = wrapper.scrollTop;
        if (!delta) return;
        folding = true;
        wrapper.scrollTop = 0;
        smoother.scrollTop(smoother.scrollTop() + delta);
        folding = false;
      },
      { signal: controller.signal },
    );
    document.querySelectorAll<HTMLAnchorElement>('a[href^="#"]').forEach((link) => {
      link.addEventListener(
        'click',
        (event) => {
          if (event.ctrlKey || event.metaKey || event.shiftKey || event.altKey) return;
          const hash = link.getAttribute('href')!;
          const target = document.getElementById(decodeURIComponent(hash.slice(1)));
          if (!target) return;
          event.preventDefault();
          history.pushState(null, '', hash);
          if (!target.matches('a,button,input,select,textarea,summary,[tabindex]')) {
            target.tabIndex = -1;
            target.addEventListener('blur', () => target.removeAttribute('tabindex'), {
              once: true,
            });
          }
          anchorFocus = true;
          target.focus({ preventScroll: true });
          anchorFocus = false;
          smoother.scrollTo(target, true, 'top 24px');
        },
        { signal: controller.signal },
      );
    });
    return () => {
      controller.abort();
      smoother.kill();
      root.removeAttribute('data-smooth-scroll');
    };
  });
}
