// dependencies
import { Canvas } from '@react-three/fiber'
import { useEffect, useRef, useState } from 'react'
import { Environment, MeshDistortMaterial, GradientTexture, PerspectiveCamera, Backdrop, Sky, OrthographicCamera, ContactShadows, OrbitControls, Extrude, GizmoHelper, Plane, SpotLight, MeshTransmissionMaterial } from '@react-three/drei'
import { DoubleSide, Shape, Vector2 } from 'three'

const Vertex: React.FC<{ point: Vector2; onDrag: (v: Vector2, callback: boolean) => any }> = ({
    point,
    onDrag,
}) => {
    /*
    A handle for a single vertex.
    */

    // where am i
    const [position, updatePosition] = useState<{ x_window: number; y_window: number }>(
        relativePosition(point),
    )
    // update position from prop
    useEffect(() => updatePosition(relativePosition(point)), [point])
    // calculate position from Vector2
    function relativePosition(point: Vector2): { x_window: number; y_window: number } {
        return {
            x_window: 100 * point.x + window.innerWidth / 2,
            y_window: 100 * -1 * point.y + window.innerHeight / 2,
        }
    }
    // handle movement of points
    const [mouseDown, setMouseDown] = useState<boolean>(false)
    useEffect(() => {
        // change position if mouse is down
        const changePosition = (e: MouseEvent) => {
            if (mouseDown) {
                updatePosition({ x_window: e.clientX, y_window: e.clientY })
                onDrag(
                    new Vector2(
                        (e.clientX - window.innerWidth / 2) / 100,
                        ((e.clientY - window.innerHeight / 2) * -1) / 100,
                    ),
                    false,
                )
            }
        }
        // release mouse is mouse down
        const releasePoint = (e: MouseEvent) => {
            if (mouseDown) {
                onDrag(
                    new Vector2(
                        (e.clientX - window.innerWidth / 2) / 100,
                        ((e.clientY - window.innerHeight / 2) * -1) / 100,
                    ),
                    true,
                )
            }
            setMouseDown(false)
        }
        window.addEventListener('mousemove', changePosition)
        window.addEventListener('mouseup', releasePoint)
        return () => {
            window.removeEventListener('mousemove', changePosition)
            window.removeEventListener('mouseup', releasePoint)
        }
    }, [mouseDown])

    return (
        <div
            className='vertex'
            style={{
                top: `${position.y_window - 5}px`,
                left: `${position.x_window - 5}px`,
            }}
            onMouseDown={() => setMouseDown(true)}
        />
    )
}

export const Polygon: React.FC<{ polygon: Vector2[]; onChange: (V: Vector2[]) => any }> = ({
    polygon,
    onChange,
}): JSX.Element => {
    /*
    Body of the polygon.
    */
    const ref = useRef()
    // where am i
    const [_polygon, updatePolygon] = useState<Vector2[]>(polygon)
    // update Polygon from prop
    useEffect(() => {
        updatePolygon(polygon)
    }, [polygon])
    const mesh = useRef<THREE.Mesh>(null)
    return (
        <>
            <Canvas>

                <Sky />
                <GizmoHelper alignment="bottom-right" margin={[80, 80]}/>

                {/* <Environment background preset='night'/> */}
                {/* <Backdrop receiveShadow floor={0.25} segments={20} > */}
                {/* <ambientLight /> */}
                {/* <Environment preset="city" /> */}
                <pointLight position={[0, 0, 1]}  intensity={0.8} />
                {/* <ambientLight intensity={0.8} /> */}
                {_polygon && (
                    <mesh ref={mesh}>
                        
                        {/* <shapeGeometry args={[new Shape(_polygon)]} /> */}
                        <Extrude args={[new Shape(_polygon), { depth: 0.01, bevelEnabled: false }]}  >
                        {/* <meshStandardMaterial color="#e63946" /> */}
                        <MeshDistortMaterial ref={ref} speed={2} shadowSide={DoubleSide}>
                            <GradientTexture stops={[0, 1]} colors={['#0066cc', '#ffcc00']} size={10} />
                        </MeshDistortMaterial >
                        </Extrude>

                    </mesh>

                )}
                {/* <ContactShadows frames={1} rotation={[0, 0,0]} position={[0, 0, -0.5]} scale={10} opacity={1} far={10} blur={2} /> */}
                <Plane args={[10, 10]} rotation={[0, 0, -Math.PI / 2]} position={[0, 0, -1]} receiveShadow={true} >
                    <meshLambertMaterial />
                </Plane>
                <OrthographicCamera position={[0, 0, 10]} zoom={100} makeDefault />
                <OrbitControls />

                {/* </Backdrop> */}

            </Canvas>
            {_polygon &&
                _polygon.map((v: Vector2, i: number) => (
                    <Vertex
                        key={i}
                        point={v}
                        onDrag={(v: Vector2, callback: boolean) => {
                            let tmp = [..._polygon]
                            tmp[i] = v
                            // console.log(`I should be updating the mesh! ${tmp[2]!.x} ${tmp[2]!.y}`)
                            callback ? onChange(tmp) : updatePolygon(tmp)
                        }}
                    />
                ))}
        </>
    )
}
