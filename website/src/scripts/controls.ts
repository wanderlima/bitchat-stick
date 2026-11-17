import presets from '../data/short-messages.json';

export type Direction = 'up' | 'down' | 'left' | 'right' | 'confirm' | 'neutral';
export type InputSource = 'pointer' | 'keyboard' | 'scroll';
export interface PreviewState { screen: string; direction: Direction; source: InputSource; }
type Command = { action: Exclude<Direction,'neutral'>; source: InputSource } | { chapter: number; source: 'scroll' };
type Flow='menu'|'page'|'received'|'reply'|'shorts'|'keyboard'|'review'|'queued'|'start';

export function sendPreviewCommand(command: Command) {
  document.querySelector('[data-controls]')?.dispatchEvent(new CustomEvent('preview-command', { detail: command }));
}

export function initControls() {
  const root=document.querySelector<HTMLElement>('[data-controls]');
  if(!root) return;
  const feedback=root.querySelector<HTMLElement>('.control-feedback')!;
  const hint=root.querySelector<HTMLElement>('[data-demo-hint]')!;
  const images=root.querySelectorAll<SVGImageElement>('[data-preview-screen]');
  const dynamic=root.querySelector<SVGImageElement>('[data-preview-dynamic]')!;
  const screenBase=root.querySelector<HTMLElement>('[data-screen-base]')!.dataset.screenBase!;
  const names=['Nearby','Channels','InfoBot','Settings'],keys=['nearby','channels','infobot','settings'];
  const methods=['Short msg','Keyboard','Welcome msg'];
  let selected=0,flow:Flow='menu',method=0,preset=0,send=false;
  let reviewOrigin:Flow='shorts';
  function view():{screen:string;status:string;hint:string} {
    switch(flow){
      case 'start':return{screen:'start',status:'Start screen.',hint:'Press A or push right to start.'};
      case 'received':return{screen:'received',status:'Message received from Test phone.',hint:'“Hello from a phone.” Push right to reply; left returns to Channels.'};
      case 'reply':return{screen:`reply-method-${method}`,status:`Reply to Test phone. ${methods[method]} selected.`,hint:'Choose a reply method with up/down. Push right to open Short msg.'};
      case 'shorts':return{screen:`short-${preset}`,status:`Short message ${preset+1} of ${presets.length}: ${presets[preset]}`,hint:'Up/down browse all 32 replies. Right reviews your choice. Left goes back.'};
      case 'keyboard':return{screen:'keyboard-sample',status:'Keyboard preview. Sample draft: Hello!',hint:'This keyboard screen shows a sample draft. Right reviews it; left chooses another method.'};
      case 'review':return{screen:`review-${preset}-${send?'send':'cancel'}`,status:`Review reply: ${presets[preset]} ${send?'Send':'Cancel'} selected.`,hint:'Recipient: Test phone. Up selects Send; down selects Cancel. Right confirms.'};
      case 'queued':return{screen:`queued-${preset}`,status:`Reply queued: ${presets[preset]} Simulated demo.`,hint:'Your reply is queued in this simulation. Right returns to the message. No real message is sent.'};
      case 'page':return{screen:keys[selected],status:`${names[selected]} opened.`,hint:selected===1?'Push right to read the latest message. Left or hold B returns to Home.':selected===0?'Push right to reply to Test phone. Left returns to Home.':'Push left or hold B to return to Home.'};
      default:return{screen:`home-${keys[selected]}`,status:`${names[selected]} selected.`,hint:'Up/down choose. Right opens. Left returns to Start.'};
    }
  }
  function render(direction:Direction='neutral',source:InputSource='pointer') {
    const state=view();let found=false;
    images.forEach(image=>{const active=image.dataset.previewScreen===state.screen;found ||= active;image.setAttribute('opacity',active?'1':'0');});
    dynamic.setAttribute('opacity',found?'0':'1');
    if(!found)dynamic.setAttribute('href',`${screenBase}${state.screen}.png`);
    root!.dataset.screen=state.screen;root!.dataset.flow=flow;root!.dataset.shortMessage=presets[preset];
    feedback.textContent=state.status;hint.textContent=state.hint;
    root!.dispatchEvent(new CustomEvent<PreviewState>('preview-change',{detail:{screen:state.screen,direction,source}}));
  }
  function receive(source:InputSource='pointer') {selected=1;flow='received';method=0;preset=0;send=false;render('neutral',source);}
  function back(source:InputSource='pointer') {
    if(flow==='menu'){flow='start';render('left',source);return;}
    if(flow==='received'){flow='page';selected=1;render('left',source);return;}
    if(flow==='reply'||flow==='queued'){flow='received';render('left',source);return;}
    if(flow==='shorts'||flow==='keyboard'){flow='reply';render('left',source);return;}
    if(flow==='review'){flow=reviewOrigin;render('left',source);return;}
    flow='menu';selected=0;render('left',source);feedback.textContent='Back to Home.';
  }
  function act(action:Exclude<Direction,'neutral'>,source:InputSource) {
    if(action==='left'){back(source);return;}
    const confirm=action==='confirm'||action==='right',step=action==='up'?-1:1;
    switch(flow){
      case 'start':if(confirm){flow='menu';selected=0;}break;
      case 'menu':if(confirm)flow='page';else selected=(selected+step+names.length)%names.length;break;
      case 'page':if(confirm&&selected===1){receive(source);return;}if(confirm&&selected===0){flow='reply';method=0;}break;
      case 'received':if(confirm){flow='reply';method=0;}break;
      case 'reply':
        if(!confirm)method=(method+step+methods.length)%methods.length;
        else if(method===0)flow='shorts';
        else if(method===1){flow='keyboard';preset=0;}
        else{preset=0;flow='review';reviewOrigin='reply';send=false;}
        break;
      case 'shorts':if(confirm){flow='review';reviewOrigin='shorts';send=false;}else preset=(preset+step+presets.length)%presets.length;break;
      case 'keyboard':if(confirm){flow='review';reviewOrigin='keyboard';send=false;}break;
      case 'review':if(confirm)flow=send?'queued':reviewOrigin;else send=action==='up';break;
      case 'queued':if(confirm)flow='received';break;
    }
    render(action,source);
  }
  root.addEventListener('preview-command',event=>{
    const command=(event as CustomEvent<Command>).detail;
    if('chapter' in command){
      selected=command.chapter===0?0:1;method=0;send=false;
      flow=(['menu','menu','page','received','reply','review'] as const)[command.chapter]??'menu';
      if(command.chapter===5){preset=1;reviewOrigin='shorts';}
      render(command.chapter===0||command.chapter===3?'neutral':command.chapter===1?'down':'right','scroll');
    }else act(command.action,command.source);
  });
  root.querySelector<HTMLButtonElement>('[data-demo-receive]')!.disabled=false;
  root.querySelector('[data-demo-receive]')!.addEventListener('click',event=>receive((event as MouseEvent).detail===0?'keyboard':'pointer'));
  root.querySelectorAll<HTMLButtonElement>('[data-joy-action]').forEach(button=>{
    button.disabled=false;
    button.addEventListener('click',event=>act(button.dataset.joyAction as Exclude<Direction,'neutral'>,event.detail===0?'keyboard':'pointer'));
  });
  root.querySelector('[data-joystick-pad]')?.addEventListener('keydown',event=>{
    const key=event as KeyboardEvent;
    const action=({ArrowUp:'up',ArrowDown:'down',ArrowLeft:'left',ArrowRight:'right'} as const)[key.key as 'ArrowUp'];
    if(action){key.preventDefault();if(!key.repeat)act(action,'keyboard');}
  });
  root.querySelectorAll<HTMLButtonElement>('[data-control]').forEach(button=>{
    button.disabled=false;const key=button.dataset.control;
    let longPress=false,timer:ReturnType<typeof setTimeout>|undefined,pressStarted=0;
    const hold=700;
    button.addEventListener('click',event=>{if(longPress){longPress=false;return;}act(key==='A'?'confirm':key==='B'?'up':'down',event.detail===0?'keyboard':'pointer');});
    const clear=()=>{clearTimeout(timer);timer=undefined;};
    // A busy frame can deliver pointerup before the timer callback. The held duration still counts.
    const finishHold=(source:InputSource)=>{
      if(!longPress&&pressStarted&&performance.now()-pressStarted>=hold){longPress=true;back(source);}
      pressStarted=0;clear();
    };
    button.addEventListener('pointerdown',event=>{
      longPress=false;pressStarted=0;
      if(key!=='B')return;
      pressStarted=performance.now();
      try{button.setPointerCapture(event.pointerId);}catch{/* The pointer can already be released. */}
      timer=setTimeout(()=>{if(!pressStarted)return;longPress=true;pressStarted=0;back();},hold);
    });
    button.addEventListener('pointerup',()=>finishHold('pointer'));
    button.addEventListener('pointercancel',()=>{pressStarted=0;clear();longPress=false;});
    button.addEventListener('keydown',event=>{
      if(key==='B'&&(event.key===' '||event.key==='Enter')){event.preventDefault();if(!event.repeat){longPress=false;pressStarted=performance.now();timer=setTimeout(()=>{if(!pressStarted)return;longPress=true;pressStarted=0;back('keyboard');},hold);}}
    });
    button.addEventListener('keyup',event=>{
      if(key!=='B'||(event.key!==' '&&event.key!=='Enter'))return;
      event.preventDefault();finishHold('keyboard');button.click();
    });
    button.addEventListener('blur',()=>{pressStarted=0;clear();});
  });
  render();
}
