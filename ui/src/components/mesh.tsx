// dependencies
import { useControls, button } from 'leva'
import { useState } from 'react'
import useWebSocket from 'react-use-websocket'
import * as THREE from 'three'

// src
// import Polygon from '../types'

// to delete...?
// @ts-ignore: Unreachable code error
import RandomPolygon from 'randompolys'

// https://codesandbox.io/s/m7inl?file=/src/App.js:452-781
// https://www.youtube.com/watch?v=2kTQZVzkXgI
// https://stackoverflow.com/questions/67555786/custom-buffergeometry-in-react-three-fiber
// https://github.com/bicarbon8/QuadSphere/blob/in-javascript/src/components/shapes.tsx

// const context = createContext()

// const Circle = forwardRef(
// 	({ children, opacity = 1, radius = 0.05, segments = 16, color = '#ff1050', ...props }, ref) => (
// 		<mesh ref={ref} {...props}>
// 			<circleGeometry args={[radius, segments]} />
// 			<meshBasicMaterial transparent={opacity < 1} opacity={opacity} color={color} />
// 			{children}
// 		</mesh>
// 	),
// )

// function Nodes(positions) {
// 	const [nodes, set] = useState([])
// 	const group = useRef()
// 	return (
// 		<context.Provider value={set}>
// 			<group ref={group}>
// 				{positions.positions.map((position, index) => (
// 					<Node key={index} position={[position.x, position.y, 0]} />
// 				))}
// 			</group>
// 		</context.Provider>
// 	)
// }

// const Node = forwardRef(({ color = 'black', name, position = [0, 0, 0], ...props }, ref) => {
// 	// const set = useContext(context);
// 	const { size, camera } = useThree()
// 	const [pos, setPos] = useState(() => new THREE.Vector3(...position))
// 	const state = useMemo(() => ({ position: pos }), [pos])
// 	// Register this node on mount, unregister on unmount
// 	// useLayoutEffect(() => {
// 	//   set((nodes) => [...nodes, state]);
// 	//   return () => void set((nodes) => nodes.filter((n) => n !== state));
// 	// }, [state, pos]);
// 	// Drag n drop, hover
// 	const [hovered, setHovered] = useState(false)
// 	useEffect(() => void (document.body.style.cursor = hovered ? 'grab' : 'auto'), [hovered])
// 	const bind = useDrag(({ down, xy: [x, y] }) => {
// 		document.body.style.cursor = down ? 'grabbing' : 'grab'
// 		setPos(
// 			new THREE.Vector3((x / size.width) * 2 - 1, -(y / size.height) * 2 + 1, 0)
// 				.unproject(camera)
// 				.multiply({ x: 1, y: 1, z: 0 })
// 				.clone(),
// 		)
// 	})
// 	return (
// 		<Circle
// 			ref={ref}
// 			{...bind()}
// 			opacity={0.2}
// 			radius={0.5}
// 			color={color}
// 			position={pos}
// 			{...props}
// 		>
// 			<Circle
// 				radius={0.1}
// 				position={[0, 0, 0.1]}
// 				onPointerOver={() => setHovered(true)}
// 				onPointerOut={() => setHovered(false)}
// 				color={hovered ? '#ff1050' : color}
// 			></Circle>
// 		</Circle>
// 	)
// })

// const [polygon, setPolygon] = useState<Polygon>({})
// const [shape, setShape] = useState<THREE.Shape>(new THREE.Shape(polygon))

// const self = useRef<THREE.Mesh>(null)
// useEffect(() => {
// 	const { geometry } = self.current
// 	const { position } = geometry.attributes
// 	position.needsUpdate = true
// 	geometry.computeVertexNormals()
// }, [selected])

export default function Mesh(): JSX.Element {
	/*
	Using a three.js mesh, create our arbitrarily shaped membrane. 
	*/

	// Random polys isn't typed!
	// Once we have a type, we can remove the nulls, and subsequently any null checks cause
	// const [polygon, setPolygon] = useState<Polygon>({})
	// const [shape, setShape] = useState<THREE.Shape>(new THREE.Shape(polygon))
	const [_, setPolygon] = useState<any | null>(null)
	const [shape, setShape] = useState<THREE.Shape | null>(null)

	// function for regenerating a mesh
	const regenerateMesh = (): void => {
		const randomPoly = new RandomPolygon(
			11,
			{ topLeft: { x: -2, y: -2 }, bottomRight: { x: 2, y: 2 } },
			10,
		)
		setPolygon(randomPoly.polygon)
		setShape(new THREE.Shape(randomPoly.polygon))
	}
	// this will be wildly different when we use websockets and generate shapes in c++...
	// for now we initialise everything in this file

	// declare web socket
	const { sendMessage } = useWebSocket('endpoint', {
		onOpen: () => console.log('opened'),
		share: true,
	})
	// update shape
	useControls({
		'new shape': button(() => {
			regenerateMesh()
			// okay hi! Should we not do this with a request and response?
			sendMessage(JSON.stringify({ type: 'new_shape' }))
			// setButtonClicked((buttonClicked) => !buttonClicked)
		}),
	})
	// initialise shape with websocket
	shape === null && regenerateMesh()

	return (
		<mesh
		// ref={self}
		// onClick={(event) => setSelected(event.faceIndex || 0)}
		// onPointerMove={(e: ThreeEvent.PointerEvent) => {
		// 	{
		// 		/* let poly = [...polygon]
		//     poly[2] = event.point
		//     setPolygon(poly) */
		// 	}
		// }}
		>
			{
				// null check ! can be removed by solving type issue above
			}
			<shapeGeometry args={[shape!]} />
			<meshStandardMaterial color='orange' />
			{/* <Circle
                color='blue' 
                position={[0.5, 0.5, 1]} /> */}
			{/* <Nodes positions={polygon} /> */}
			{/* <planeGeometry attach="geometry" args={[1, 1]} /> */}
			{/* <bufferGeometry attach="geometry" {...geo} /> */}
		</mesh>
	)
}
