// dependencies
import { Canvas } from '@react-three/fiber'
import { observer } from 'mobx-react'
import { useContext, useEffect, useLayoutEffect, useState } from 'react'
import useWebSocket from 'react-use-websocket'
import { Vector2 } from 'three'

// src
import { callbacks, JuceMessage, JuceIntegration, ParametersContext } from './juceIntegration'
import { VALUE_TREE_STATE_CHANGE } from './messages/callbackEventTypes'
import { ParametersModelType } from './models/ParametersModel'
import { Mesh, Vertices } from './polygon'
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
	// web socket communication
	const endpoint = 'ws://localhost:8000/ui'
	const { sendMessage } = useWebSocket(endpoint, {
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
				existingCallbacks
					? existingCallbacks.forEach((cb) => cb(message.data))
					: console.log(`No callbacks registered for event type "${message.eventType}"`, {
							message,
					  })
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

	// update polygon coordinates
	const parameters: ParametersModelType | undefined = useContext(ParametersContext)
	const [polygon, setPolygon] = useState<Vector2[] | null>(null)
	useEffect(() => {
		if (parameters?.vertices) {
			const flatVertices = [...parameters.vertices.value] // this is the array of vertices flattened
			console.log('vertices changed', flatVertices)
			// convert the array of vertices to an array of Vector2
			// the array of vertices is a flat array of x,y,x,y,x,y
			const vertices: Vector2[] = []
			for (let i = 0; i < flatVertices.length; i += 2) {
				vertices.push(new Vector2(flatVertices[i], flatVertices[i + 1]))
			}
			setPolygon(vertices)
		}
	}, [parameters?.vertices])

	console.log(polygon)

	return (
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
				{polygon && (
					<>
						<Mesh polygon={polygon} />
						<Vertices polygon={polygon} />
					</>
				)}
			</Canvas>
		</JuceIntegration>
	)
})

export default App
