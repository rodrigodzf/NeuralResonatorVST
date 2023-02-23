import { button, LevaPanel, useControls, useCreateStore } from 'leva'
import useWebSocket, { ReadyState } from 'react-use-websocket'
import { useGlobalState } from './State'

function Panel() {
	const [endpoint, setEndpoint] = useGlobalState('endpoint')

	const { sendMessage } = useWebSocket(endpoint, {
		onOpen: () => console.log('opened'),
		share: true,
	})

	useControls({
		density: {
			value: 0,
			min: 0,
			max: 100,
			step: 1,
			onChange: (value) => {
				sendMessage(value)
			},
		},
		stiffness: { value: 0, min: 0, max: 100, step: 1 },
	})

	return null
}

export default Panel
