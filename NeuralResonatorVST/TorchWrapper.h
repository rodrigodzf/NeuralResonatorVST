#pragma once

#include "QueueThread.h"
#include "ProcessorIf.h"
#include "TorchWrapperIf.h"
#include "ServerThreadIf.h"
#include "RemoteParameterAttachment.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <torch/script.h>
#include <torch/torch.h>

class TorchWrapper : public TorchWrapperIf, private juce::ValueTree::Listener
{
public:
    enum class ModelType
    {
        ShapeEncoder,
        FC
    };

    TorchWrapper(
        ProcessorIf* processorPtr,
        juce::AudioProcessorValueTreeState& vts
    );
    ~TorchWrapper();

    TorchWrapperIf* getTorchWrapperIfPtr() override;

    void loadModel(
        const std::string& modelPath,
        const ModelType modelType,
        const std::string& deviceString = "cpu"
    );

    void handleReceivedNewShape(const juce::Path shape);

    void updateMaterial(const std::vector<float>& material);
    void updatePosition(const std::vector<float>& position);

    void predictCoefficients();

    void setServerThreadIf(ServerThreadIf* serverThreadIfPtr);
    bool startThread();

protected:
    void valueTreePropertyChanged(
        juce::ValueTree& changedTree,
        const juce::Identifier& changedProperty
    ) override;

    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
        override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;
    void valueTreeParentChanged(juce::ValueTree&) override;

private:
    torch::jit::Module mShapeEncoderNetwork;
    torch::jit::Module mFCNetwork;

    // intermediate tensor for features
    torch::Tensor mFeatureTensor;
    torch::Tensor mLastMaterialTensor;
    torch::Tensor mLastPositionTensor;
    std::vector<float> mCoefficients;

    // flags
    bool mFeaturesReady = false;

    // value tree state
    juce::AudioProcessorValueTreeState& mVts;
    std::unique_ptr<RemoteParameterAttachment> densityAttachment;
    std::unique_ptr<RemoteParameterAttachment> stiffnessAttachment;
    std::unique_ptr<RemoteParameterAttachment> poissonsRatioAttachment;
    std::unique_ptr<RemoteParameterAttachment> alphaAttachment;
    std::unique_ptr<RemoteParameterAttachment> betaAttachment;
    std::unique_ptr<RemoteParameterAttachment> xposAttachment;
    std::unique_ptr<RemoteParameterAttachment> yposAttachment;

    // server thread if
    ServerThreadIf* mServerThreadIf = nullptr;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ProcessorIf* mProcessorPtr;

    QueueThread mQueueThread{"torch_wrapper"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TorchWrapper)
};
