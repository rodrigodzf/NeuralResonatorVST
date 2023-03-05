import { button, useControls } from 'leva'
import { observer } from 'mobx-react'
import RandomPolygon from 'randompolys'
import React, { useContext, useEffect, useRef, useState } from 'react'
import { ParametersContext } from './JuceIntegration'

import { Shape, Vector2 } from 'three'
import { Vertex } from './valueTree/InputStream'

export const Mesh = observer(({ sendMessage }) => {
  const parameters = useContext(ParametersContext)!
  const mesh = useRef<THREE.Mesh>(null)
  const [polygon, setPolygon] = useState<Vector2[]>()

  const regenerateMesh = () => {
    const count = 10
    const bounds = {
      topLeft: { x: -1, y: -1 },
      bottomRight: { x: 1, y: 1 },
    }
    const epsilon = 10
    const randomPoly = new RandomPolygon(count, bounds, epsilon)
    sendMessage(JSON.stringify({ type: 'new_shape', shape: randomPoly.polygon }))
  }

  useEffect(() => {
    const flatVertices = [...parameters.vertices.value!] // this is the array of vertices flattened
    console.log('vertices changed', flatVertices)

    // convert the array of vertices to an array of Vector2
    // the array of vertices is a flat array of x,y,x,y,x,y
    const vertices: Vector2[] = []
    for (let i = 0; i < flatVertices.length; i += 2) {
        vertices.push({ x: flatVertices[i], y: flatVertices[i + 1] } as Vector2)
    }

    setPolygon(vertices)
  }, [parameters.vertices.value])

  useControls({
    'new shape': button(() => {
      regenerateMesh()
    }),
  })

  return (
    <mesh ref={mesh}>
      <shapeGeometry args={[new Shape(polygon)]} />
      {/* <Nodes positions={polygon} /> */}
      {/* <planeGeometry attach="geometry" args={[1, 1]} /> */}
      {/* <bufferGeometry attach="geometry" {...geo} /> */}
      <meshStandardMaterial color='orange' />
    </mesh>
  )
})
