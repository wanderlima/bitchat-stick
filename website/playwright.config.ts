import { defineConfig } from '@playwright/test';
export default defineConfig({
  testDir: './tests',
  fullyParallel: true,
  // Software WebGL on the CI runner finishes the smooth-scroll demo just past 30s.
  timeout: process.env.CI ? 60_000 : 30_000,
  forbidOnly: Boolean(process.env.CI),
  retries: process.env.CI ? 1 : 0,
  workers: process.env.CI ? 2 : undefined,
  reporter: 'list',
  use: { baseURL: 'http://127.0.0.1:4322/bitchat-stick/', trace: 'retain-on-failure' },
  webServer: {
    command: 'npm run preview -- --port 4322 --ignore-lock',
    url: 'http://127.0.0.1:4322/bitchat-stick/',
    reuseExistingServer: !process.env.CI,
  },
});
