// import { sendMessageToJuce } from "../juceCommunication";
// import { JuceVariant } from '../valueTree/InputStream'

// export const setParameter = <T extends JuceVariant>(id: string, value: T) => {
// 	const endpoint = 'ws://localhost:3000/ui'

// const { sendMessage } = useWebSocket(endpoint, { share: true }, false)
// sendMessage(
// 	JSON.stringify({
// 		eventType: 'Plugin::setParameter',
// 		data: {
// 			id,
// 			value,
// 		},
// 	}),
// )
// }

export const initPlugin = () => {
	//   sendMessageToJuce({ eventType: "Plugin::init" });
}
