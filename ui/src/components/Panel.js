import { button, LevaPanel, useControls, useCreateStore } from 'leva'
import { useEffect } from 'react'
import useWebSocket, { ReadyState } from 'react-use-websocket'
import { useGlobalState } from './State'

function Panel({ws}) {

    const [density, setDensity] = useGlobalState('density')
    const [stiffness, setStiffness] = useGlobalState('stiffness')
    const [pratio, setPratio] = useGlobalState('pratio')
    const [alpha, setAlpha] = useGlobalState('alpha')
    const [beta, setBeta] = useGlobalState('beta')

	const [{densityUi, stiffnessUi, pratioUi, alphaUi, betaUi}, set] = useControls(() => ({
		densityUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
		},
		stiffnessUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
		},
		pratioUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
		},
		alphaUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
		},
		betaUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
		},
	}))

    // this comes from the ws
    useEffect(() => {
        if (ws.current.readyState !== ReadyState.OPEN) return
        set({densityUi: density})
    }, [density])

    useEffect(() => {
        if (ws.current.readyState !== ReadyState.OPEN) return
        set({stiffnessUi: stiffness})
    }, [stiffness])

    useEffect(() => {
        if (ws.current.readyState !== ReadyState.OPEN) return
        set({pratioUi: pratio})
    }, [pratio])

    useEffect(() => {
        if (ws.current.readyState !== ReadyState.OPEN) return
        set({alphaUi: alpha})
    }, [alpha])

    useEffect(() => {
        if (ws.current.readyState !== ReadyState.OPEN) return
        set({betaUi: beta})
    }, [beta])


    // this goes to the ws
	useEffect(() => {
        // if connection is not open, do nothing
        if (ws.current.readyState !== ReadyState.OPEN) return
        const material = {
            density: densityUi,
            stiffness: stiffnessUi,
            pratio: pratioUi,
            alpha: alphaUi,
            beta: betaUi,
        }
		ws.current.send(JSON.stringify({ type: 'new_material', material: material }))
	}, [densityUi, stiffnessUi, pratioUi, alphaUi, betaUi])

	return null
}

export default Panel
