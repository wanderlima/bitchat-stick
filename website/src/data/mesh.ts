export type Point = readonly [number, number];

export const deviceShell = { width: 356, height: 212 };
export const deviceSlot = { x: 0, y: 17, width: 356, height: 178 };

export const peerNames = ['west', 'relay', 'stick', 'north', 'east'] as const;
type PeerName = (typeof peerNames)[number];
type Placement = { x: number; y: number; scale: number };
type Layout = Record<PeerName, Placement>;

const desktop: Layout = {
  west: { x: 120, y: 440, scale: 1 },
  relay: { x: 714, y: 524, scale: 1 },
  stick: { x: 902, y: 229, scale: 1 },
  north: { x: 1334, y: 84, scale: 1 },
  east: { x: 1388, y: 514, scale: 1 },
};

const mobile: Layout = {
  west: { x: 22, y: 99, scale: 0.32 },
  relay: { x: 62, y: 269, scale: 0.42 },
  stick: { x: 89, y: 119, scale: 0.42 },
  north: { x: 262, y: 31, scale: 0.32 },
  east: { x: 267, y: 274, scale: 0.32 },
};

export function meshBounds(layout: Layout, name: PeerName) {
  const { x, y, scale } = layout[name];
  const shell = name === 'stick' ? deviceShell : { width: 92, height: 132 };
  return { x, y, width: shell.width * scale, height: shell.height * scale };
}

function makeScene(layout: Layout, size: Point, bends: Point[][]) {
  const centers = peerNames.map((name): Point => {
    const { x, y, width, height } = meshBounds(layout, name);
    return [x + width / 2, y + height / 2];
  });
  const routes = bends.map((waypoints, index) => ({
    points: [centers[index], ...waypoints, centers[index + 1]],
    duration: [1.8, 1, 0.9, 1.2][index],
  }));
  return { layout, size, routes };
}

export const desktopScene = makeScene(
  desktop,
  [1600, 760],
  [
    [
      [650, 506],
      [650, 590],
    ],
    [
      [876, 590],
      [876, 335],
    ],
    [
      [1300, 335],
      [1300, 150],
    ],
    [
      [1504, 150],
      [1504, 580],
    ],
  ],
);

const westY = mobile.west.y + (132 * mobile.west.scale) / 2;
const relayY = mobile.relay.y + (132 * mobile.relay.scale) / 2;
const stickY = mobile.stick.y + (deviceShell.height * mobile.stick.scale) / 2;
const northY = mobile.north.y + (132 * mobile.north.scale) / 2;
const eastY = mobile.east.y + (132 * mobile.east.scale) / 2;

export const mobileScene = makeScene(
  mobile,
  [327, 350],
  [
    [
      [17, westY],
      [17, relayY],
    ],
    [
      [80, relayY],
      [80, stickY],
    ],
    [
      [251, stickY],
      [251, northY],
    ],
    [
      [310, northY],
      [310, eastY],
    ],
  ],
);
