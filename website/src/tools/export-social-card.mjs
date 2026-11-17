import { chromium } from '@playwright/test';
import { fileURLToPath } from 'node:url';

// The share asset uses the actual website design and software-rendered UI.
const browser = await chromium.launch();
try {
  const page = await browser.newPage({ viewport: { width: 1200, height: 630 }, reducedMotion: 'reduce' });
  await page.goto('http://127.0.0.1:4322/bitchat-stick/');
  await page.evaluate(() => document.fonts.ready);
  await page.locator('.hero .eyebrow').evaluate(element => { element.textContent = 'BITCHAT STICK / OPEN SOURCE / ESP32'; });
  await page.addStyleTag({ content: `
    .site-header,.site-footer,.skip-link,main>.section,.hero-actions,.hero-meta,.hero-statement { display:none!important }
    .hero { width:1200px;height:630px;min-height:630px;margin:0 }
    .hero-content { width:1072px;margin:0 64px;padding:40px 0 }
    .hero-intro { width:530px;margin-top:40px }
    h1 { font-size:64px;line-height:68px }
    .hero-description { width:510px;font-size:20px;line-height:30px }
    .mesh-desktop { left:-220px;top:0;transform:scale(.85);transform-origin:top left }
    .hero-caption { position:absolute;left:0;top:420px;width:280px;margin:0;font-size:12px;line-height:18px }
  ` });
  await page.screenshot({ path: fileURLToPath(new URL('../../public/social-card.png', import.meta.url)) });
} finally {
  await browser.close();
}
