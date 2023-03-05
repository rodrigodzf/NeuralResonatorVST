import { makeAutoObservable, toJS } from 'mobx'
import { ParametersType, PARAMETER_IDS } from './parameters'
import { ValueTree } from '../valueTree/ValueTree'
import { createValueTreeSynchroniser } from '../valueTree/ValueTreeSynchroniser'
import { ParameterModel } from './ParameterModel'

export class ParametersModel {
	constructor(private valueTree: ValueTree, parameterIds: string[]) {
		//console.log((this.valueTree.childrenWithProperty('id', 'vertices')!).properties.get('value')!)

		parameterIds.forEach((parameterId) => {
			;(this as any)[parameterId] = new ParameterModel(
				this.valueTree.childrenWithProperty('id', parameterId)!,
			)
		})

		makeAutoObservable(this)
	}
}

// Used to inject the parameter types, which are dynamically created fields
export const makeParametersModel = <T>(valueTree: ValueTree, parameterIds: string[]) => {
	return new ParametersModel(valueTree, parameterIds) as ParametersModel & T
}

// Default way to create a parameters model hooked up to the AudioProcessorValueTreeState.
// Promise resolves once the initial value tree sync has arrived
export const makeConnectedParametersModel = async <T = ParametersModelType>(
	callback: Function = () => {},
	parameterIds = PARAMETER_IDS,
	treeId: string = 'PARAMETERS',
) => {
	return new Promise<T>((resolve) => {
		const valueTree = new ValueTree()

		createValueTreeSynchroniser(treeId, valueTree, () => {
			const parametersModel = makeParametersModel<T>(valueTree, parameterIds, callback)
			resolve(parametersModel)
		})
	})
}

export type ParametersModelType = ParametersModel & ParametersType
