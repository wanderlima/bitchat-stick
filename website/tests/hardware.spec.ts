import { test, expect, type Page } from '@playwright/test';

async function openHardware(page:Page,reduced=false) {
  await page.setViewportSize({width:1440,height:1000});
  await page.emulateMedia({reducedMotion:reduced?'reduce':'no-preference'});
  await page.goto('');
  const root=page.locator('[data-hardware-story]');
  await page.getByRole('button',{name:'Move down',exact:true}).focus();
  await expect(root).toHaveAttribute('data-model-state','ready',{timeout:20000});
  await expect.poll(async()=>Number(await root.getAttribute('data-render-frame'))).toBeGreaterThan(0);
  return root;
}

test('3D screen and articulated joystick follow real demo actions', async ({page})=>{
  const errors:string[]=[];page.on('pageerror',error=>errors.push(error.message));
  const root=await openHardware(page);
  // Explicit walkthrough reset makes this independent of focus scrolling into the pinned range.
  await page.getByRole('button',{name:'Explore',exact:true}).click();
  await expect(root).toHaveAttribute('data-screen','home-nearby');
  const down=page.getByRole('button',{name:'Move down',exact:true});
  await root.evaluate(element=>{
    const state=window as unknown as {observedJoystickPeak:number};state.observedJoystickPeak=0;
    new MutationObserver(()=>{state.observedJoystickPeak=Math.max(state.observedJoystickPeak,Number((element as HTMLElement).dataset.joystickTilt));})
      .observe(element,{attributes:true,attributeFilter:['data-joystick-tilt']});
  });
  await down.click();
  await expect(root).toHaveAttribute('data-rendered-screen','home-channels');
  await expect(page.getByRole('status')).toHaveText('Channels selected.');
  await expect.poll(()=>page.evaluate(()=>(window as unknown as {observedJoystickPeak:number}).observedJoystickPeak)).toBeGreaterThan(.02);
  await expect(root).toHaveAttribute('data-joystick-action','neutral');
  await page.getByRole('button',{name:'Open selection',exact:true}).click();
  await expect(root).toHaveAttribute('data-rendered-screen','channels');
  await page.getByRole('button',{name:'Go back',exact:true}).click();
  await expect(root).toHaveAttribute('data-rendered-screen','home-nearby');
  expect(errors).toEqual([]);
});

test('horizontal scroll chapters synchronize the view and application', async ({page})=>{
  const root=await openHardware(page);
  await expect(root).toHaveAttribute('data-scroll-mode','horizontal');
  await page.getByRole('button',{name:'Navigate',exact:true}).click();
  await expect(root).toHaveAttribute('data-screen','home-channels');
  await expect(page.getByRole('button',{name:'Navigate',exact:true})).toHaveAttribute('aria-current','step');
  await page.getByRole('button',{name:'Open',exact:true}).click();
  await expect(root).toHaveAttribute('data-screen','channels');
  await page.getByRole('button',{name:'Receive',exact:true}).click();
  await expect(root).toHaveAttribute('data-screen','received');
  await page.getByRole('button',{name:'Reply',exact:true}).click();
  await expect(root).toHaveAttribute('data-screen','reply-method-0');
  await page.getByRole('button',{name:'Send',exact:true}).click();
  await expect(root).toHaveAttribute('data-screen','review-1-cancel');
  await expect.poll(async()=>Number(await root.getAttribute('data-progress'))).toBeGreaterThan(.98);
  await page.mouse.wheel(0,-900);
  await expect(root).toHaveAttribute('data-screen','reply-method-0');
});

test('mouse parallax is subtle; reduced motion removes pinning and parallax', async ({page})=>{
  const root=await openHardware(page);
  await page.mouse.move(1,500);
  await page.waitForTimeout(650);
  const left=Number(await root.getAttribute('data-view-yaw'));
  await page.mouse.move(1400,500);
  await expect.poll(async()=>Math.abs(Number(await root.getAttribute('data-view-yaw'))-left)).toBeGreaterThan(.02);
  const difference=Math.abs(Number(await root.getAttribute('data-view-yaw'))-left);
  expect(difference).toBeLessThan(.26);
  await page.emulateMedia({reducedMotion:'reduce'});
  await expect(root).toHaveAttribute('data-scroll-mode','steps');
  await page.getByRole('button',{name:'Navigate',exact:true}).click();
  await expect(root).toHaveAttribute('data-rendered-screen','home-channels');
  await expect.poll(async()=>Number(await root.getAttribute('data-view-yaw'))).toBeCloseTo(-.18,3);
  await page.mouse.move(10,10);await page.waitForTimeout(150);
  expect(Number(await root.getAttribute('data-view-yaw'))).toBeCloseTo(-.18,3);
});

test('mobile controls and keyboard work without horizontal overflow or scroll trapping', async ({page})=>{
  await page.setViewportSize({width:375,height:812});await page.goto('');
  const root=page.locator('[data-hardware-story]');
  await page.getByRole('button',{name:'Move down',exact:true}).focus();
  await expect(root).toHaveAttribute('data-scroll-mode','steps');
  await page.keyboard.press('ArrowDown');
  await expect(page.getByRole('status')).toHaveText('Channels selected.');
  await page.keyboard.press('ArrowRight');
  await expect(page.getByRole('status')).toHaveText('Channels opened.');
  await page.keyboard.press('ArrowLeft');
  await expect(page.getByRole('status')).toHaveText('Back to Home.');
  expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth)).toBe(true);
});

test('model failure leaves the complete accessible 2D demo usable', async ({page})=>{
  await page.route('**/models/*.glb',route=>route.abort());
  await page.goto('');
  await page.getByRole('button',{name:'Move down',exact:true}).focus();
  const root=page.locator('[data-hardware-story]');
  await expect(root).toHaveAttribute('data-model-state','fallback');
  await expect(page.locator('.control-device')).toBeVisible();
  await page.getByRole('button',{name:'Move down',exact:true}).click();
  await expect(page.getByRole('status')).toHaveText('Channels selected.');
  await expect(page.locator('[data-preview-screen="home-channels"]')).toHaveAttribute('opacity','1');
});

for(const reduce of [false,true]) test(`receive, choose a short message, cancel safely and queue a reply (reduced=${reduce})`,async({page})=>{
  const root=await openHardware(page,reduce);
  const right=page.getByRole('button',{name:'Open selection',exact:true});
  await page.getByRole('button',{name:'Receive a message',exact:true}).click();
  await expect(root).toHaveAttribute('data-rendered-screen','received');
  await expect(page.getByRole('status')).toHaveText('Message received from Test phone.');
  await right.click();await expect(root).toHaveAttribute('data-rendered-screen','reply-method-0');
  await right.click();await expect(root).toHaveAttribute('data-rendered-screen','short-0');
  await page.getByRole('button',{name:'Move down',exact:true}).click();
  await expect(root).toHaveAttribute('data-rendered-screen','short-1');
  await expect(page.getByRole('status')).toContainText('Thanks!');
  await right.click();await expect(root).toHaveAttribute('data-rendered-screen','review-1-cancel');
  await right.click();await expect(root).toHaveAttribute('data-rendered-screen','short-1');
  await expect(page.getByRole('status')).not.toContainText('queued');
  await right.click();await page.getByRole('button',{name:'Move up',exact:true}).click();
  await expect(root).toHaveAttribute('data-rendered-screen','review-1-send');
  await right.click();await expect(root).toHaveAttribute('data-rendered-screen','queued-1');
  await expect(page.getByRole('status')).toContainText('Reply queued: Thanks!');
  await right.click();await expect(root).toHaveAttribute('data-rendered-screen','received');
});

test('all 32 short replies can be browsed in both directions with the keyboard',async({page})=>{
  const root=await openHardware(page,true);
  await page.getByRole('button',{name:'Receive a message',exact:true}).click();
  const right=page.getByRole('button',{name:'Open selection',exact:true});
  await right.click();await right.click();
  await page.getByRole('button',{name:'Move up',exact:true}).focus();
  await page.keyboard.press('ArrowUp');
  await expect(root).toHaveAttribute('data-rendered-screen','short-31');
  await expect(page.getByRole('status')).toContainText('32 of 32: Got it.');
  await page.keyboard.press('ArrowDown');
  await expect(root).toHaveAttribute('data-rendered-screen','short-0');
  await expect(page.getByRole('status')).toContainText('1 of 32: Hello!');
});

test('text follows a little, then settles exactly without snapping page scroll',async({page})=>{
  const root=await openHardware(page);
  await page.getByRole('button',{name:'Navigate',exact:true}).click();
  await expect(root).toHaveAttribute('data-text-snap','settled');
  await expect.poll(async()=>Math.abs(Number(await root.getAttribute('data-progress'))-.2)).toBeLessThan(.002);
  await page.mouse.wheel(0,140);
  await expect.poll(async()=>Number(await root.getAttribute('data-progress'))).toBeGreaterThan(.21);
  await expect(root).toHaveAttribute('data-text-snap','settled');
  const offset=await root.locator('[data-chapter="1"]').evaluate(element=>new DOMMatrix(getComputedStyle(element).transform).m41);
  expect(Math.abs(offset)).toBeLessThan(.1);
  const progress=Number(await root.getAttribute('data-progress'));
  expect(progress).toBeGreaterThan(.21);expect(progress).toBeLessThan(.3);
});

test('vertical float is visible, pauses offscreen and stops with reduced motion',async({page})=>{
  const root=await openHardware(page);
  const initial=Number(await root.getAttribute('data-float-y'));
  await expect.poll(async()=>Math.abs(Number(await root.getAttribute('data-float-y'))-initial)).toBeGreaterThan(.0001);
  await page.getByRole('heading',{name:'Bitchat Stick FAQ',exact:true}).scrollIntoViewIfNeeded();
  await expect.poll(()=>root.evaluate(element=>element.getBoundingClientRect().bottom)).toBeLessThan(0);
  await page.waitForTimeout(250);
  const frame=await root.getAttribute('data-render-frame');
  await page.waitForTimeout(250);expect(await root.getAttribute('data-render-frame')).toBe(frame);
  await page.emulateMedia({reducedMotion:'reduce'});
  await page.getByRole('button',{name:'Move down',exact:true}).focus();
  await expect(root).toHaveAttribute('data-float-y','0.000000');
});
