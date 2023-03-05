import React, { useRef, useContext, useEffect, useLayoutEffect, useState, FC } from 'react'
import '../css/App.css'
// import { Leva } from "leva";
import { Canvas } from '@react-three/fiber'
import Panel from './Panel'
import { Mesh } from './Mesh'
import { useGlobalState, setGlobalState } from './State'
import { makeConnectedParametersModel, ParametersModelType } from './models/ParametersModel'
import { callbacks, JuceMessage } from './juceCommunication'
import { VALUE_TREE_STATE_CHANGE } from './messages/callbackEventTypes'
import { observer } from 'mobx-react'
import { JuceIntegration, ParametersContext } from './JuceIntegration'
import { ParameterModel } from './models/ParameterModel'
import useWebSocket from 'react-use-websocket'

interface Props {
  parameter: ParameterModel<number>
}

export const ParameterSlider: FC<Props> = observer((props) => {
  return props.parameter ? <h1>{props.parameter.value}</h1> : null
})

const ParamDebug = () => {
  const parameters = useContext(ParametersContext)!
  return (
    console.log('density', parameters.density.value),
    (<ParameterSlider parameter={parameters.density} />)
  )
}

const App = observer(() => {
  const [endpoint, setEndpoint] = useGlobalState('endpoint')
  const ws = useRef<WebSocket | undefined>()

  const { sendMessage } = useWebSocket(endpoint, {
    onOpen: () => console.log('ws opened'),
    onClose: () => console.log('ws closed'),
    onMessage: (event) => {
      console.log('ws message')
      const message: JuceMessage<any> = {
        eventType: VALUE_TREE_STATE_CHANGE,
        data: {
          treeId: 'PARAMETERS',
          changes: [event.data],
        },
      }

      try {
        const existingCallbacks = callbacks.get(message.eventType)

        if (existingCallbacks) {
          existingCallbacks.forEach((cb) => cb(message.data))
        } else {
          console.log(`No callbacks registered for event type "${message.eventType}"`, {
            message,
          })
        }
      } catch (e) {
        console.error('Error handling message from JUCE', { e, message })
        throw e
      }
    },
    shouldReconnect: (closeEvent) => false,
  })

  // componentDidMount
  useLayoutEffect(() => {
    console.log('ws mounted')
    return () => {
      console.log('ws unmounted')
    }
  }, [])

  return (
    <>
      <JuceIntegration>
        {/* <ParamDebug/> */}
        <Panel sendMessage={sendMessage} />
        <Canvas
          orthographic
          camera={{
            position: [0, 0, 10],
            zoom: 100,
            up: [0, 1, 0],
            far: 10000,
          }}
        >
          <ambientLight />
          <pointLight position={[10, 10, 10]} />
          <Mesh sendMessage={sendMessage} />
        </Canvas>
      </JuceIntegration>
    </>
  )
})

export default App
