import { test, expect } from '@playwright/test';
import AxeBuilder from '@axe-core/playwright';

for (const width of [320, 375, 768, 1440]) {
  test(`responsive layout and long content at ${width}px`, async ({ page }) => {
    await page.setViewportSize({ width, height: 900 });
    await page.emulateMedia({ reducedMotion: 'reduce' });
    const errors: string[] = [];
    page.on('pageerror', (error) => errors.push(error.message));
    page.on('response', (response) => {
      if (response.status() >= 400) errors.push(`${response.status()} ${response.url()}`);
    });
    await page.goto('');
    await expect(page.locator('.hero')).toHaveAttribute('data-motion', 'reduced');
    await expect(page.getByRole('heading', { level: 1 })).toHaveCount(1);
    expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(
      true,
    );
    await page.locator('details').evaluateAll((elements) =>
      elements.forEach((el) => {
        if (el instanceof HTMLDetailsElement) el.open = true;
      }),
    );
    await page
      .locator('summary')
      .first()
      .evaluate((el) => {
        el.textContent = 'A very long question about Bitchat compatibility '.repeat(8);
      });
    await page
      .locator('.hardware-facts dd')
      .first()
      .evaluate((el) => {
        el.textContent = 'ExtremelyLongUnbreakableHardwareIdentifier'.repeat(8);
      });
    expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(
      true,
    );
    for (const section of await page.locator('main > section').all()) {
      await section.scrollIntoViewIfNeeded();
      expect(await section.evaluate((el) => el.getBoundingClientRect().width <= innerWidth)).toBe(
        true,
      );
    }
    expect(errors).toEqual([]);
  });
}

for (const width of [375, 1440]) {
  test(`accessible page at ${width}px`, async ({ page }) => {
    await page.setViewportSize({ width, height: 900 });
    await page.emulateMedia({ reducedMotion: 'reduce' });
    await page.goto('');
    await expect(page.locator('.control-preview')).toBeVisible();
    const results = await new AxeBuilder({ page })
      .withTags(['wcag2a', 'wcag2aa', 'wcag21aa', 'wcag22aa'])
      .analyze();
    expect(results.violations).toEqual([]);
    const boxes = await page.locator('[data-controls]').evaluate((el) => {
      const copy = el.querySelector('.controls-copy')!.getBoundingClientRect();
      const device = el.querySelector('.control-preview')!.getBoundingClientRect();
      const buttons = el.querySelector('.physical-buttons')!.getBoundingClientRect();
      const details = el.querySelector('.controls-details')!.getBoundingClientRect();
      return {
        copyRight: copy.right,
        deviceLeft: device.left,
        deviceTop: device.top,
        deviceBottom: device.bottom,
        buttonsBottom: buttons.bottom,
        detailsTop: details.top,
      };
    });
    if (width > 900) expect(boxes.deviceLeft).toBeGreaterThan(boxes.copyRight);
    else {
      expect(boxes.deviceTop).toBeGreaterThan(boxes.buttonsBottom);
      expect(boxes.detailsTop).toBeGreaterThan(boxes.deviceBottom);
    }
  });
}

test('keyboard FAQ, controls, long press and skip link', async ({ page }) => {
  await page.goto('');
  await page.keyboard.press('Tab');
  await expect(page.getByRole('link', { name: 'Skip to content' })).toBeFocused();
  await page.keyboard.press('Enter');
  const second = page.locator('details').nth(1);
  await second.locator('summary').focus();
  await page.keyboard.press('Enter');
  await expect(second).toHaveAttribute('open');
  await page.keyboard.press('Enter');
  await expect(second).not.toHaveAttribute('open');
  const down = page.getByRole('button', { name: 'C — move down', exact: true });
  await down.focus();
  await expect(down).toBeInViewport();
  await down.click();
  await expect(page.getByRole('status')).toHaveText('Channels selected.');
  await page.getByRole('button', { name: 'A — confirm', exact: true }).click();
  await expect(page.getByRole('status')).toHaveText('Channels opened.');
  const up = page.getByRole('button', { name: 'B — move up; hold to go back', exact: true });
  await up.focus();
  await page.keyboard.down('Enter');
  await page.waitForTimeout(800);
  await page.keyboard.up('Enter');
  await expect(page.getByRole('status')).toHaveText('Back to Home.');
  await down.click();
  await up.click({ delay: 800 });
  await expect(page.getByRole('status')).toHaveText('Start screen.');
  await expect(page.locator('[data-preview-screen="start"]')).toHaveAttribute('opacity', '1');
  await page.getByRole('button', { name: 'A — confirm', exact: true }).click();
  await expect(page.getByRole('status')).toHaveText('Nearby selected.');
});

test('infinite hero has no player, pauses offscreen and respects site/system motion preference', async ({
  page,
}) => {
  await page.setViewportSize({ width: 1440, height: 1000 });
  await page.emulateMedia({ reducedMotion: 'no-preference' });
  await page.goto('');
  await expect(page.locator('.hero')).toHaveAttribute('data-motion', 'playing');
  await expect(page.getByRole('button', { name: /Pause animation|Resume animation/ })).toHaveCount(
    0,
  );
  await expect(page.locator('html')).toHaveAttribute('data-smooth-scroll', '');
  await page.getByRole('link', { name: 'Features', exact: true }).click();
  await expect(page).toHaveURL(/#features$/);
  await expect(page.locator('#features')).toBeFocused();
  await expect
    .poll(() =>
      page.locator('#features').evaluate((el) => Math.abs(el.getBoundingClientRect().top - 24)),
    )
    .toBeLessThan(3);
  const toggle = page.getByRole('checkbox', { name: 'Reduce motion', exact: true });
  await toggle.focus();
  await expect(toggle).toBeInViewport();
  await toggle.check();
  await expect(page.locator('.hero')).toHaveAttribute('data-motion', 'reduced');
  await expect(page.locator('html')).not.toHaveAttribute('data-smooth-scroll');
  expect(await page.locator('html').evaluate((el) => getComputedStyle(el).scrollBehavior)).toBe(
    'auto',
  );
  await page.setViewportSize({ width: 375, height: 812 });
  await expect(toggle).toBeChecked();
  await toggle.uncheck();
  await page.locator('#top').scrollIntoViewIfNeeded();
  await expect(page.locator('.hero')).toHaveAttribute('data-motion', 'playing');
  await page.locator('#faq').scrollIntoViewIfNeeded();
  await expect(page.locator('.hero')).toHaveAttribute('data-motion', 'paused');
  await page.emulateMedia({ reducedMotion: 'reduce' });
  await expect(page.locator('.hero')).toHaveAttribute('data-motion', 'reduced');
  await expect(toggle).toBeChecked();
  await expect(toggle).toBeDisabled();
  const received = page.locator('.mesh-mobile [data-screen="received"]');
  expect(await received.evaluate((el) => getComputedStyle(el).opacity)).toBe('1');
});

for (const heroWidth of [375, 1440])
  test(`both hero scenarios repeat with connected active routes at ${heroWidth}px`, async ({
    page,
  }) => {
    test.setTimeout(30000);
    await page.setViewportSize({ width: heroWidth, height: 1000 });
    await page.emulateMedia({ reducedMotion: 'no-preference' });
    await page.goto('');
    await expect(page.locator('.hero')).toHaveAttribute('data-motion', 'playing');
    const scene = heroWidth > 900 ? '.mesh-desktop' : '.mesh-mobile';
    const seen = new Set<string>();
    let sawActiveRoute = false;
    let maxAccents = 0;
    let maxRoutes = 0;
    const deadline = Date.now() + 17000;
    while (Date.now() < deadline) {
      const state = await page
        .locator(`${scene} [data-screen]`)
        .evaluateAll((elements) =>
          elements
            .filter((el) => getComputedStyle(el).opacity === '1')
            .map((el) => el.getAttribute('data-screen')!),
        );
      state.forEach((value) => seen.add(value));
      const { accents, routes } = await page.locator(scene).evaluate((svg) => ({
        accents: Array.from(svg.querySelectorAll('.device-accent')).filter(
          (el) => Number(getComputedStyle(el).opacity) > 0.5,
        ).length,
        routes: Array.from(svg.querySelectorAll('[data-route]')).filter(
          (el) => Number(getComputedStyle(el).opacity) > 0.5,
        ).length,
      }));
      maxAccents = Math.max(maxAccents, accents);
      maxRoutes = Math.max(maxRoutes, routes);
      if (routes) {
        expect(accents).toBeGreaterThan(0);
        sawActiveRoute = true;
      }
      const escaped = await page.locator(`${scene} [data-route]`).evaluateAll((edges) => {
        const failures: string[] = [];
        for (const edge of edges) {
          if (Number(getComputedStyle(edge).opacity) < 0.5) continue;
          const matrix = (edge as SVGGElement).getScreenCTM()!;
          for (const element of edge.children) {
            const rect = element as SVGRectElement;
            if (rect.tagName.toLowerCase() !== 'rect') continue;
            const x = Number(rect.getAttribute('x')),
              y = Number(rect.getAttribute('y'));
            const width = Number(rect.getAttribute('width')),
              height = Number(rect.getAttribute('height'));
            const start = new DOMPoint(x, y).matrixTransform(matrix),
              end = new DOMPoint(x + width, y + height).matrixTransform(matrix);
            const live = rect.getBoundingClientRect();
            if (
              live.left < start.x - 2 ||
              live.top < start.y - 2 ||
              live.right > end.x + 2 ||
              live.bottom > end.y + 2
            )
              failures.push(`${edge.getAttribute('data-route')}:${rect.getAttribute('data-leg')}`);
          }
        }
        return failures;
      });
      expect(escaped).toEqual([]);
      await page.waitForTimeout(150);
    }
    expect([...seen].sort()).toEqual(['confirm', 'home', 'queued', 'received', 'writing']);
    expect(sawActiveRoute).toBe(true);
    expect(maxAccents).toBeGreaterThan(1);
    expect(maxRoutes).toBeGreaterThan(1);
    expect(Number(await page.locator('.hero').getAttribute('data-loop'))).toBeGreaterThan(0);
  });

test('content, FAQ and real links work without JavaScript', async ({ browser }) => {
  const context = await browser.newContext({ javaScriptEnabled: false });
  const page = await context.newPage();
  await page.goto('http://127.0.0.1:4322/bitchat-stick/');
  await expect(page.getByRole('heading', { name: 'Bitchat Stick FAQ' })).toBeVisible();
  await expect(page.getByRole('link', { name: /^View on GitHub/ })).toHaveAttribute(
    'href',
    'https://github.com/wanderlima/bitchat-stick',
  );
  await page.locator('details').nth(4).locator('summary').click();
  await expect(page.locator('details').nth(4).locator('.faq-answer')).toBeVisible();
  await expect(page.getByRole('checkbox', { name: 'Reduce motion', exact: true })).toBeHidden();
  await expect(page.getByRole('button', { name: 'A — confirm', exact: true })).toBeDisabled();
  await context.close();
});

test('canonical, sitemap and robots use the repository base path', async ({ page, request }) => {
  await page.goto('');
  await expect(page.locator('link[rel="canonical"]')).toHaveAttribute(
    'href',
    'https://wanderlima.github.io/bitchat-stick/',
  );
  const schema = JSON.parse(
    (await page.locator('script[type="application/ld+json"]').textContent()) ?? '{}',
  );
  expect(
    schema['@graph'].find((item: { '@type': string }) => item['@type'] === 'FAQPage').mainEntity,
  ).toHaveLength(8);
  const sitemap = await request.get('sitemap.xml');
  expect(sitemap.ok()).toBe(true);
  expect(await sitemap.text()).toContain('https://wanderlima.github.io/bitchat-stick/');
  const robots = await request.get('robots.txt');
  expect(await robots.text()).toContain('https://wanderlima.github.io/bitchat-stick/sitemap.xml');
});

test('external destinations open safely in a new tab; internal navigation stays local', async ({
  page,
}) => {
  await page.goto('');
  const links = await page
    .locator('a[href^="https://"]')
    .evaluateAll((elements) =>
      elements.map((el) => ({ target: el.getAttribute('target'), rel: el.getAttribute('rel') })),
    );
  expect(links.length).toBeGreaterThan(10);
  for (const link of links) {
    expect(link.target).toBe('_blank');
    expect(link.rel).toContain('noopener');
    expect(link.rel).toContain('noreferrer');
  }
  await expect(page.getByRole('link', { name: 'Features', exact: true })).not.toHaveAttribute(
    'target',
    '_blank',
  );
});

test('all section titles and content animate on entrance, with static reduced-motion fallback', async ({
  page,
}) => {
  await page.emulateMedia({ reducedMotion: 'no-preference' });
  await page.goto('');
  const headings = page.locator('section.section h2');
  expect(await headings.count()).toBe(8);
  await expect.poll(() => page.locator('section.section h2[aria-label]').count()).toBe(8);
  for (const section of await page.locator('section.section').all()) {
    await section.scrollIntoViewIfNeeded();
    await page.waitForTimeout(500);
    const heading = section.locator('h2');
    await expect(heading).toBeVisible();
    expect(await heading.locator('[aria-hidden]').count()).toBeGreaterThan(0);
  }
  await page.emulateMedia({ reducedMotion: 'reduce' });
  await expect.poll(() => page.locator('section.section h2[aria-label]').count()).toBe(0);
  for (const item of await page.locator('[data-enter-item]').all())
    expect(await item.evaluate((el) => getComputedStyle(el).opacity)).toBe('1');
});

test('social metadata and machine-readable facts match visible iPhone evidence', async ({
  page,
  request,
}) => {
  await page.goto('');
  const image = await page.locator('meta[property="og:image"]').getAttribute('content');
  expect(image).toBe('https://wanderlima.github.io/bitchat-stick/social-card.png');
  const response = await request.get('social-card.png');
  expect(response.ok()).toBe(true);
  expect(response.headers()['content-type']).toContain('image/png');
  const facts = await request.get('llms.txt');
  expect(facts.ok()).toBe(true);
  expect(await facts.text()).toContain('works with the iPhone app');
  expect(await facts.text()).toContain('RAM-only');
  expect(await page.locator('#status').textContent()).toContain('works with Bitchat on iPhone');
});

test('GA4 and GTM initialize once on the production host without sending real analytics during tests', async ({
  page,
  request,
}) => {
  const loads: string[] = [];
  await page.route('https://www.googletagmanager.com/**', async (route) => {
    loads.push(route.request().url());
    await route.fulfill({
      contentType: 'application/javascript',
      body: '/* analytics network mocked */',
    });
  });
  await page.route('https://wanderlima.github.io/**', async (route) => {
    const url = new URL(route.request().url());
    const response = await request.get(`http://127.0.0.1:4322${url.pathname}`);
    await route.fulfill({ response });
  });
  await page.goto('https://wanderlima.github.io/bitchat-stick/');
  await expect.poll(() => loads.length).toBe(2);
  expect(loads).toContain('https://www.googletagmanager.com/gtag/js?id=G-VHFJLDXMVC');
  expect(loads).toContain('https://www.googletagmanager.com/gtm.js?id=GTM-K58ZNCMN');
  const events = await page.evaluate(() =>
    (window as unknown as { dataLayer: IArguments[] }).dataLayer.map((event) => Array.from(event)),
  );
  expect(
    events.filter((event) => event[0] === 'config' && event[1] === 'G-VHFJLDXMVC'),
  ).toHaveLength(1);
  await page.goto('http://127.0.0.1:4322/bitchat-stick/');
  expect(await page.locator('script[src*="googletagmanager.com"]').count()).toBe(0);
});

test('hero entrance runs once before the loop and stays static with reduced motion', async ({
  page,
}) => {
  await page.setViewportSize({ width: 1440, height: 1000 });
  await page.emulateMedia({ reducedMotion: 'no-preference' });
  await page.goto('');
  await expect(page.locator('.hero')).toHaveAttribute('data-entrance', 'complete');
  await expect(page.locator('.hero')).toHaveAttribute('data-motion', 'playing');
  expect(await page.locator('h1').evaluate((el) => getComputedStyle(el).opacity)).toBe('1');
  await page.setViewportSize({ width: 375, height: 812 });
  await expect(page.locator('.hero')).toHaveAttribute('data-entrance', 'complete');
  expect(await page.locator('.hero-actions').evaluate((el) => getComputedStyle(el).opacity)).toBe(
    '1',
  );
  await page.emulateMedia({ reducedMotion: 'reduce' });
  await expect(page.locator('.hero')).toHaveAttribute('data-entrance', 'static');
  expect(await page.locator('h1').evaluate((el) => getComputedStyle(el).opacity)).toBe('1');
  await page.emulateMedia({ reducedMotion: 'no-preference' });
  await expect(page.locator('.hero')).toHaveAttribute('data-entrance', 'complete');
});
