/**
 * A Typescript implementation of JUCE's ValueTreeSynchroniser class, to
 * allow our JS app to receive ValueTree state updates. The implementation
 * is read-only, i.e. it can only receive updates, not send them.
 */

import { registerCallback } from '../juceIntegration'
import { VALUE_TREE_STATE_CHANGE } from '../messages/callbackEventTypes'
import { InputStream } from './InputStream'
import { performUpdate } from './mobxHelpers'
import { ValueTree } from './ValueTree'

enum ChangeType {
	propertyChanged = 1,
	fullSync = 2,
	childAdded = 3,
	childRemoved = 4,
	childMoved = 5,
	propertyRemoved = 6,
}

export const applyChange = (
	valueTree: ValueTree,
	input: Uint8Array,
	onFullSync = () => {},
): boolean => {
	const inputStream = new InputStream(input)

	const type: ChangeType = inputStream.readByte()

	if (type === ChangeType.fullSync) {
		valueTree.readFromStream(inputStream)
		onFullSync()
		return true
	}

	const subtree = readSubTreeLocation(valueTree, inputStream)

	if (!subtree.isValid()) return false

	switch (type) {
		case ChangeType.propertyChanged: {
			const property = inputStream.readString()

			performUpdate(() => {
				subtree.properties.set(property, inputStream.readVar())
			})
			return true
		}

		case ChangeType.propertyRemoved: {
			const property = inputStream.readString()
			performUpdate(() => {
				subtree.properties.delete(property)
			})
			return true
		}

		case ChangeType.childAdded: {
			const index = inputStream.readCompressedInt()
			performUpdate(() => {
				subtree.children.splice(index, 0, ValueTree.readFromStream(inputStream))
			})
			return true
		}

		case ChangeType.childRemoved: {
			const index = inputStream.readCompressedInt()
			if (index < subtree.children.length && index >= 0) {
				performUpdate(() => {
					subtree.children.splice(index, 1)
				})
				return true
			}

			console.assert(false, 'Invalid child index - data corrupt or trees are out of sync')
			break
		}

		case ChangeType.childMoved: {
			const oldIndex = inputStream.readCompressedInt()
			const newIndex = inputStream.readCompressedInt()

			if (
				oldIndex < subtree.children.length &&
				oldIndex >= 0 &&
				newIndex < subtree.children.length &&
				newIndex >= 0
			) {
				performUpdate(() => {
					const temp = subtree.children[oldIndex]
					subtree.children[oldIndex] = subtree.children[newIndex]
					subtree.children[newIndex] = temp
				})
				return true
			}

			console.assert(false, 'Invalid child index - data corrupt or trees are out of sync')
			break
		}

		default:
			console.assert(false, 'Invalid operation received - data corrupt')
			break
	}

	return false
}

const readSubTreeLocation = (valueTree: ValueTree, input: InputStream) => {
	const numLevels = input.readCompressedInt()

	if (numLevels > 65536 || numLevels < 0) return new ValueTree()

	for (let i = numLevels; --i >= 0; ) {
		const index = input.readCompressedInt()

		if (index > valueTree.children.length || index < 0) return new ValueTree()

		valueTree = valueTree.children[index]
	}

	return valueTree
}

type ValueTreeStateChangeMessageBody = { treeId: string; changes: string[] }

export const createValueTreeSynchroniser = (
	treeId: string,
	targetTree: ValueTree,
	onInitialSyncRecieved = () => {},
) => {
	const initialSyncRecieved = false

	registerCallback(VALUE_TREE_STATE_CHANGE, (message: ValueTreeStateChangeMessageBody) => {
		if (message.treeId !== treeId) return

		performUpdate(() => {
			message.changes.forEach((change) => {
				applyChange(
					targetTree,
					// Decode base64 encded change: https://stackoverflow.com/questions/21797299/convert-base64-string-to-arraybuffer
					Uint8Array.from(atob(change), (c) => c.charCodeAt(0)),
					() => {
						if (!initialSyncRecieved) onInitialSyncRecieved()
					},
				)
			})
		})
	})
}
