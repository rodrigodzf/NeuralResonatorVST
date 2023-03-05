import { configure, runInAction } from "mobx";

export const USE_MOBX = true;

// Sensible default to warn about missing observers/observables
/*if (USE_MOBX)
  configure({
    observableRequiresReaction: true,
    reactionRequiresObservable: true,
  });*/

export const performUpdate = (fn: () => void) =>
  USE_MOBX ? runInAction(() => fn()) : fn();
