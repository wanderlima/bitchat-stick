export type Point = readonly [number, number];
export type Route = {
  points: readonly Point[];
  duration: number;
  steps: number;
  destination: string;
};

/** Drawn shell, and the 2:1 slot the landscape model actually fills. */
export const deviceShell = { width: 356, height: 212 };
export const deviceSlot = { x: 0, y: 17, width: 356, height: 178 };

/** Same dogleg network as the first hero. Endpoints stop on the landscape shell. */
export const desktopRoutes: Route[] = [
  {
    points: [
      [60, 490],
      [160, 490],
      [160, 210],
      [550, 210],
      [550, 590],
      [760, 590],
    ],
    duration: 1.8,
    steps: 24,
    destination: 'relay',
  },
  {
    points: [
      [760, 590],
      [900, 590],
      [900, 335],
      [902, 335],
    ],
    duration: 1,
    steps: 20,
    destination: 'stick',
  },
  {
    points: [
      [1258, 335],
      [1320, 335],
      [1320, 150],
      [1334, 150],
    ],
    duration: 0.9,
    steps: 18,
    destination: 'north',
  },
  {
    points: [
      [1426, 150],
      [1436, 150],
      [1436, 580],
      [1444, 580],
    ],
    duration: 1.2,
    steps: 24,
    destination: 'east',
  },
];

export const mobileRoutes: Route[] = [
  {
    points: [
      [8, 170],
      [38, 170],
      [38, 240],
      [85, 240],
      [85, 294],
    ],
    duration: 1.8,
    steps: 24,
    destination: 'relay',
  },
  {
    points: [
      [85, 294],
      [96, 294],
      [96, 170],
      [115, 170],
    ],
    duration: 1,
    steps: 20,
    destination: 'stick',
  },
  {
    points: [
      [275, 170],
      [288, 170],
      [288, 52],
      [312, 52],
    ],
    duration: 0.9,
    steps: 18,
    destination: 'north',
  },
  {
    points: [
      [312, 52],
      [326, 52],
      [326, 300],
      [327, 300],
    ],
    duration: 1.2,
    steps: 24,
    destination: 'east',
  },
];

export function length(points: readonly Point[]) {
  return points
    .slice(1)
    .reduce(
      (total, point, index) =>
        total + Math.abs(point[0] - points[index][0]) + Math.abs(point[1] - points[index][1]),
      0,
    );
}
