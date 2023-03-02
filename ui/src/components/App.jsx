import React, { useRef, useEffect, useLayoutEffect } from 'react'
import '../css/App.css'
// import { Leva } from "leva";
import { Canvas } from '@react-three/fiber'
import Panel from './Panel'
import { Mesh } from './Mesh'
import { useGlobalState, setGlobalState } from './State'

export default function App() {
	const [endpoint, setEndpoint] = useGlobalState('endpoint');
    const [density, setDensity] = useGlobalState('density');
	const ws = useRef(null);

	// the ctor is called at every update

    useEffect(() => {
        console.log('density app', density);
    }, [density])

	// componentDidMount
	useLayoutEffect(() => {
		console.log('ws mounted')
        ws.current = new WebSocket(endpoint);
        ws.current.onopen = () => console.log('ws opened');
        ws.current.onclose = () => console.log('ws closed');
        ws.current.onmessage = (e) => {
            // console.log('ws message', e.data);
            const data = JSON.parse(e.data);
            const [key, value] = Object.entries(data)[0];
            // get key from data and set value
            setGlobalState(key, value);
        }
		const wsCurrent = ws.current
		return () => {
			console.log('ws unmounted')
			if (wsCurrent.readyState !== WebSocket.CONNECTING) {
				wsCurrent.close()
			}
		}
	}, [])

	return (
		<>
			{/* <Leva
            titleBar={false}
            flat={true} 
            oneLineLabels={true}
         /> */}
			<Panel ws={ws} />
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
				<Mesh ws={ws} />
			</Canvas>
		</>
	)
}
