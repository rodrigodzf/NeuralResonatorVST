// dependencies
import { useEffect, useRef, useState } from 'react'
import { Shape, Vector2 } from 'three'

export const Mesh = ({ polygon }: { polygon: Vector2[] }): JSX.Element => {
	/*
	Body of the polygon.
	*/
	const mesh = useRef<THREE.Mesh>(null)
	return (
		<mesh ref={mesh}>
			<shapeGeometry args={[new Shape(polygon)]} />
			<meshStandardMaterial color='orange' />
		</mesh>
	)
}

const Vertex: React.FC<{ point: Vector2, onDrag: (v: Vector2) => any }> = ({ point, onDrag }) => {
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
				onDrag(new Vector2(
					(e.clientX - (window.innerWidth / 2)) / 100.,
					(e.clientY - (window.innerHeight / 2)) * -1 / 100.,
				))
			}
		}
		// release mouse is mouse down
		const releasePoint = () => setMouseDown(false)
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

export const Vertices = ({
	polygon,
	onChange,
}: {
	polygon: Vector2[]
	onChange: (V: Vector2[]) => any
}): JSX.Element => {
	/*
	All vertices.
	*/
	// update polygon from prop
	const [_polygon, updatePolygon] = useState<Vector2[]>(polygon)
	useEffect(() => {
		console.log(polygon)
		updatePolygon(polygon)
	}, [polygon])
	return (
		<>
			{_polygon &&
				_polygon.map((v: Vector2, i: number) => (
					<Vertex
						key={i}
						point={v}
						onDrag={(v: Vector2) => {
							let tmp = _polygon
							tmp[i] = v
							onChange(tmp)
						}}
					/>
				))}
		</>
	)
}
