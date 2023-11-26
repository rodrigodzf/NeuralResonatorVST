/**
 * A Typescript implementation of the relevant parts of JUCE's InputStream class,
 * to allow us to parse data out of the encoded ValueTree changes
 */

import { ValueTree } from './ValueTree'

// For jest
// const TextDecoderImpl = typeof TextDecoder !== 'undefined' ? TextDecoder : 'undefined'

enum VariantStreamMarkers {
	varMarker_Int = 1,
	varMarker_BoolTrue = 2,
	varMarker_BoolFalse = 3,
	varMarker_Double = 4,
	varMarker_String = 5,
	varMarker_Int64 = 6,
	varMarker_Array = 7,
	varMarker_Binary = 8,
	varMarker_Undefined = 9,
}

export type JuceVariant = number | BigInt | string | boolean | undefined | ValueTree | Point | JuceVariant[]

export class InputStream {
	constructor(private data: Uint8Array) {}

	private readPosition = 0

	readString = (): string => {
		let buffer = ''

		while (true) {
			const c = this.readByte()
			if (c === 0) return buffer

			buffer += String.fromCharCode(c)
		}
	}

	readByte = (): number => {
		const byte = this.data[this.readPosition]
		this.incrementReadPosition(1)
		return byte || 0
	}

	readInt = (): number => {
		const bytes = this.read(4)
		// TODO error handling
		return this.makeInt32(bytes)
	}

	readInt64 = (): BigInt => {
		const bytes = this.read(8)
		// TODO error handling
		return this.makeInt64(bytes)
	}

	readDouble = (): number => {
		const bytes = this.read(8)
		// TODO error handling
		return this.makeDouble(bytes)
	}

	readCompressedInt = (): number => {
		const sizeByte = this.readByte()

		if (sizeByte === 0) return 0

		const numBytes = sizeByte & 0x7f

		if (numBytes > 4) {
			console.assert(false, 'Data is corrupted')
			return 0
		}

		let bytes: number[] = [0, 0, 0, 0]

		// TODO error handling: if (read (bytes, numBytes) != numBytes)
		for (let i = 0; i < numBytes; i++) {
			bytes[i] = this.readByte()
		}

		const number = this.makeInt32(bytes)

		return sizeByte >> 7 ? -number : number
	}

	readVar = (): JuceVariant => {
		const numBytes = this.readCompressedInt()

		if (numBytes === 0) {
			console.assert(false, 'Data is corrupted (I think!)')
			return undefined
		}

		const marker = this.readByte()

		switch (marker) {
			case VariantStreamMarkers.varMarker_Int:
				return this.readInt()
			case VariantStreamMarkers.varMarker_Int64:
				return this.readInt64()
			case VariantStreamMarkers.varMarker_BoolTrue:
				return true
			case VariantStreamMarkers.varMarker_BoolFalse:
				return false
			case VariantStreamMarkers.varMarker_Double:
				return this.readDouble()
			case VariantStreamMarkers.varMarker_String:
				const string = new TextDecoder().decode(this.read(numBytes - 2))
				this.incrementReadPosition(1) // skip over the 0 byte end marker
				return string
			case VariantStreamMarkers.varMarker_Binary:
				throw new Error('Not implemented')
			case VariantStreamMarkers.varMarker_Array:
				const array: any = []
				for (let i = this.readCompressedInt(); --i >= 0; ) array.push(this.readVar())
				return array
			default:
				this.incrementReadPosition(numBytes - 1)
				return
		}
	}

	private makeInt32 = (bytes: number[] | Uint8Array): number => {
		// from ByteOrder::makeInt
		return bytes[0]! | (bytes[1]! << 8) | (bytes[2]! << 16) | (bytes[3]! << 24)
	}

	private makeInt64 = (bytes: Uint8Array): BigInt => {
		// This is a neater way to do the conversion but currently unsupported on Safari
		/*const dataView = new DataView(bytes.buffer);
    return dataView.getBigInt64(0, true);*/

		// Based on https://coolaj86.com/articles/convert-js-bigints-to-typedarrays/,
		// corrected for little-endian format and variables renamed for clarity
		const hexArray: string[] = []

		bytes.forEach((byte) => {
			let hex = byte.toString(16)
			if (hex.length % 2) {
				hex = '0' + hex
			}
			hexArray.unshift(hex)
		})

		return BigInt('0x' + hexArray.join(''))
	}

	private makeDouble = (bytes: Uint8Array): number => {
		const dataView = new DataView(bytes.buffer)
		return dataView.getFloat64(0, true)
	}

	private incrementReadPosition = (delta: number) => {
		this.readPosition = Math.min(this.readPosition + delta, this.data.length - 1)
	}

	private read = (size: number): Uint8Array => {
		const readPosition = this.readPosition
		this.incrementReadPosition(size)

		// TODO error handling
		return this.data.slice(readPosition, readPosition + size)
	}
}
