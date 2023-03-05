import { button, LevaPanel, useControls, useCreateStore } from 'leva'
import { useEffect, useContext } from 'react'
import useWebSocket, { ReadyState } from 'react-use-websocket'
import { useGlobalState } from './State'
import { ParametersContext } from './JuceIntegration'
import { observer } from 'mobx-react'

const Panel = observer(({sendMessage}) => {
    const parameters = useContext(ParametersContext)!

	const [, set] = useControls(() => ({
		densityUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
            onChange: (value) => {
                sendMessage(JSON.stringify({ type: 'new_parameter', id: 'density', value: value }))
            }
		},
		stiffnessUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
            onChange: (value) => {
                sendMessage(JSON.stringify({ type: 'new_parameter', id: 'stiffness', value: value }))
            }
		},
		pratioUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
            onChange: (value) => {
                sendMessage(JSON.stringify({ type: 'new_parameter', id: 'pratio', value: value }))
            }
		},
		alphaUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
            onChange: (value) => {
                sendMessage(JSON.stringify({ type: 'new_parameter', id: 'alpha', value: value }))
            }
		},
		betaUi: {
			value: 0.5,
			min: 0,
			max: 1,
			step: 0.01,
            onChange: (value) => {
                sendMessage(JSON.stringify({ type: 'new_parameter', id: 'beta', value: value }))
            }
		},
	}))

	// this comes from the ws
	useEffect(() => {
		set({ densityUi: parameters.density.value })
	}, [parameters.density.value])

    useEffect(() => {
        set({ stiffnessUi: parameters.stiffness.value })
    }, [parameters.stiffness.value])
    
    useEffect(() => {
        set({ pratioUi: parameters.pratio.value })
    }, [parameters.pratio.value])

    useEffect(() => {
        set({ alphaUi: parameters.alpha.value })
    }, [parameters.alpha.value])

    useEffect(() => {
        set({ betaUi: parameters.beta.value })
    }, [parameters.beta.value])

    useEffect(() => {
		console.log('updated parameters')
	}, [parameters.vertices.values])

	return null
})

export default Panel
