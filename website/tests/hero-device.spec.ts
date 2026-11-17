import { test, expect } from '@playwright/test';

test('hero device is the StickC model and follows the pointer', async ({ page }) => {
  const errors: string[] = [];
  page.on('pageerror', error => errors.push(error.message));
  await page.setViewportSize({ width: 1440, height: 1000 });
  await page.emulateMedia({ reducedMotion: 'no-preference' });
  await page.goto('');
  const hero = page.locator('.hero');
  const host = page.locator('[data-hero-device]');
  await expect(hero).toHaveAttribute('data-device', 'ready', { timeout: 20000 });
  await expect(host.locator('canvas')).toBeVisible();
  await expect.poll(async () => host.getAttribute('data-rendered-screen')).toMatch(/home|writing|confirm|queued|received/);
  await expect(page.locator('.mesh-desktop [data-peer="stick"] > .stick-label')).toBeVisible();
  await page.mouse.move(20, 420);
  await page.waitForTimeout(700);
  const left = Number(await host.getAttribute('data-view-yaw'));
  await page.mouse.move(1420, 420);
  await expect.poll(async () => Math.abs(Number(await host.getAttribute('data-view-yaw')) - left)).toBeGreaterThan(0.2);
  const difference = Math.abs(Number(await host.getAttribute('data-view-yaw')) - left);
  expect(difference).toBeLessThan(0.8);
  await page.emulateMedia({ reducedMotion: 'reduce' });
  await expect.poll(async () => Number(await host.getAttribute('data-view-yaw'))).toBeCloseTo(-0.22, 2);
  await page.mouse.move(1420, 200);
  await page.waitForTimeout(200);
  expect(Number(await host.getAttribute('data-view-yaw'))).toBeCloseTo(-0.22, 2);
  expect(errors).toEqual([]);
});

test('hero device falls back to the flat stick when the model is missing', async ({ page }) => {
  await page.route('**/models/stickc-plus2.glb', route => route.abort());
  await page.setViewportSize({ width: 1440, height: 1000 });
  await page.goto('');
  await expect(page.locator('.hero')).toHaveAttribute('data-device', 'fallback', { timeout: 20000 });
  await expect(page.locator('.mesh-desktop [data-peer="stick"] > path').first()).toBeVisible();
});
