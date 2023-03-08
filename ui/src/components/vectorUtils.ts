import { Vector2 } from 'three'

export const convertArrayToVector2 = (vertices: number[]): Vector2[] => {
	const V: Vector2[] = []
	for (let i = 0; i < vertices.length; i += 2) {
		V.push(new Vector2(vertices[i], vertices[i + 1]))
	}
	return V
}

export const convertVector2ToArray = (vertices: Vector2[]): number[] => {
	const V: number[] = []
	vertices.forEach((v) => {
		V.push(v.x, v.y)
	})
	return V
}
