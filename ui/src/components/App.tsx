// dependencies
import { Canvas } from '@react-three/fiber'
import { observer } from 'mobx-react'
import { useLayoutEffect } from 'react'
import useWebSocket from 'react-use-websocket'

// src
import { callbacks, JuceMessage, JuceIntegration } from './juceIntegration'
import { VALUE_TREE_STATE_CHANGE } from './messages/callbackEventTypes'
import { Mesh } from './mesh'
import { Panel } from './panel'
import '../scss/App.scss'

// const ParamDebug = () => {
//   const parameters = useContext(ParametersContext)!
//   return (
//     console.log('density', parameters.density.value),
//     (<ParameterSlider parameter={parameters.density} />)
//   )
// }

// const ParameterSlider: React.FC<{ parameter: ParameterModel<number> }> = observer(
// 	({ parameter }) => {
// 		return parameter ? <h1>{parameter.value}</h1> : null
// 	},
// )

const App = observer(() => {
	// const ws = useRef<WebSocket | undefined>()

	const { sendMessage } = useWebSocket('endpoint', {
		onOpen: () => console.log('ws opened'),
		onClose: () => console.log('ws closed'),
		onMessage: (e: WebSocketEventMap['message']) => {
			console.log('ws message')
			const message: JuceMessage<any> = {
				eventType: VALUE_TREE_STATE_CHANGE,
				data: {
					treeId: 'PARAMETERS',
					changes: [e.data],
				},
			}

			try {
				const existingCallbacks = callbacks.get(message.eventType)

				if (existingCallbacks) {
					existingCallbacks.forEach((cb) => cb(message.data))
				} else {
					console.log(`No callbacks registered for event type "${message.eventType}"`, {
						message,
					})
				}
			} catch (e) {
				console.error('Error handling message from JUCE', { e, message })
				throw e
			}
		},
		shouldReconnect: (_: WebSocketEventMap['close']): boolean => false,
	})

	// componentDidMount
	useLayoutEffect(() => {
		console.log('ws mounted')
		return () => {
			console.log('ws unmounted')
		}
	}, [])

	return (
		<>
			<JuceIntegration>
				{/* <ParamDebug/> */}
				<Panel sendMessage={sendMessage} />
				<Canvas
					orthographic
					camera={{
						position: [0, 0, 10],
						zoom: 100,
						up: [0, 1, 0],
						far: 10000,
					}}
				>
					<ambientLight />
					<pointLight position={[10, 10, 10]} />
					<Mesh sendMessage={sendMessage} />
				</Canvas>
			</JuceIntegration>
		</>
	)
})

export default App
