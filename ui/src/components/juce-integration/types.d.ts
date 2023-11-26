export interface JuceMessage<T> {
	eventType: string
	data?: T
}
