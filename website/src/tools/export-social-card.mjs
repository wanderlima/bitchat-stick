import { chromium } from '@playwright/test';
import { fileURLToPath } from 'node:url';

// Share image: hero copy on the left, the Stick with one firmware screen on the right.
// Routes and the other peers stay out of the card.
const browser = await chromium.launch();
try {
  const page = await browser.newPage({
    viewport: { width: 1200, height: 630 },
    reducedMotion: 'reduce',
  });
  await page.addInitScript(() => {
    globalThis.document.documentElement.dataset.socialCard = 'true';
  });
  await page.goto('http://127.0.0.1:4322/bitchat-stick/');
  await page.evaluate(() => globalThis.document.fonts.ready);
  await page.locator('.hero .eyebrow').evaluate((element) => {
    element.textContent = 'BITCHAT STICK / OPEN SOURCE / ESP32';
  });
  await page.locator('.hero-caption p').evaluate((element) => {
    element.textContent = 'UI screens are software renders with simulated messages.';
  });
  await page.evaluate(() => {
    for (const image of globalThis.document.querySelectorAll('[data-screen]')) {
      const active = image.getAttribute('data-screen') === 'home';
      image.setAttribute('opacity', active ? '1' : '0');
      image.style.opacity = active ? '1' : '0';
    }
  });
  await page.addStyleTag({
    content: `
    .site-header,.site-footer,.skip-link,main>.section,.hero-actions,.hero-meta,.hero-statement,.mobile-graph { display:none!important }
    .mesh-routes,.lit-routes,.ripples,.packet,.hero-peers,[data-peer]:not([data-peer="stick"]) { display:none!important }
    .hero { width:1200px;height:630px;min-height:630px;max-width:none;margin:0;overflow:hidden;display:flex;align-items:center }
    .hero-content { width:auto;margin:0 48px;padding:0;z-index:1 }
    .hero-lead > .eyebrow,.hero-intro > *,.hero-statement,.hero-caption { background:transparent!important;box-shadow:none!important }
    .hero-intro { width:640px;margin-top:0 }
    h1 { font-size:60px;line-height:64px }
    .hero-description { width:500px;font-size:20px;line-height:30px }
    .mesh-desktop { position:absolute!important;left:0;top:0;width:1800px;height:auto;transform:none!important;margin:0;z-index:2 }
    .hero-device { opacity:1!important;z-index:6!important }
    .hero-follow { position:static }
    .hero-caption { position:static;width:440px;max-width:440px;margin:28px 0 0;font-size:14px;line-height:22px }
  `,
  });
  await placeDevice(page);
  await page.waitForFunction(
    () => {
      const hero = globalThis.document.querySelector('.hero');
      const state = hero?.getAttribute('data-device');
      return state === 'ready' || state === 'fallback';
    },
    { timeout: 20000 },
  );
  await placeDevice(page);
  await page.evaluate(() => globalThis.dispatchEvent(new Event('resize')));
  await page.waitForFunction(
    () => {
      const hero = globalThis.document.querySelector('.hero');
      const host = globalThis.document.querySelector('[data-hero-device]');
      if (hero?.getAttribute('data-device') === 'fallback') {
        return true;
      }
      return host?.getAttribute('data-rendered-screen') === 'home';
    },
    { timeout: 20000 },
  );
  await page.evaluate(
    () =>
      new Promise((resolve) => {
        globalThis.requestAnimationFrame(() => globalThis.requestAnimationFrame(resolve));
      }),
  );
  await page.screenshot({
    path: fileURLToPath(new URL('../../public/social-card.png', import.meta.url)),
  });
} finally {
  await browser.close();
}

async function placeDevice(page) {
  await page.evaluate(() => {
    const mesh = globalThis.document.querySelector('.mesh-desktop');
    const hero = globalThis.document.querySelector('.hero');
    const slot = [...globalThis.document.querySelectorAll('[data-device-slot]')].find(
      (element) => element.getClientRects().length,
    );
    if (!mesh || !hero || !slot) {
      throw new Error('Social card device slot was not found.');
    }
    const heroBox = hero.getBoundingClientRect();
    const slotBox = slot.getBoundingClientRect();
    const meshBox = mesh.getBoundingClientRect();
    const desiredWidth = 480;
    const nextWidth = meshBox.width * (desiredWidth / slotBox.width);
    mesh.style.width = `${nextWidth}px`;
    const placed = slot.getBoundingClientRect();
    const shiftedMesh = mesh.getBoundingClientRect();
    const desiredLeft = heroBox.right - 56 - placed.width;
    const desiredTop = heroBox.top + (heroBox.height - placed.height) / 2 - 16;
    mesh.style.left = `${shiftedMesh.left - heroBox.left + (desiredLeft - placed.left)}px`;
    mesh.style.top = `${shiftedMesh.top - heroBox.top + (desiredTop - placed.top)}px`;
    mesh.style.transform = 'none';
  });
}
