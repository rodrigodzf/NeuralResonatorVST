import { createGlobalState } from 'react-hooks-global-state'

const initialState = {
	endpoint: 'ws://127.0.0.1:8000/ui',
	positions: {
		0: { x: 0, y: 0, z: 0 },
	},
    density: 0.5,
    stiffness: 0.5,
    pratio: 0.5,
    alpha: 0.5,
    beta: 0.5,
}

export const { useGlobalState, setGlobalState } = createGlobalState(initialState)
