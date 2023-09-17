// dependencies
import { Leva, button, useControls } from 'leva'
import { observer } from 'mobx-react'
import { useContext, useEffect } from 'react'
import { ParametersContext } from './juce-integration'

type ParameterObject = {
	max: number
	min: number
	step: number
	value: number
	onChange: (msg: string) => void
}

export type LevaParameters = {
	alpha: ParameterObject
	beta: ParameterObject
	density: ParameterObject
	pratio: ParameterObject
	stiffness: ParameterObject
}

export const LevaPanel = observer(
	({ sendMessage }: { sendMessage: (msg: string) => void }): JSX.Element => {
		const parameters = useContext(ParametersContext)
		const [_, set, get] = useControls(() => ({
			density: {
				value: parameters!.density.value,
				min: 0.0,
				max: 2.0,
				step: 0.01,
				onChange: (value: number) => {
					sendMessage(
						JSON.stringify({ type: 'new_parameter', id: 'density', value: value }),
					)
				},
			},
			stiffness: {
				value: parameters!.stiffness.value,
				min: 0.0,
				max: 1.0,
				step: 0.01,
				onChange: (value: number) => {
					sendMessage(
						JSON.stringify({ type: 'new_parameter', id: 'stiffness', value: value }),
					)
				},
			},
			'poisson ratio': {
				value: parameters!.pratio.value,
				min: -0.5,
				max: 1.5,
				step: 0.01,
				onChange: (value: number) => {
					sendMessage(
						JSON.stringify({ type: 'new_parameter', id: 'pratio', value: value }),
					)
				},
			},
			alpha: {
				value: parameters!.alpha.value,
				min: -0.5,
				max: 1.5,
				step: 0.01,
				onChange: (value: number) => {
					sendMessage(
						JSON.stringify({ type: 'new_parameter', id: 'alpha', value: value }),
					)
				},
			},
			beta: {
				value: parameters!.beta.value,
				min: -0.5,
				max: 1.5,
				step: 0.01,
				onChange: (value: number) => {
					sendMessage(JSON.stringify({ type: 'new_parameter', id: 'beta', value: value }))
				},
			},
			'vertex number': {
				value: 10,
				min: 3,
				max: 32,
				step: 1,
			},
			'new shape': button(() => {
				sendMessage(JSON.stringify({ type: 'new_shape', value: get('vertex number') }))
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
			set({ 'poisson ratio': parameters!.pratio.value })
		}, [parameters!.pratio.value])
		useEffect(() => {
			set({ alpha: parameters!.alpha.value })
		}, [parameters!.alpha.value])
		useEffect(() => {
			set({ beta: parameters!.beta.value })
		}, [parameters!.beta.value])
		useEffect(() => {
			set({ 'vertex number': parameters!.vertices.value.length / 2 })
		}, [parameters!.vertices.value])

		return (
			<>
				<Leva
					// oneLineLabels={true}
					titleBar={false}
					flat={true}
					hideCopyButton={true}
				/>
			</>
		)
	},
)
