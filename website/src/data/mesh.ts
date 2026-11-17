export type Point = readonly [number, number];
export type Route = { points: readonly Point[]; duration: number; steps: number; destination: string };
export const desktopRoutes: Route[] = [
  { points: [[58,490],[640,490],[640,590],[760,590]], duration: 1.8, steps: 24, destination: 'relay' },
  { points: [[760,590],[760,400],[1080,400]], duration: 1, steps: 20, destination: 'stick' },
  { points: [[1080,190],[1380,190]], duration: .9, steps: 18, destination: 'north' },
  { points: [[1380,150],[1380,580],[1490,580]], duration: 1.2, steps: 24, destination: 'east' },
];
export const mobileRoutes: Route[] = [
  { points: [[27,170],[48,170],[48,294],[85,294]], duration: 1.8, steps: 24, destination: 'relay' },
  { points: [[85,294],[85,200],[190,200]], duration: 1, steps: 20, destination: 'stick' },
  { points: [[190,48],[299,48]], duration: .9, steps: 18, destination: 'north' },
  { points: [[299,66],[299,277]], duration: 1.2, steps: 24, destination: 'east' },
];
export function length(points: readonly Point[]) {
  return points.slice(1).reduce((total, p, i) => total + Math.abs(p[0] - points[i][0]) + Math.abs(p[1] - points[i][1]), 0);
}
