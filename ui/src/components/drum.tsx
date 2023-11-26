// dependencies
import { useEffect, useState } from 'react'

// src
import { MutablePoint } from './mutable-point'

export const Drum: React.FC<{
	polygon: Polygon
	strike: Point
	onPolygonChange?: (P: Polygon) => void
	onStrikeChange?: (p: Point) => void
}> = ({ polygon, strike, onPolygonChange = () => {}, onStrikeChange = () => {} }) => {
	/*
	Places a mutable, polygonal membrane within .drum using an SVG.
	*/

	// where am i
	const [_polygon, updatePolygon] = useState<Polygon>(polygon)
	const [_strike, updateStrike] = useState<Point>(strike)
	// update Polygon from prop
	useEffect(() => {
		updatePolygon(polygon)
	}, [polygon])
	useEffect(() => {
		updateStrike(strike)
	}, [strike])

	return (
		<div className='drum'>
			<svg version='1.1' x='0px' y='0px' viewBox='0 0 100 100' xmlSpace='preserve'>
				<polygon points={_polygon.map((p: Point) => `${p.x * 100},${p.y * 100}`).join(' ')} />
			</svg>
			{_polygon.map((p: Point, i: number) => (
				<MutablePoint
					key={i}
					point={p}
					onDrag={(v: Point, callback: boolean) => {
						const tmp: Polygon = [...polygon]
						tmp[i] = v
						callback ? onPolygonChange(tmp) : updatePolygon(tmp)
					}}
				/>
			))}
			<MutablePoint
				className='strike'
				point={_strike}
				onDrag={(p: Point) => {
					onStrikeChange(p)
				}}
			/>
		</div>
	)
}
