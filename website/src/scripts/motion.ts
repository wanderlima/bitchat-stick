import gsap from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';
import { SplitText } from 'gsap/SplitText';
import { reducedMotion } from './preferences';

gsap.registerPlugin(ScrollTrigger, SplitText);
export async function initMotion() {
  await document.fonts.ready;
  const media=gsap.matchMedia();
  media.add('(min-width: 0px)',context=>{
    if(reducedMotion()) return;
    document.querySelectorAll<HTMLElement>('[data-reveal-title]').forEach(heading=>{
      const split=SplitText.create(heading,{type:'words,chars',tag:'span',aria:'auto'});
      const groupSize=Math.ceil(split.chars.length/17);
      ScrollTrigger.create({trigger:heading,start:'top 88%',once:true,onEnter:()=>{
        context.add(()=>{gsap.from(split.chars,{opacity:0,duration:.08,stagger:index=>Math.floor(index/groupSize)*.02,ease:'steps(1)'});});
      }});
    });
    document.querySelectorAll<HTMLElement>('section.section').forEach(section=>{
      const items=section.querySelectorAll<HTMLElement>('[data-enter-item], .section-heading > p, :scope > .action, :scope > .text-link');
      ScrollTrigger.batch(items,{start:'top 92%',once:true,interval:.06,batchMax:4,onEnter:batch=>{
        const targets=batch.filter(element=>!element.contains(document.activeElement));
        if(targets.length) context.add(()=>{gsap.from(targets,{y:12,opacity:0,duration:.22,stagger:.055,ease:'steps(3)'});});
      }});
    });
    ScrollTrigger.create({trigger:'[data-terminal]',start:'top 85%',once:true,onEnter:()=>{
      context.add(()=>{
        gsap.from('.terminal-line',{y:8,opacity:0,duration:.18,stagger:.12,ease:'steps(3)'});
        gsap.from('.upload-progress',{scaleX:0,duration:1.2,ease:'steps(12)'});
      });
    }});
    const controller=new AbortController();
    const signal=controller.signal;
    document.querySelectorAll<HTMLButtonElement>('[data-control]').forEach(button=>{
      button.addEventListener('pointerdown',()=>context.add(()=>{gsap.to(button,{y:2,duration:.1,ease:'steps(2)',overwrite:true});}),{signal});
      const release=()=>context.add(()=>{gsap.to(button,{y:0,duration:.06,ease:'steps(1)',overwrite:true});});
      button.addEventListener('pointerup',release,{signal});
      button.addEventListener('pointercancel',release,{signal});
      button.addEventListener('pointerleave',release,{signal});
    });
    document.querySelectorAll<HTMLDetailsElement>('.faq-list details').forEach(details=>{
      let pointer=false;
      details.querySelector('summary')!.addEventListener('pointerdown',()=>{pointer=true;},{signal});
      details.querySelector('summary')!.addEventListener('keydown',()=>{pointer=false;},{signal});
      details.addEventListener('toggle',()=>{
        if(details.open && pointer) context.add(()=>{gsap.fromTo(details.querySelector('.faq-answer'),{opacity:0},{opacity:1,duration:.12,ease:'steps(2)',overwrite:true});});
        pointer=false;ScrollTrigger.refresh();
      },{signal});
    });
    ScrollTrigger.refresh();
    return ()=>controller.abort();
  });
}
