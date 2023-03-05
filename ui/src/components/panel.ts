// dependencies
import { useEffect } from 'react'
import { useControls } from 'leva'
import useWebSocket from 'react-use-websocket'

type ParameterObject = {
	max: number
	min: number
	step: number
	value: number
}

export default function Panel(): null {
	const { sendMessage } = useWebSocket('endpoint', {
		onOpen: () => console.log('opened'),
		share: true,
	})

	const material: {
		density: ParameterObject
		stiffness: ParameterObject
		alpha: ParameterObject
		pratio: ParameterObject
		beat: ParameterObject
	} = useControls({
		density: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
		},
		stiffness: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
		},
		pratio: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
		},
		alpha: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
		},
		beta: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
		},
	})

	useEffect(() => {
		sendMessage(JSON.stringify({ type: 'new_material', material: material }))
	}, [material])

	return null
}
