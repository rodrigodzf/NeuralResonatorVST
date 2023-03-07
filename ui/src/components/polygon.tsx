// dependencies
import { useRef } from 'react'
import { Shape, Vector2 } from 'three'

export const Mesh = ({ polygon }: { polygon: Vector2[] }): JSX.Element => {
	/*
	Body of the polygon.
	*/
	const mesh = useRef<THREE.Mesh>(null)
	return (
		<mesh ref={mesh}>
			<shapeGeometry args={[new Shape(polygon)]} />
			{/* <Nodes positions={polygon} /> */}
			{/* <planeGeometry attach="geometry" args={[1, 1]} /> */}
			{/* <bufferGeometry attach="geometry" {...geo} /> */}
			<meshStandardMaterial color='orange' />
		</mesh>
	)
}

const Vertex: React.FC<{ point: Vector2 }> = ({ point }) => {
	/*
	A handle for a single vertex.
	*/
	console.log(`I am a point: ${point}`)
	return <></>
}

export const Vertices = ({ polygon }: { polygon: Vector2[] }): JSX.Element => {
	/*
	All vertices.
	*/
	return <>{polygon && polygon.map((v: Vector2) => <Vertex point={v} />)}</>
}
