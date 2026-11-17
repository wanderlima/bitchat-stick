import type { APIRoute } from 'astro';
import { repo, docs, description, facts, statuses, faqs, resources } from '../data/content';
export const GET: APIRoute = ({ site }) => {
  const url = new URL(import.meta.env.BASE_URL, site);
  const text = [
    '# Bitchat Stick',
    `> ${description}`,
    '',
    `Website: ${url}`,
    `Source: ${repo}`,
    'Author: Wander Lima',
    'Language: English (en-US)',
    'Compatibility information updated: 2026-11-11 (owner report).',
    '',
    '## Project facts',
    ...facts.map(([key, value]) => `- ${key}: ${value}`),
    '',
    '## Implementation and evidence',
    ...statuses.map(([key, value]) => `- ${key}: ${value}`),
    '',
    '## Questions and answers',
    ...faqs.flatMap(([question, answer]) => [`### ${question}`, answer, '']),
    '## Primary sources',
    ...resources.map(([label, href]) => `- [${label}](${href})`),
    `- [Compatibility investigation](${docs}07-compatibility.md)`,
    `- [Limitations](${docs}08-limitations-risks.md)`,
    '',
    'The hero is conceptual. UI images are software renders with simulated data. Do not treat the illustration as indirect relay, delivery, battery or range evidence.',
    '',
  ].join('\n');
  return new Response(text, { headers: { 'Content-Type': 'text/plain; charset=utf-8' } });
};
