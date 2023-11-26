// dependencies
import { observer } from 'mobx-react'
import { useContext, useEffect, useState } from 'react'
import useWebSocket from 'react-use-websocket'

// src
import { VALUE_TREE_STATE_CHANGE } from './juce-integration/messages/callbackEventTypes'
import { ParametersModelType } from './juce-integration/models/ParametersModel'
import { Drum } from './drum'
import { callbacks, JuceIntegration, JuceMessage, ParametersContext } from './juce-integration'
import { LevaPanel } from './leva-panel'
import '../scss/App.scss'

const Internal = observer(({ sendMessage }: { sendMessage: (msg: string) => void }) => {
	// update polygon coordinates
	const parameters: ParametersModelType | undefined = useContext(ParametersContext)
	const [polygon, setPolygon] = useState<Polygon | null>(null)
	const [strike, setStrike] = useState<Point | null>(null)

	useEffect(() => {
		if (parameters?.vertices.value) {
			const a = [...parameters.vertices.value]
			console.log(`new polygon ${a.toString()}`)
			const P: Polygon = []
			for (let i = 0; i < a.length; i += 2) {
				P.push({ x: a[i] as NonNullable<number>, y: a[i + 1] as NonNullable<number> })
			}
			setPolygon(P)
		}
	}, [parameters?.vertices.value])

	useEffect(() => {
		console.log(`new strike ${[parameters?.xpos.value, parameters?.ypos.value].toString()}`)
		parameters && setStrike({ x: parameters.xpos.value, y: parameters.ypos.value })
	}, [parameters?.xpos.value, parameters?.ypos.value])

	return (
		<>
			<LevaPanel sendMessage={sendMessage} />
			{polygon && strike && (
				<Drum
					polygon={polygon}
					strike={strike}
					onPolygonChange={(P: Polygon) => {
						const flatVertices: number[] = []
						P.map((p: Point) => flatVertices.push(p.x, p.y))
						sendMessage(JSON.stringify({ type: 'update_shape', value: flatVertices }))
					}}
					onStrikeChange={(p: Point) => {
						sendMessage(JSON.stringify({ type: 'new_parameter', id: 'xpos', value: p.x }))
						sendMessage(JSON.stringify({ type: 'new_parameter', id: 'ypos', value: p.y }))
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
		onOpen: () => {
			console.log('ws opened')
		},
		onClose: () => {
			console.log('ws closed')
		},
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
		shouldReconnect: (): boolean => false,
	})

	return (
		<JuceIntegration>
			<Internal sendMessage={sendMessage} />
		</JuceIntegration>
	)
})

export default App
