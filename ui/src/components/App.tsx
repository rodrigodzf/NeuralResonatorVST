// dependencies
import { Canvas } from '@react-three/fiber'

// src
import Mesh from './mesh'
import Panel from './panel'
import '../scss/App.scss'

export default function App(): JSX.Element {
	return (
		<>
			<Panel />
			<Canvas
				orthographic={true}
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

// leaving this here so no one forgets how to do it
// import { Leva } from "leva";
/* 
<Leva
	titleBar={false}
	flat={true} 
	oneLineLabels={true}
/> 
*/
