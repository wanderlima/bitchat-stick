import gsap from 'gsap';
import { desktopRoutes, mobileRoutes, length, type Route } from '../data/mesh';
import { reducedMotion } from './preferences';

/** One connected graph, one envelope, active devices and route highlighted together. */
export async function initHero() {
  const hero=document.querySelector<HTMLElement>('.hero');
  if (!hero) return;
  const sources=[...new Set(Array.from(hero.querySelectorAll<SVGImageElement>('[data-screen]')).map(image=>image.getAttribute('href')!))];
  const ready=await Promise.all(sources.map(source=>{const image=new Image();image.src=source;return image.decode();})).then(()=>true,()=>false);
  if(!ready){hero.dataset.motion='static';return;}
  const media=gsap.matchMedia();
  let entrancePlayed=false;
  media.add({ desktop:'(min-width: 901px)', mobile:'(max-width: 900px)', reduce:'(prefers-reduced-motion: reduce)' }, context=>{
    if (reducedMotion()) { entrancePlayed=true;hero.dataset.entrance='static';hero.dataset.motion='reduced';return; }
    const desktop=context.conditions?.desktop;
    const scene=hero.querySelector<SVGSVGElement>(`[data-mesh="${desktop ? 'desktop' : 'mobile'}"]`)!;
    const routes=desktop ? desktopRoutes : mobileRoutes;
    const packet=scene.querySelector<SVGGElement>('.packet')!;
    const screens=scene.querySelectorAll<SVGImageElement>('[data-screen]');
    const lit=scene.querySelectorAll<SVGGElement>('[data-route]');
    const accents=scene.querySelectorAll<SVGGElement>('.device-accent');
    const timeline=gsap.timeline({paused:true,repeat:-1,defaults:{ease:'steps(1)'},onRepeat:()=>{ hero.dataset.loop=String(Number(hero.dataset.loop || 0)+1); }});
    hero.dataset.loop='0';
    const screen=(name:string, at:number)=>{
      timeline.set(screens,{opacity:0},at);
      timeline.set(scene.querySelector(`[data-screen="${name}"]`),{opacity:1},at);
    };
    function travel(route:Route,index:number,at:number) {
      const points=route.points, distance=length(points);
      const edge=scene.querySelector<SVGGElement>(`[data-route="${index}"]`)!;
      const nodes=['west','relay','stick','north','east'];
      const pair=[scene.querySelector(`[data-peer="${nodes[index]}"] .device-accent`),scene.querySelector(`[data-peer="${nodes[index+1]}"] .device-accent`)];
      timeline.set([lit,accents],{opacity:0},at);
      timeline.set(edge,{opacity:1},at);
      timeline.set(pair,{opacity:1},at);
      timeline.set(packet,{x:points[0][0],y:points[0][1],opacity:1},at);
      let cursor=at;
      for(let i=1;i<points.length;i++) {
        const leg=Math.abs(points[i][0]-points[i-1][0])+Math.abs(points[i][1]-points[i-1][1]);
        const duration=route.duration*leg/distance;
        const ease=`steps(${Math.max(1,Math.round(route.steps*leg/distance))})`;
        const segment=edge.querySelector<SVGRectElement>(`[data-leg="${i-1}"]`)!;
        const property=segment.dataset.axis==='x' ? 'scaleX' : 'scaleY';
        timeline.set(segment,{[property]:0,svgOrigin:`${points[i-1][0]} ${points[i-1][1]}`},at);
        timeline.to(segment,{[property]:1,duration,ease},cursor);
        timeline.to(packet,{x:points[i][0],y:points[i][1],duration,ease},cursor);
        cursor+=duration;
      }
      timeline.set(packet,{opacity:0},at+route.duration);
      // Hold the complete connection and both device silhouettes through the 120ms hop.
      timeline.to([edge,...pair],{opacity:0,duration:.12,ease:'steps(2)'},at+route.duration);
    }
    timeline.addLabel('idle',0);
    screen('home',0);
    timeline.set([lit,accents,packet],{opacity:0},0);
    timeline.addLabel('writing',.8);screen('writing',.8);
    timeline.addLabel('confirm',2);screen('confirm',2);
    timeline.addLabel('queued',2.6);screen('queued',2.6);
    timeline.addLabel('outbound-north',3);travel(routes[2],2,3);
    timeline.addLabel('outbound-east',4.02);travel(routes[3],3,4.02);
    timeline.set([lit,accents,packet],{opacity:0},6.34);
    timeline.addLabel('inbound-west',6.46);screen('home',6.46);travel(routes[0],0,6.46);
    timeline.addLabel('inbound-relay',8.38);travel(routes[1],1,8.38);
    timeline.addLabel('received',9.38);screen('received',9.38);
    timeline.addLabel('relay-north',11.2);travel(routes[2],2,11.2);
    timeline.addLabel('relay-east',12.22);travel(routes[3],3,12.22);
    timeline.addLabel('rest',13.54);
    timeline.set([lit,accents,packet],{opacity:0},15.34);
    timeline.to({},{duration:.12},15.34);
    timeline.pause(0);
    let visible=false;
    let entrance:gsap.core.Timeline | undefined;
    let entranceComplete=entrancePlayed;
    hero.dataset.entrance=entranceComplete ? 'complete' : 'pending';
    const sync=()=>{
      const paused=!visible || document.hidden;
      if(!entranceComplete){
        entrance?.paused(paused);
        timeline.pause();
        hero.dataset.motion=paused ? 'paused' : 'entering';
      } else {
        timeline.paused(paused);
        hero.dataset.motion=paused ? 'paused' : 'playing';
      }
    };
    if(!entrancePlayed){
      entrancePlayed=true;
      entrance=gsap.timeline({paused:true,defaults:{ease:'steps(3)'},onStart:()=>{hero.dataset.entrance='playing';},onComplete:()=>{
        entranceComplete=true;hero.dataset.entrance='complete';sync();
      }});
      entrance.from(hero.querySelector('.hero-content > .eyebrow'),{opacity:0,y:4,duration:.16},0)
        .from(hero.querySelector('h1'),{opacity:0,y:8,duration:.28,ease:'steps(4)'},.04)
        .from(hero.querySelector('.hero-description'),{opacity:0,y:8,duration:.24},.10)
        .from(hero.querySelector('.hero-actions'),{opacity:0,y:8,duration:.24},.18)
        .from(hero.querySelector('.hero-meta'),{opacity:0,duration:.20},.24)
        .from(scene,{opacity:0,y:12,duration:.32,ease:'steps(4)'},.12)
        .from(scene.querySelectorAll('[data-peer]'),{opacity:0,duration:.16,stagger:.04},.16);
      if(desktop) entrance.from(hero.querySelector('.hero-statement'),{opacity:0,y:8,duration:.22},.30);
    }
    // Keyboard focus never lands on a transparent CTA while the entrance is running.
    const showFocusedAction=()=>{if(!entranceComplete) entrance?.progress(1);};
    hero.addEventListener('focusin',showFocusedAction);
    const observer=new IntersectionObserver(([entry])=>{visible=entry.isIntersecting;sync();},{threshold:0});
    observer.observe(hero);
    document.addEventListener('visibilitychange',sync);
    return ()=>{observer.disconnect();document.removeEventListener('visibilitychange',sync);hero.removeEventListener('focusin',showFocusedAction);};
  });
}
