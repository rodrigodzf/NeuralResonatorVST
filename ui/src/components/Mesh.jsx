import React, {
	createContext,
	useRef,
	useState,
	useEffect,
	useMemo,
	forwardRef,
	useContext,
	useLayoutEffect,
} from 'react'
import { useFrame, useThree } from '@react-three/fiber'
import { Line, Text } from '@react-three/drei'
import { useControls, button } from 'leva'
import { useDrag } from '@use-gesture/react'
import RandomPolygon from 'randompolys'
import { useGlobalState } from './State'
import useWebSocket, { ReadyState } from 'react-use-websocket'

import * as THREE from 'three'

// https://codesandbox.io/s/m7inl?file=/src/App.js:452-781
// https://www.youtube.com/watch?v=2kTQZVzkXgI
// https://stackoverflow.com/questions/67555786/custom-buffergeometry-in-react-three-fiber
// https://github.com/bicarbon8/QuadSphere/blob/in-javascript/src/components/shapes.tsx

const context = createContext()
const Circle = forwardRef(
	({ children, opacity = 1, radius = 0.05, segments = 16, color = '#ff1050', ...props }, ref) => (
		<mesh ref={ref} {...props}>
			<circleGeometry args={[radius, segments]} />
			<meshBasicMaterial transparent={opacity < 1} opacity={opacity} color={color} />
			{children}
		</mesh>
	),
)

function Nodes(positions) {
	const [nodes, set] = useState([])
	const group = useRef()
	return (
		<context.Provider value={set}>
			<group ref={group}>
				{positions.positions.map((position, index) => (
					<Node key={index} position={[position.x, position.y, 0]} />
				))}
			</group>
		</context.Provider>
	)
}

const Node = forwardRef(({ color = 'black', name, position = [0, 0, 0], ...props }, ref) => {
	// const set = useContext(context);
	const { size, camera } = useThree()
	const [pos, setPos] = useState(() => new THREE.Vector3(...position))
	const state = useMemo(() => ({ position: pos }), [pos])
	// Register this node on mount, unregister on unmount
	// useLayoutEffect(() => {
	//   set((nodes) => [...nodes, state]);
	//   return () => void set((nodes) => nodes.filter((n) => n !== state));
	// }, [state, pos]);
	// Drag n drop, hover
	const [hovered, setHovered] = useState(false)
	useEffect(() => void (document.body.style.cursor = hovered ? 'grab' : 'auto'), [hovered])
	const bind = useDrag(({ down, xy: [x, y] }) => {
		document.body.style.cursor = down ? 'grabbing' : 'grab'
		setPos(
			new THREE.Vector3((x / size.width) * 2 - 1, -(y / size.height) * 2 + 1, 0)
				.unproject(camera)
				.multiply({ x: 1, y: 1, z: 0 })
				.clone(),
		)
	})
	return (
		<Circle
			ref={ref}
			{...bind()}
			opacity={0.2}
			radius={0.5}
			color={color}
			position={pos}
			{...props}
		>
			<Circle
				radius={0.1}
				position={[0, 0, 0.1]}
				onPointerOver={() => setHovered(true)}
				onPointerOut={() => setHovered(false)}
				color={hovered ? '#ff1050' : color}
			></Circle>
		</Circle>
	)
})

export const Mesh = (props) => {
	const [endpoint, setEndpoint] = useGlobalState('endpoint')
	const [hovered, setHover] = useState()
	const [active, setActive] = useState(false)
	const [selected, setSelected] = useState()
	const [buttonClicked, setButtonClicked] = useState(false)
	const [polygon, setPolygon] = useState(() => {
		const count = 11
		const bounds = {
			topLeft: { x: -2, y: -2 },
			bottomRight: { x: 2, y: 2 },
		}
		const epsilon = 10
		const randomPoly = new RandomPolygon(count, bounds, epsilon)
		return randomPoly.polygon
	})
	const [shape, setShape] = useState(() => {
		//     new THREE.Path(polygon)
	})

	const mesh = useRef()

	const updateBufferGeometry = () => {
		const { geometry } = mesh.current
		const { position } = geometry.attributes
		position.needsUpdate = true
		geometry.computeVertexNormals()
		console.log('geometry ' + geometry)
	}

	useEffect(() => {
		updateBufferGeometry()
	}, [selected])

	const regenerateMesh = () => {
		const count = 11
		const bounds = {
			topLeft: { x: -2, y: -2 },
			bottomRight: { x: 2, y: 2 },
		}
		const epsilon = 10
		const randomPoly = new RandomPolygon(count, bounds, epsilon)
		setPolygon(randomPoly.polygon)
	}

	useEffect(() => {
		setShape(new THREE.Shape(polygon))
	}, [polygon])

	const { sendMessage } = useWebSocket(endpoint, {
		onOpen: () => console.log('opened'),
		share: true,
	})

	useControls({
		'new shape': button(() => {
			setButtonClicked((buttonClicked) => !buttonClicked)
			regenerateMesh()
			sendMessage(JSON.stringify({ type: 'new_shape' }))
		}),
	})
	return (
		<mesh
			{...props}
			ref={mesh}
			onClick={(event) => {
				setSelected(event.faceIndex)
				{
					/* console.log(mesh.current.geometry) */
				}
			}}
			onPointerMove={(event) => {
				{
					/* let poly = [...polygon]
                poly[2] = event.point
                setPolygon(poly) */
				}
			}}
		>
			<shapeGeometry args={[shape]} />
			{/* <Circle
                color='blue' 
                position={[0.5, 0.5, 1]} /> */}
			<Nodes positions={polygon} />
			{/* <planeGeometry attach="geometry" args={[1, 1]} /> */}
			{/* <bufferGeometry attach="geometry" {...geo} /> */}
			<meshStandardMaterial color='orange' />
		</mesh>
	)
}
