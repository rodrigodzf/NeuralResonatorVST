// dependencies
import { useControls } from 'leva'
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

export const Panel = observer(({ sendMessage }: { sendMessage: (msg: string) => void}): null => {
	const parameters = useContext(ParametersContext)
	
	const [_, set]= useControls({
		alpha: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'pratio', value }))
			},
		},
		beta: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'pratio', value }))
			},
		},
		density: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'pratio', value }))
			},
		},
		pratio: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'pratio', value }))
			},
		},
		stiffness: {
			max: 1,
			min: 0,
			step: 0.01,
			value: 0.5,
			onChange: (value: number) => {
				sendMessage(JSON.stringify({ type: 'new_parameter', id: 'pratio', value }))
			},
		},
	})

	// this comes from the ws
	useEffect(() => {
		set({ densityUi: parameters!.density.value })
	}, [parameters!.density.value])
	useEffect(() => {
		set({ stiffnessUi: parameters!.stiffness.value })
	}, [parameters!.stiffness.value])
	useEffect(() => {
		set({ pratioUi: parameters!.pratio.value })
	}, [parameters!.pratio.value])
	useEffect(() => {
		set({ alphaUi: parameters!.alpha.value })
	}, [parameters!.alpha.value])
	useEffect(() => {
		set({ betaUi: parameters!.beta.value })
	}, [parameters!.beta.value])

	return null
})
