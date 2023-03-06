import { useControls } from 'leva'
import { observer } from 'mobx-react'
import { useContext, useEffect } from 'react'
import { sendMessageToJuce } from './juceCommunication'
import { ParametersContext } from './JuceIntegration'
import { setParameter } from './messages/pluginMessages'

const Panel = observer(() => {
  const parameters = useContext(ParametersContext)!

  const [, set] = useControls(() => ({
    densityUi: {
      value: 0.5,
      min: 0,
      max: 1,
      step: 0.01,
      onChange: (value) => {
        setParameter('density', value)
    },
    },
    stiffnessUi: {
      value: 0.5,
      min: 0,
      max: 1,
      step: 0.01,
      onChange: (value) => {
        setParameter('stiffness', value)
      },
    },
    pratioUi: {
      value: 0.5,
      min: 0,
      max: 1,
      step: 0.01,
      onChange: (value) => {
        setParameter('pratio', value)
      },
    },
    alphaUi: {
      value: 0.5,
      min: 0,
      max: 1,
      step: 0.01,
      onChange: (value) => {
        setParameter('alpha', value)
      },
    },
    betaUi: {
      value: 0.5,
      min: 0,
      max: 1,
      step: 0.01,
      onChange: (value) => {
        setParameter('beta', value)
      },
    },
  }))

  // this comes from the ws
  useEffect(() => {
    set({ densityUi: parameters.density.value })
  }, [parameters.density.value])

  useEffect(() => {
    set({ stiffnessUi: parameters.stiffness.value })
  }, [parameters.stiffness.value])

  useEffect(() => {
    set({ pratioUi: parameters.pratio.value })
  }, [parameters.pratio.value])

  useEffect(() => {
    set({ alphaUi: parameters.alpha.value })
  }, [parameters.alpha.value])

  useEffect(() => {
    set({ betaUi: parameters.beta.value })
  }, [parameters.beta.value])

  return null
})

export default Panel
