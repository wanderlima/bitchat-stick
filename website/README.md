# Bitchat Stick website

Static, English (en-US) one-page website built with **Astro 7**, **Tailwind CSS 4** and **GSAP 3**. It describes Bitchat Stick as modular ESP32 firmware, with initial support for the M5StickC Plus2 and the optional Mini JoyC HAT. The Plus2 illustration and JoyC scene stay. Website commands do not build or flash the Stick.

## Local development

Use **Node.js 24 LTS** (or Node >=22.23.4) and npm. The repository's previously installed Node 22.22.3 is too old for this Astro version. `website/.node-version` records the recommended major version. Use your preferred Node version manager; there is no project-wide runtime override.

```sh
cd website
npm ci
npm run dev
```

Open <http://127.0.0.1:4321/bitchat-stick/>. The repository base path is intentional and is also used locally so asset/link errors are caught before deployment.

```sh
npm run check
npm run build
npm run preview
```

The static output is `website/dist/`. It contains no application server, account system or firmware flashing endpoint. GA4 and the supplied GTM container load on deployed hosts only; development and localhost previews do not collect visits. The terminal illustration links to the actual USB build guide; its progress bar does not represent a connected device.

## Browser checks

After installing dependencies:

```sh
npx playwright install chromium
npm run build
npm test
```

Tests run against the production preview on port 4322. They cover widths 320/375/768/1440, long and unbreakable text, native FAQ disclosure, keyboard focus, button navigation and long presses, both hero scenarios and infinite repeat, active route/device accents, offscreen pause, site/system reduced motion, smooth-scroll anchor focus, external links, JavaScript-disabled content, local assets, canonical/social metadata, sitemap, robots, machine-readable project facts, analytics initialization with a mocked network, and automated WCAG checks with axe. Automated checks do not replace human review or performance measurements on a physical phone.

## GitHub Pages

The target URL is <https://wanderlima.github.io/bitchat-stick/>. This URL is configured as a deployment target, not evidence that this branch has been published.

1. In repository **Settings → Pages → Build and deployment → Source**, select **GitHub Actions**.
2. Merge/push the website and workflow to `main`.
3. The `Website` workflow installs with the lockfile, audits dependencies, checks types, builds and runs browser tests. Only a successful build on `main` deploys `website/dist/` to Pages. Pull requests validate without deploying.
4. Check the workflow result and the resulting public URL. Manual runs are also available from Actions.

No GitHub settings or remote content were changed when creating this implementation. Firmware files remain in their existing locations.

For a custom domain or repository rename, update `site` and `base` in `astro.config.mjs`. Asset paths use Astro's `BASE_URL`; canonical, sitemap and robots derive from the same configuration.

## Structure

- `src/pages/`: page, sitemap and robots endpoints, prerendered at build time.
- `src/layouts/`: document metadata, local fonts and structured data.
- `src/components/`: actions, icons, headings, controls and responsive hero scenes.
- `src/data/`: approved English copy and the shared route geometry.
- `src/styles/`: Tailwind theme tokens and responsive layout rules.
- `src/scripts/`: client-only hero, controls and section motion.
- `public/icons/`: downloaded original Figma vectors, served locally. `routes-reference.svg` preserves the source geometry for reference; animation geometry is in `src/data/mesh.ts`.
- `public/screens/`: real firmware UI software renders with simulated messages, including the five hero states and nine physical-control preview states. These are not photographs, hardware captures, or delivery evidence.
- `tests/`: production browser acceptance tests.

All application source is inside `src/`; only configuration, tests, documentation and static assets sit outside it.

## Design and motion

Source: [Bitchat Stick — refinado in Figma](https://www.figma.com/design/z2zlfw1uBejLJ5fFfRGkeZ/Bitchat-Stick?node-id=56-567). Hero animation handoff: node `75:915`.

Tokens: black `#000000`, cream `#EBDCBA`, mint `#84FF84`, forest `#082D21`, panel `#101418`. Pixelify Sans 500 is reserved for short headings; Courier Prime 400/700 is used for readable body copy. Fonts are self-hosted through Fontsource; the deployed font licenses are included in `public/font-licenses.txt`. Icons remain decorative alongside visible link labels.

The hero uses the 240×135 landscape UI and nearest-neighbor image sampling. The Stick is turned on its side so that firmware text stays upright. Viewports below 640 px use the centered mobile scene. Wider viewports use the desktop scene, with the outer peers inside the 1600×760 frame. There is no horizontal document overflow.

### Hero sequence

One 11.4-second timeline contains two conceptual scenarios: the Stick sends one message, then receives one.

| Moment | Start | Duration |
| --- | ---: | ---: |
| Home | 0 | 800 ms |
| Writing | 0.8 s | 1,200 ms |
| Send selected | 2.0 s | 600 ms |
| Queued | 2.6 s | 400 ms |
| Stick → North | 3.0 s | 900 ms |
| North → East | 3.9 s | 1,200 ms |
| Hold, then clear the outbound route | 5.1 s | until 6.4 s |
| West → Relay, screen returns to Home | 6.4 s | 1,800 ms |
| Relay → Stick | 8.2 s | 1,000 ms |
| Received | 9.2 s | until the route clears at 11.2 s |
| Rest | 11.0 s | loop ends at 11.4 s |

Each route is a polyline between device centers. One envelope glides along that line while the active stroke grows behind it. A ripple starts at the sending device, and another starts at the destination when the envelope arrives. The second hop of each story does not ripple the intermediate peer again. Source and destination stay accented until that story's route clears. Base forest routes stay visible. Inactive peers return to cream. Screens swap as complete bitmaps. No typing is fabricated on top of the firmware images. Queued never claims Delivered or Read.

The 11.4-second timeline repeats indefinitely while visible. There is no play/pause button on the hero. The footer offers a labeled **Reduce motion** checkbox, also reachable through a keyboard-only Motion settings shortcut. It stops the loop and other movement and restores the static Received frame. The OS reduced-motion preference is always honored; when active, the site checkbox is checked and disabled with an explanation. Leaving the viewport or hiding the tab pauses the loop automatically. All content and FAQ remain usable without JavaScript; the interactive controls demo is disabled until its script loads.

Every section h2 uses the same capped SplitText character reveal (80ms per character group, at most 17 stagger slots). Section text, facts, cards, documentation links and the controls illustration enter in small batches with y=12px and opacity, 220ms per item and a 55ms stagger. Each entrance runs once. Content remains present in the static HTML, and focused interactive items are not faded out. FAQ layout opens immediately, with at most a 120ms pointer fade; keyboard actions are immediate. Transform and opacity are the only animated visual properties.

Desktop fine-pointer scrolling uses GSAP ScrollSmoother with a 650ms catch-up. The hardware walkthrough has its own pinned section; the rest of the page keeps ordinary vertical flow. Internal anchors preserve the fragment and move focus to their destination. Touch devices retain native scrolling, with smooth internal anchors. Site/system reduced motion disables smoothing and section/hero motion.

The physical-controls section uses the approved 3D wireframe with a simulated app screen; its SVG Device fallback remains available without WebGL. The walkthrough now has six stages: Explore, Navigate, Open, Receive, Reply and Send. The interactive flow can receive a simulated message from Test phone, choose a reply method, browse all 32 short messages, review the recipient/text, cancel or explicitly select Send, and show Queued. Cancel is selected initially. Keyboard is a read-only sample draft; this remains a bounded demonstration rather than a full emulator. No website action sends a real message or implies a delivery/read receipt.

### Technical honesty

The network shape and message journey are illustrative. The owner has confirmed operation with the iPhone app. Indirect relay, range, background behavior, battery endurance and individual receipt/reconnect scenarios still require detailed device testing. History is RAM-only. The page preserves those limitations in the hero, Project status and FAQ. Marketing copy must not convert a conceptual route or software render into interoperability evidence.

The page includes semantic headings, native FAQ disclosures, visible keyboard focus, a skip link, 44px minimum controls, canonical/Open Graph metadata, a sitemap, robots, a 1200×630 social preview and factual WebSite/WebPage/Person/SoftwareSourceCode/FAQ structured data. `llms.txt` derives project facts, evidence, FAQ answers and source links from the same content module as the visible page, avoiding conflicting machine-only copy. Structured data is not a promise of search-engine rich results.

On a GitHub Pages project site, `robots.txt` is served under the repository path. Crawlers normally consult the domain-root robots policy, so this file cannot override the account site’s root policy. The project sitemap remains available at the configured base URL.

## Implementation validation — 2026-11-11

- `npm run check`: 0 errors and 0 warnings.
- `npm run build`: static output generated successfully.
- `CI=1 npm test`: the initial 11 Chromium browser tests passed, including automated axe checks at 375px and 1440px.
- `npm audit`: 0 known vulnerabilities at validation time.
- Real-browser visual review: desktop hero/features and mobile hero/controls, with no console errors. A local desktop lab trace observed LCP 257 ms and CLS 0.00; these are localhost measurements, not deployed or physical-phone performance guarantees.

The firmware was not changed, built or uploaded as part of this website implementation. GitHub Pages has not been published from this branch.

## Analytics

The owner's Web stream **Home** (stream ID `16323837288`) uses measurement ID **G-VHFJLDXMVC**. `src/components/Analytics.astro` initializes one direct GA4 Google tag and loads the supplied **GTM-K58ZNCMN** container asynchronously. A hostname guard excludes localhost/127.0.0.1/IPv6 loopback, and development output does not include the loader.

The initialization is browser-tested on a mocked production origin, with both Google script requests intercepted. Tests therefore do not submit visits. After publishing, verify the real URL in Analytics Realtime or Tag Assistant. No account settings or GTM tags were published by this implementation. GA4 is configured directly; adding the same GA4 page-view tag to GTM later would require removing the direct configuration to avoid duplicate collection.

## SEO, GEO and accessibility review

- The owner's iPhone confirmation is reflected in visible status/FAQ text, JSON-LD, project facts and the linked evidence documents. Specific untested receipt/reconnect, relay, range and battery claims remain qualified.
- Metadata includes a canonical project URL, Open Graph/Twitter large-image cards, sitemap discovery and linked structured entities. The social image is a static capture of the actual website visual and software-rendered firmware UI, not a photograph.
- `llms.txt` is an additional plain-text summary, not a search/AI visibility guarantee. FAQ answers and primary-source documentation remain accessible as static HTML.
- External links use `_blank` with `noopener noreferrer` and an accessible new-tab notice. Internal links preserve same-page navigation.
- Keyboard focus, mobile touch targets, a skip link, native disclosures, live control feedback and a site-wide reduced-motion control supplement the OS preference. The hero is decorative to assistive technology; its conceptual/software-render disclosure stays readable.
- Final browser checks include 320px/375px mobile, 768px tablet, 1440px desktop and long/unbreakable content. Automated axe results do not establish complete WCAG conformance or physical-phone performance.

To regenerate the share image, run the production preview on port 4322, then `npm run render:social`, followed by `npm run build`. The export preserves the conceptual-network and simulated-UI disclosures.

## Refinement validation — 2026-11-11

- `npm run check`: 0 errors, 0 warnings.
- `npm run build`: static output generated successfully.
- `CI=1 npm test`: all 16 Chromium tests passed against a fresh production-preview server.
- Automated axe checks passed at 375px and 1440px; responsive/long-content checks passed at 320px, 375px, 768px and 1440px.
- The tests observed both complete hero journeys and a second loop on desktop/mobile, verified one active connection and its two accented devices, and checked that transformed segments stay inside their original route bounds.
- GA4/GTM were tested on a mocked production origin without sending visits. Local preview produces no Google script requests.
- Native-browser visual review confirmed the active hero route, desktop controls layout and mobile device placement. Physical-phone performance remains a manual review item.
- Dependency audit: 0 known vulnerabilities.

### Initial hero entrance

On first load, a 520ms stepped entrance reveals the eyebrow, title, description, actions and metadata, followed by the connected illustration and its five devices. The existing 11.4-second network loop starts after the entrance and never repeats it. Resizing or revisiting the hero within the same page does not replay the entrance. Reduced motion shows the final static layout immediately; focusing a hero action completes the entrance so a keyboard user never lands on a transparent link. The technical disclosure remains readable throughout.

The entrance revision passed `npm run check`, `npm run build` and all 17 browser tests, including once-only entrance, resize and live reduced-motion changes.

## Horizontal hardware walkthrough

The physical-controls section uses the approved StickC Plus2 + Mini JoyC reconstruction as a Three.js wireframe. Desktop viewports at least 901 px wide and 740 px high pin the section: scrolling changes six horizontal text chapters, the device angle and the demo selection. The active text follows the scroll gesture by at most 14 px; 140 ms after progress stops, it settles to its exact reading position over 220 ms. The snap applies to the text, not the page scroll, avoiding interference with focus and ScrollSmoother. The pinned journey spans about 3,400 px on desktop. Smaller viewports and reduced-motion preferences use explicit chapter buttons with ordinary page scrolling.

The joystick pad, A/B/C buttons, keyboard and scroll walkthrough share one preview state in `src/scripts/controls.ts`. The 3D display uses the same simulated firmware screenshots as the 2D fallback. Named cap, stem and skirt parts rotate around a shared joystick pivot; confirmation briefly depresses it. Manual app input takes precedence over focus-induced scrolling. Fine-pointer mouse tilt now reaches about 7 degrees in yaw and 5 degrees in pitch, with damped movement. The device floats vertically by ±1.6 mm with a 4.8-second period while visible. Reduced motion removes both floating and mouse parallax. This preview is not connected to a physical device.

Three.js is dynamically imported near the section. The model and screen textures load then, not with the hero. WebGL renders the float only while visible, stops outside the viewport / in hidden tabs, and releases its resources on page exit. With reduced motion it returns to on-demand rendering. Composer textures load as needed with an 18-texture working-set limit. Invisible depth geometry hides rear edges; it does not add shaded surfaces to the visible wireframe. If WebGL or model loading fails, the SVG preview and all app controls remain usable.

The committed web asset is `public/models/bitchat-stick-wire.glb`. Regenerate it after approving new source models:

```sh
npm run build:wire-model
```

The generator reads approved solids and contours from `../../m5stickc-plus2-models/`, omits internal battery/electronics geometry, preserves metre units and adds the animation pivot. Rebuilding the source CAD models is a separate step. The normal website build uses the committed GLB and does not run model generation.

`tests/hardware.spec.ts` checks actual rendered screen changes, joystick articulation, horizontal scroll synchronization, mouse/reduced-motion behavior, mobile keyboard navigation and the model-load fallback. The existing site tests continue to cover accessibility and the rest of the page.

### Regenerate the extended app screens

```sh
npm run build:demo-screens
```

This compiles `test/ui_preview.cpp` against the real UI and the VT323 bitmaps, using simulated data. It exports 240×135 PNGs for the hero, the hardware walkthrough, reply methods, short-message selections, Send/Cancel focus and Queued states. Existing PlatformIO library dependencies under `.pio/libdeps/m5stickc-plus2/` and a native C/C++ compiler are required only for regeneration. Normal website builds use the committed screenshots.

The hardware tests also cover the receive/reply/Cancel/Send flow in normal and reduced-motion modes, first/last preset wraparound, text settling after partial scrolling, vertical floating, and offscreen/reduced-motion pause behavior.
