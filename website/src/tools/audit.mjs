import { execFileSync } from 'node:child_process';

// GHSA-fc5q-39xc-vjpf covers every published picomatch release through 4.0.7.
// `npm audit fix --force` only offers an Astro 5 downgrade, which still depends on picomatch.
// Remove this entry once a patched release exists and npm audit stops reporting it.
const allowed = new Set(['GHSA-fc5q-39xc-vjpf']);
const blockingSeverities = new Set(['high', 'critical']);

function advisoryId(via) {
  const match = String(via.url ?? '').match(/GHSA-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}/i);
  return match?.[0] ?? null;
}

let stdout;
try {
  stdout = execFileSync('npm', ['audit', '--json', '--audit-level=high'], {
    encoding: 'utf8',
    stdio: ['ignore', 'pipe', 'inherit'],
  });
} catch (error) {
  stdout = error.stdout ?? '';
  if (!stdout) throw error;
}

const report = JSON.parse(stdout);
const found = new Set();
const blocking = [];

for (const [name, vulnerability] of Object.entries(report.vulnerabilities ?? {})) {
  if (!blockingSeverities.has(vulnerability.severity)) continue;
  for (const via of vulnerability.via ?? []) {
    if (typeof via !== 'object' || via === null) continue;
    const id = advisoryId(via);
    if (!id) {
      blocking.push(`${name}: ${via.title ?? 'advisory without a GHSA id'}`);
      continue;
    }
    found.add(id);
    if (!allowed.has(id)) blocking.push(`${name}: ${via.title} (${id})`);
  }
}

const stale = [...allowed].filter((id) => !found.has(id));
if (stale.length > 0) {
  console.error(
    `Remove the audit allowlist entries that npm no longer reports: ${stale.join(', ')}`,
  );
  process.exit(1);
}

if (blocking.length > 0) {
  console.error(blocking.join('\n'));
  process.exit(1);
}

console.log('High and critical audit findings are limited to the documented picomatch allowlist.');
