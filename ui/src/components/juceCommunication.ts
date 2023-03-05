export const callbacks = new Map<string, Function[]>();

export interface JuceMessage<T> {
  eventType: string;
  data?: T;
}

export const registerCallback = <T extends Function>(
  eventType: string,
  handler: T
) => {
  const existingCallbacks = callbacks.get(eventType);

  if (existingCallbacks) existingCallbacks.push(handler);
  else callbacks.set(eventType, [handler]);
};
