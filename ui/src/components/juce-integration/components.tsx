// dependencies
import { createContext, useEffect, useRef, useState } from 'react'

// src
import { makeConnectedParametersModel, ParametersModelType } from './models/ParametersModel'

export const ParametersContext = createContext<ParametersModelType | undefined>(undefined)

export const JuceIntegration: React.FC<{
	children: React.ReactNode
}> = ({ children }) => {
	const [ready, setReady] = useState<boolean>(false)
	const parameters = useRef<ParametersModelType | undefined>()

	useEffect(() => {
		void (async () => {
			parameters.current = await makeConnectedParametersModel()
			setReady(true)
		})()
	}, [])

	return <ParametersContext.Provider value={parameters.current}>{ready && children}</ParametersContext.Provider>
}
