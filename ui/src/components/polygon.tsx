// dependencies
import { Canvas } from '@react-three/fiber'
import { useEffect, useRef, useState } from 'react'
import { Shape, Vector2 } from 'three'

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
	useEffect(() => {
		updatePosition(relativePosition(point))
	}, [point])
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

	// where am i
	const [_polygon, updatePolygon] = useState<Vector2[]>(polygon)
	// update Polygon from prop
	useEffect(() => {
		updatePolygon(polygon)
	}, [polygon])
	const mesh = useRef<THREE.Mesh>(null)
	return (
		<>
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
				{_polygon && (
					<mesh ref={mesh}>
						<shapeGeometry args={[new Shape(_polygon)]} />
						<meshStandardMaterial color='orange' />
					</mesh>
				)}
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
