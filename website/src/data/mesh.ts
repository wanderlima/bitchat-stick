export type Point = readonly [number, number];
export type Route = { points: readonly Point[]; duration: number; steps: number; destination: string };
export const desktopRoutes: Route[] = [
  { points: [[60,490],[160,490],[160,210],[550,210],[550,590],[760,590]], duration: 1.8, steps: 24, destination: 'relay' },
  { points: [[760,590],[900,590],[900,335],[1080,335]], duration: 1, steps: 20, destination: 'stick' },
  { points: [[1080,335],[1240,335],[1240,150],[1380,150]], duration: .9, steps: 18, destination: 'north' },
  { points: [[1380,150],[1460,150],[1460,580],[1490,580]], duration: 1.2, steps: 24, destination: 'east' },
];
export const mobileRoutes: Route[] = [
  { points: [[8,170],[38,170],[38,240],[85,240],[85,294]], duration: 1.8, steps: 24, destination: 'relay' },
  { points: [[85,294],[96,294],[96,170],[190,170]], duration: 1, steps: 20, destination: 'stick' },
  { points: [[190,170],[288,170],[288,52],[312,52]], duration: .9, steps: 18, destination: 'north' },
  { points: [[312,52],[326,52],[326,300],[327,300]], duration: 1.2, steps: 24, destination: 'east' },
];
export function length(points: readonly Point[]) {
  return points.slice(1).reduce((total, p, i) => total + Math.abs(p[0] - points[i][0]) + Math.abs(p[1] - points[i][1]), 0);
}
