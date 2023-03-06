// dependencies
import { button, useControls } from 'leva'
import { observer } from 'mobx-react'
// @ts-ignore
import RandomPolygon from 'randompolys'
import { useContext, useEffect, useRef, useState } from 'react'
import { Shape, Vector2 } from 'three'

// src
import { ParametersContext } from './juceIntegration'

export const Mesh = observer(({ sendMessage }: SendMessage) => {
	const parameters = useContext(ParametersContext)
	const mesh = useRef<THREE.Mesh>(null)
	const [polygon, setPolygon] = useState<Vector2[] | null>(null)

	useEffect(() => {
		const flatVertices = [...parameters?.vertices.value!] // this is the array of vertices flattened
		console.log('vertices changed', flatVertices)

		// convert the array of vertices to an array of Vector2
		// the array of vertices is a flat array of x,y,x,y,x,y
		const vertices: Vector2[] = []
		for (let i = 0; i < flatVertices.length; i += 2) {
			vertices.push({ x: flatVertices[i], y: flatVertices[i + 1] } as Vector2)
		}

		setPolygon(vertices)
	}, [parameters?.vertices.value])

	useControls({
		'new shape': button(() => {
			sendMessage(JSON.stringify({ type: 'new_shape' }))
		}),
	})

	return (
		<mesh ref={mesh}>
			<shapeGeometry args={[new Shape(polygon!)]} />
			{/* <Nodes positions={polygon} /> */}
			{/* <planeGeometry attach="geometry" args={[1, 1]} /> */}
			{/* <bufferGeometry attach="geometry" {...geo} /> */}
			<meshStandardMaterial color='orange' />
		</mesh>
	)
})
