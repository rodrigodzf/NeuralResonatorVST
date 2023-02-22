import { createGlobalState } from 'react-hooks-global-state';


const initialState = {
    endpoint: "ws://127.0.0.1:8000/ui",
    positions: {
        "0": {x: 0, y: 0, z: 0},
    }
};

export const { useGlobalState } = createGlobalState(initialState);