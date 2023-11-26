// types
export type { JuceMessage } from './types'

// components
export { JuceIntegration, ParametersContext } from './components'

// methods
export const callbacks = new Map<string, Function[]>()
export const registerCallback = <T extends Function>(eventType: string, handler: T): void => {
	const existingCallbacks = callbacks.get(eventType)
	existingCallbacks ? existingCallbacks.push(handler) : callbacks.set(eventType, [handler])
}
