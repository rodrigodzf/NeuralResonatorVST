// dependencies
/// <reference types="vite/client" />
declare module 'leva'
declare module 'react-use-websocket'

// type send message
type SendMessage = { sendMessage: (msg: string) => void }
