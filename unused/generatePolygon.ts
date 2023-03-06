// Point class
export class Point {
	x: number
	y: number
	constructor(x: number, y: number) {
		this.x = x
		this.y = y
	}
	theta(): number {
		return Math.atan2(this.y, this.x)
	}
}

class Polygon extends Array<Point> {}

type SplitEven<
	T extends readonly any[],
	Start extends any[] = [],
	End extends any[] = [],
> = T['length'] extends 0 | 1
	? [[...Start, ...T], End]
	: T extends readonly [infer S, ...infer M, infer E]
	? SplitEven<M, [...Start, S], [E, ...End]>
	: never

type Shuffle<T extends readonly any[]> = T['length'] extends 0 | 1
	? T
	: SplitEven<T> extends infer A
	? A extends readonly [readonly [infer AH, ...infer AR], readonly [infer BH, ...infer BR]]
		? [AH, BH, ...Shuffle<[...AR, ...BR]>]
		: never
	: never

function shuffle<T extends readonly any[]>(array: T): Shuffle<T> {
	if (array.length === 0 || array.length === 1) {
		return array.slice() as Shuffle<T>
	}

	const rest = array.slice()
	const middle = rest.splice(Math.ceil(array.length / 2), 1)[0]
	const first = rest.shift()

	return [first, middle, ...shuffle(rest)] as Shuffle<T>
}
function generateConvexPolygon(N: number, seed: number = 0): Polygon {
	/*
      Generate convex shapes according to Pavel Valtr's 1995 algorithm.
      Adapted from Sander Verdonschot's Java version, found here:
      https://cglab.ca/~sander/misc/ConvexGeneration/ValtrAlgorithm.java
      input:
          N = the number of vertices
          seed? = the seed for the random number generators
      output:
          V = a polygon of N random vertices
      */

	// Initialise variables
	const P: Polygon = []
	let X: number[] = new Array(N)
	let Y: number[] = new Array(N)
	const X_rand: number[] = new Array(N)
	const Y_rand: number[] = new Array(N)
	let last_true: number = 0
	let last_false: number = 0

	// Initialise and sort random coordinates
	X_rand.fill(0).map(() => Math.random())
	Y_rand.fill(0).map(() => Math.random())
	X_rand.sort()
	Y_rand.sort()

	// divide the interior points into two chains
	for (let i = 1; i < N; i++) {
		if (i != N - 1) {
			if (Math.round(Math.random()) === 1) {
				X[i] = X_rand[i] - X_rand[last_true]
				Y[i] = Y_rand[i] - Y_rand[last_true]
				last_true = i
			} else {
				X[i] = X_rand[last_false] - X_rand[i]
				Y[i] = Y_rand[last_false] - Y_rand[i]
				last_false = i
			}
		} else {
			X[0] = X_rand[i] - X_rand[last_true]
			Y[0] = Y_rand[i] - Y_rand[last_true]
			X[i] = X_rand[last_false] - X_rand[i]
			Y[i] = Y_rand[last_false] - Y_rand[i]
		}
	}

	// randomly combine x and y coordinates
	Y = shuffle(Y)

	for (let i = 0; i < N; i++) {
		P.push(new Point(X[i], Y[i]))
	}

	// sort by polar angle
	P.sort((p1: Point, p2: Point) => p1.theta() - p2.theta())

	// arrange points end to end to form a polygon
	let x_min: number = Infinity
	let x_max: number = -Infinity
	let y_min: number = Infinity
	let y_max: number = -Infinity
	let x: number = 0.0
	let y: number = 0.0

	for (let i = 0; i < N; i++) {
		const p: Point = new Point(x, y)
		x += P[i].x
		y += P[i].y
		P[i] = p
		x_min = Math.min(P[i].x, x_min)
		x_max = Math.max(P[i].x, x_max)
		y_min = Math.min(P[i].y, y_min)
		y_max = Math.max(P[i].y, y_max)
	}

	// center around origin
	for (let i = 0; i < N; i++) {
		P[i].x += (x_max - x_min) / 2.0 - x_max
		P[i].y += (y_max - y_min) / 2.0 - y_max
	}

	return P
}
