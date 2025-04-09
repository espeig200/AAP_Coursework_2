/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CW2DelayAudioProcessor::CW2DelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
    , treeState(*this, nullptr, juce::Identifier("PARAMETERS"), createParameterLayout())
{

    const juce::StringArray params = { "delayTime", "feedback", "dryWet",};

    for (int i = 0; i < params.size(); ++i)
        treeState.addParameterListener(params[i], this);
       }

CW2DelayAudioProcessor::~CW2DelayAudioProcessor()
{
}

//==============================================================================
const juce::String CW2DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CW2DelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CW2DelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CW2DelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CW2DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CW2DelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CW2DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CW2DelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CW2DelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void CW2DelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CW2DelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    CW2Delay.reset();
    CW2Delay.prepare(spec);

    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = getTotalNumOutputChannels();

        // Set max delay time to 3 seconds
        int maxDelaySamples = static_cast<int>(sampleRate * 3.0);
        delayBuffer.setSize(getTotalNumOutputChannels(), maxDelaySamples);
        delayBuffer.clear();

        writePosition = 0;
    }

}

void CW2DelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CW2DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CW2DelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto numSamples = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    // Convert delay time from ms to samples
    int delaySamples = static_cast<int>((mDelayLine / 1000.0f) * getSampleRate());

    // BPM sync Logic
    bool tempoSync = treeState.getRawParameterValue("syncToTempo")->load();
    int divisionIndex = static_cast<int>(treeState.getRawParameterValue("noteDivision")->load());

    if (auto* playhead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playhead->getCurrentPosition(info) && info.bpm > 0)
        {
            currentBPM = info.bpm;
        }
    }

    if (tempoSync)
    {
        float beatLengthMs = 60000.0f / currentBPM;
        float delayMs = beatLengthMs; // Default: 1/4

        switch (divisionIndex)
        {
        case 0: delayMs = beatLengthMs; break;             // 1/4
        case 1: delayMs = beatLengthMs / 2.0f; break;       // 1/8
        case 2: delayMs = beatLengthMs / 3.0f; break;       // 1/8T
        case 3: delayMs = beatLengthMs / 4.0f; break;       // 1/16
        case 4: delayMs = beatLengthMs / 6.0f; break;       // 1/16T
        }

        mDelayLine = delayMs; // override regular delay time
    }

    // Process channels seperately
    if (getTotalNumInputChannels() < 2)
        return;
    {
        auto* channelDataL = buffer.getWritePointer(0);       // Pointer to current audio buffer (input/output)
        auto* channelDataR = buffer.getWritePointer(1);
        
        auto* delayDataL = delayBuffer.getWritePointer(0);    // Pointer to delay buffer
        auto* delayDataR = delayBuffer.getWritePointer(1);

        for (int i = 0; i < numSamples; ++i)
        {
            // Calculate current position in delay buffer to write to
            int bufferIndex = (writePosition + i) % delayBufferSize;

            // Calculate position to read from based on delay time (wrap around using modulo (%))
            int readIndex = (bufferIndex - delaySamples + delayBufferSize) % delayBufferSize;

            // feed L for bounce
            float inputSample = channelDataL[i];

            // Get the delayed sample from the delay buffer
            float delayedSampleL = delayDataL[readIndex];
            float delayedSampleR = delayDataR[readIndex];

            // Get the input sample
            float inL = channelDataL[i];
            float inR = channelDataR[i];

            // Write delay buffer, including feedback
            delayDataL[bufferIndex] = inputSample + delayedSampleR * mFeedback;
            delayDataR[bufferIndex] = delayedSampleL * mFeedback;

            // Ping Pong Logic
            float wetL = delayedSampleL;
            float wetR = delayedSampleR;

            channelDataL[i] = inL * (1.0f - dryWet) + wetL * dryWet;
            channelDataR[i] = inR * (1.0f - dryWet) + wetR * dryWet;
        }
    }

    // Move the write position forward by the number of samples just processed
    writePosition = (writePosition + numSamples) % delayBufferSize;
}



//==============================================================================
bool CW2DelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CW2DelayAudioProcessor::createEditor()
{
    return new CW2DelayAudioProcessorEditor (*this, treeState);
    }

//==============================================================================
void CW2DelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CW2DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CW2DelayAudioProcessor();
}
// Function called when parameter is changed
void CW2DelayAudioProcessor::parameterChanged(const juce::String& parameterID, float
    newValue)
{
    if (parameterID == "delayTime")
    {
        mDelayLine = newValue;
    }
    else if (parameterID == "feedback")
    {
        mFeedback = newValue;
    }
    else if (parameterID == "inGain")
    {
        inGain = newValue;
    }
    else if (parameterID == "dryWet")
    {
        dryWet = newValue;
    }
}  

juce::AudioProcessorValueTreeState::ParameterLayout CW2DelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delayTime", "Delay Time", juce::NormalisableRange<float>(10.0f, 3000.0f, 1.0f), 500.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "feedback", "Feedback", juce::NormalisableRange<float>(0.0f, 0.99f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "dryWet", "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "pingPong", "Ping Pong", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "syncToTempo", "Sync to Tempo", false));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "noteDivision", "Note Division", juce::StringArray{ "1/4", "1/8", "1/8T", "1/16", "1/16T" }, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lpfCutoff", "LPF Cutoff", juce::NormalisableRange<float>(100.0f, 10000.0f, 1.0f, 0.5f), 5000.0f));

    // BPM sync
    params.push_back(std::make_unique<juce::AudioParameterBool>("syncToTempo", "Sync to Tempo", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("noteDivision", "Note Division",
        juce::StringArray{ "1/4", "1/8", "1/8T", "1/16", "1/16T" }, 1)); // default = 1/8


    return { params.begin(), params.end() };
}

