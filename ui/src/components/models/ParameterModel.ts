import { makeAutoObservable } from 'mobx'
import { JuceVariant, Vertex } from '../valueTree/InputStream'
import { ValueTree } from '../valueTree/ValueTree'
import { setParameter } from '../messages/pluginMessages'

export class PolygonModel<T extends JuceVariant> {
	constructor(public parameter: ValueTree) {
		makeAutoObservable(this)
	}

	get id() {
		return this.parameter.properties.get('id') as string
	}

	get values() {
		const vertices: Vertex[] = []
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
