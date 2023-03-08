// dependencies
import { observer } from 'mobx-react'
import { useContext, useEffect, useState } from 'react'
import useWebSocket from 'react-use-websocket'
import { Vector2 } from 'three'

// src
import { callbacks, JuceIntegration, JuceMessage, ParametersContext } from './juceIntegration'
import { VALUE_TREE_STATE_CHANGE } from './messages/callbackEventTypes'
import { ParametersModelType } from './models/ParametersModel'
import { Polygon } from './polygon'
import { Panel } from './panel'
import '../scss/App.scss'
import { convertArrayToVector2, convertVector2ToArray } from './vectorUtils'

const Internal = observer(({ sendMessage }: { sendMessage: (msg: string) => void }) => {
	// update polygon coordinates
	const parameters: ParametersModelType | undefined = useContext(ParametersContext)
	const [polygon, setPolygon] = useState<Vector2[] | null>()

	useEffect(() => {
		if (parameters?.vertices.value) {
			console.log('new polygon')
			const flatVertices = [...parameters.vertices.value] // this is the array of vertices flattened
			setPolygon(convertArrayToVector2(flatVertices))
		}
	}, [parameters?.vertices.value])

	return (
		<>
			<Panel sendMessage={sendMessage} />
			{polygon && (
				<Polygon
					polygon={polygon}
					onChange={(V: Vector2[]) => {
						const flatVertices = convertVector2ToArray(V)
						console.log('sending vertices', flatVertices)
						sendMessage(JSON.stringify({ type: 'update_shape', value: flatVertices }))
					}}
				/>
			)}
		</>
	)
})

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

	return (
		<JuceIntegration>
			<Internal sendMessage={sendMessage} />
		</JuceIntegration>
	)
})

export default App
