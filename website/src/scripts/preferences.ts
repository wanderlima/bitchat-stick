import gsap from 'gsap';

export function reducedMotion() {
  return document.documentElement.hasAttribute('data-reduce-motion') || window.matchMedia('(prefers-reduced-motion: reduce)').matches;
}

export function initMotionPreference() {
  const input=document.querySelector<HTMLInputElement>('#reduce-motion');
  const panel=document.querySelector<HTMLElement>('[data-motion-preference]');
  if (!input || !panel) return;
  panel.hidden=false;
  const shortcut=document.querySelector<HTMLElement>('[data-motion-shortcut]');
  if(shortcut) shortcut.hidden=false;
  const system=window.matchMedia('(prefers-reduced-motion: reduce)');
  const sync=() => {
    input.checked=document.documentElement.hasAttribute('data-reduce-motion') || system.matches;
    input.disabled=system.matches;
    document.querySelector<HTMLElement>('#motion-system-note')!.hidden=!system.matches;
  };
  input.addEventListener('change',()=>{
    document.documentElement.toggleAttribute('data-reduce-motion',input.checked);
    gsap.matchMediaRefresh();
  });
  system.addEventListener('change',()=>{ sync(); gsap.matchMediaRefresh(); });
  sync();
}
