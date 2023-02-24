import React from 'react'
import '../css/App.css'
// import { Leva } from "leva";
import { Canvas } from '@react-three/fiber'
import Panel from './Panel'
import { Mesh } from './Mesh'

export default function App() {
	return (
		<>
			{/* <Leva
            titleBar={false}
            flat={true} 
            oneLineLabels={true}
         /> */}
			<Panel />
			<Canvas
				orthographic
				camera={{
					position: [0, 0, 10],
					zoom: 50,
					up: [0, 1, 0],
					far: 10000,
				}}
			>
				<ambientLight />
				<pointLight position={[10, 10, 10]} />
				<Mesh />
			</Canvas>
		</>
	)
}
