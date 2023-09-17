// dependencies
import { useEffect, useRef, useState } from 'react'

export const MutablePoint: React.FC<{
	point: Point
	onDrag: (p: Point, callback: boolean) => any
	className?: string
}> = ({ point, onDrag, className = '' }) => {
	/*
	A handle for a single Point.
	*/

	const self = useRef<HTMLDivElement>(null)
	// handle movement of points
	const [mouseDown, setMouseDown] = useState<boolean>(false)
	useEffect(() => {
		// update the location of a point and fire callback
		function updatePoint(e: MouseEvent, callback: boolean): void {
			if (mouseDown && self?.current?.parentElement) {
				const parent = self.current.parentElement.getBoundingClientRect()
				if (
					e.clientX >= parent.left &&
					e.clientX <= parent.right &&
					e.clientY >= parent.top &&
					e.clientY <= parent.bottom
				) {
					onDrag(
						{
							x: (e.clientX - parent.left) / (parent.right - parent.left),
							y: (e.clientY - parent.top) / (parent.bottom - parent.top),
						},
						callback,
					)
				}
			}
		}
		// change position if mouse is down
		function changePosition(e: MouseEvent): void {
			updatePoint(e, false)
		}
		// release mouse and fire callback if mouse down
		function releasePoint(e: MouseEvent): void {
			updatePoint(e, true)
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
			ref={self}
			className={`vertex ${className}`}
			style={{
				top: `calc(${point.y * 100}% - 5px)`,
				left: `calc(${point.x * 100}% - 5px)`,
			}}
			onMouseDown={() => setMouseDown(true)}
		/>
	)
}
