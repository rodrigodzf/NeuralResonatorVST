import { button, LevaPanel, useControls, useCreateStore } from 'leva'
import { useEffect } from 'react'
import useWebSocket, { ReadyState } from 'react-use-websocket'
import { useGlobalState } from './State'

function Panel() {
	const [endpoint, setEndpoint] = useGlobalState('endpoint')

	const { sendMessage } = useWebSocket(endpoint, {
		onOpen: () => console.log('opened'),
		share: true,
	})

	const material = useControls({
            density: {
                value: 0.5,
                min: 0,
                max: 1,
                step: 0.01,
            },
            stiffness: {
                value: 0.5,
                min: 0,
                max: 1,
                step: 0.01,
            },
            pratio: {
                value: 0.5,
                min: 0,
                max: 1,
                step: 0.01,
            },
            alpha: {
                value: 0.5,
                min: 0,
                max: 1,
                step: 0.01,

            },
            beta: {
                value: 0.5,
                min: 0,
                max: 1,
                step: 0.01,
            },
	})

    useEffect(() => {
        sendMessage(JSON.stringify({ type: 'new_material', material: material }))
    }, [material])

	return null
}

export default Panel
