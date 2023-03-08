import { makeAutoObservable } from 'mobx'
import { JuceVariant, Vertex } from '../valueTree/InputStream'
import { ValueTree } from '../valueTree/ValueTree'

export class PolygonModel<T extends JuceVariant> {
	constructor(public parameter: ValueTree) {
		makeAutoObservable(this)
		console.log('PolygonModel::constructor', this.parameter)
	}

	get id() {
		return this.parameter.properties.get('id') as string
	}

	get values() {
		const vertices: Vertex[] = []
		console.log('PolygonModel::values', this.parameter.properties.get('value'))
		// this.parameter.children.forEach((child) => {
		//   if (child.properties.get('x') !== undefined && child.properties.get('y') !== undefined) {
		//     vertices.push({
		//       id: child.properties.get('id') as number,
		//       x: child.properties.get('x') as number,
		//       y: child.properties.get('y') as number,
		//     })
		//   }
		// })
		return vertices
	}

	set value(value: T) {
		console.log('ParameterModel::value::set', value)
	}
}

export class ParameterModel<T extends JuceVariant> {
	constructor(public parameter: ValueTree) {
		makeAutoObservable(this)
	}

	get id() {
		return this.parameter.properties.get('id') as string
	}

	get value() {
		return this.parameter.properties.get('value') as T
	}

	get values() {
		const vertices: { x: number; y: number }[] = []
		this.parameter.children.forEach((child) => {
			if (
				child.properties.get('x') !== undefined &&
				child.properties.get('y') !== undefined
			) {
				vertices.push({
					x: child.properties.get('x') as number,
					y: child.properties.get('y') as number,
				})
			}
		})
		return vertices
	}

	set value(value: T) {
		console.log('ParameterModel::value::set', value)
		// this.callback && this.callback();
		// console.log(this.callback)
		// setParameter(this.id, value)
		// this.callback(this.id, value)
	}
}
