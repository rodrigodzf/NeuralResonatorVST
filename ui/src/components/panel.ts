// dependencies
import { button, useControls } from 'leva'
import { observer } from 'mobx-react'
import { useContext, useEffect } from 'react'
import { ParametersContext } from './juceIntegration'

type ParameterObject = {
	max: number
	min: number
	step: number
	value: number
	onChange: (msg: string) => void
}

export type Parameters = {
	alpha: ParameterObject
	beta: ParameterObject
	density: ParameterObject
	pratio: ParameterObject
	stiffness: ParameterObject
}

export const Panel = observer(({ sendMessage }: { sendMessage: (msg: string) => void }): null => {
	const parameters = useContext(ParametersContext)
	const [_, set] = useControls(() => ({
		density: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'density', value: value }))
			},
		},
		stiffness: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
			onChange: (value: number) => {
				sendMessage(
					JSON.stringify({ type: 'new_parameter', id: 'stiffness', value: value }),
				)
			},
		},
		pratio: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'pratio', value: value }))
			},
		},
		alpha: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'alpha', value: value }))
			},
		},
		beta: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'beta', value: value }))
			},
		},
		'new shape': button(() => {
			sendMessage(JSON.stringify({ type: 'new_shape' }))
		}),
	}))

	// this comes from the ws
	useEffect(() => {
		set({ density: parameters!.density.value })
	}, [parameters!.density.value])
	useEffect(() => {
		set({ stiffness: parameters!.stiffness.value })
	}, [parameters!.stiffness.value])
	useEffect(() => {
		set({ pratio: parameters!.pratio.value })
	}, [parameters!.pratio.value])
	useEffect(() => {
		set({ alpha: parameters!.alpha.value })
	}, [parameters!.alpha.value])
	useEffect(() => {
		set({ beta: parameters!.beta.value })
	}, [parameters!.beta.value])

	return null
})
