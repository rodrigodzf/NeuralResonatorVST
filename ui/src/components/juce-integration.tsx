// dependencies
import { createContext, useEffect, useRef, useState } from 'react'

// src
import { makeConnectedParametersModel, ParametersModelType } from './models/ParametersModel'

export const ParametersContext = createContext<ParametersModelType | undefined>(undefined)
export const callbacks = new Map<string, Function[]>()

export interface JuceMessage<T> {
	eventType: string
	data?: T
}

export const registerCallback = <T extends Function>(eventType: string, handler: T): void => {
	const existingCallbacks = callbacks.get(eventType)
	existingCallbacks ? existingCallbacks.push(handler) : callbacks.set(eventType, [handler])
}

export const JuceIntegration: React.FC<{
	children: React.ReactNode
}> = ({ children }) => {
	const [ready, setReady] = useState<boolean>(false)
	const parameters = useRef<ParametersModelType | undefined>()

	useEffect(() => {
		;(async () => {
			parameters.current = await makeConnectedParametersModel()
			setReady(true)
		})()
	}, [])

	return (
		<ParametersContext.Provider value={parameters.current}>
			{ready && children}
		</ParametersContext.Provider>
	)
}
