// dependencies
import { Canvas } from '@react-three/fiber'
import { useEffect, useRef, useState } from 'react'
import { Shape, Vector2 } from 'three'

const zoom: number = 180

const Vertex: React.FC<{
	point: Vector2
	onDrag: (v: Vector2, callback: boolean) => any
	className?: string
}> = ({ point, onDrag, className = '' }) => {
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
			x_window: zoom * point.x + (window.innerWidth * 0.625) / 2,
			y_window: zoom * -1 * point.y + window.innerHeight / 2,
		}
	}
	// handle movement of points
	const [mouseDown, setMouseDown] = useState<boolean>(false)
	useEffect(() => {
		// change position if mouse is down
		const changePosition = (e: MouseEvent) => {
			if (
				mouseDown &&
				e.clientX >= 0 &&
				e.clientX <= 500 &&
				e.clientY >= 0 &&
				e.clientY <= 400
			) {
				updatePosition({ x_window: e.clientX, y_window: e.clientY })
				onDrag(
					new Vector2(
						(e.clientX - (window.innerWidth * 0.625) / 2) / zoom,
						((e.clientY - window.innerHeight / 2) * -1) / zoom,
					),
					false,
				)
			}
		}
		// release mouse is mouse down
		const releasePoint = (e: MouseEvent) => {
			if (
				mouseDown &&
				e.clientX >= 0 &&
				e.clientX <= 500 &&
				e.clientY >= 0 &&
				e.clientY <= 400
			) {
				onDrag(
					new Vector2(
						(e.clientX - (window.innerWidth * 0.625) / 2) / zoom,
						((e.clientY - window.innerHeight / 2) * -1) / zoom,
					),
					true,
				)
			}
			setMouseDown(false)
		}
		// listeners
		window.addEventListener('mousemove', changePosition)
		window.addEventListener('mouseup', releasePoint)
		return () => {
			window.removeEventListener('mousemove', changePosition)
			window.removeEventListener('mouseup', releasePoint)
		}
	}, [mouseDown])

	return (
		<div
			className={`vertex ${className}`}
			style={{
				top: `${position.y_window - 5}px`,
				left: `${position.x_window - 5}px`,
			}}
			onMouseDown={() => setMouseDown(true)}
		/>
	)
}

export const Polygon: React.FC<{
	polygon: Vector2[]
	listener: Vector2
	onPolygonChange: (V: Vector2[]) => any
	onListenerChange: (V: Vector2) => any
}> = ({ polygon, listener, onPolygonChange, onListenerChange }): JSX.Element => {
	/*
	Body of the polygon.
	*/

	// where am i
	const mesh = useRef<THREE.Mesh>(null)
	const [_polygon, updatePolygon] = useState<Vector2[]>(polygon)
	const [_listener, updateListener] = useState<Vector2>(listener)
	// update Polygon from prop
	useEffect(() => updatePolygon(polygon), [polygon])
	useEffect(() => updateListener(listener), [listener])
	return (
		<>
			<Canvas
				orthographic
				camera={{
					position: [0, 0, 10],
					zoom,
					up: [0, 1, 0],
					far: 10000,
				}}
			>
				<ambientLight />
				<pointLight position={[10, 10, 10]} />
				{_polygon && (
					<mesh ref={mesh}>
						<shapeGeometry args={[new Shape(_polygon)]} />
						<meshStandardMaterial color='#2F7DF6' />
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
							callback ? onPolygonChange(tmp) : updatePolygon(tmp)
						}}
					/>
				))}
			<Vertex
				className='listener'
				point={listener}
				onDrag={(v: Vector2, _) => onListenerChange(v)}
			/>
		</>
	)
}
