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
    , treeState(*this, nullptr, juce::Identifier("PARAMETERS"),
        { std::make_unique<juce::AudioParameterFloat>("delayTime", "delayTime", 10.0f, 3000.0f, 500.0f),
        std::make_unique<juce::AudioParameterFloat>("feedback", "feedback", 0.0f, 0.99f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>("dryWet", "dryWet", 0.0f, 1.0f, 0.5f)})
{
       const juce::StringArray params = { "delayTime", "feedback", "dryWet",};

       for (int i = 0; i <= 3; ++i)
       {
               // adds a listener to each parameter in the array.
               treeState.addParameterListener(params[i], this);
       }

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

void CW2DelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelDataL = buffer.getWritePointer(0);
        auto* channelDataR = buffer.getWritePointer(1);

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float inL = channelDataL[i]; // 1
            float inR = channelDataR[i]; // 1
            float tempL = CW2Delay.popSample(0, mDelayLine); // 2
            float tempR = CW2Delay.popSample(1, mDelayLine); // 2
            CW2Delay.pushSample(0, inL + (tempL * mFeedback)); // 3
            CW2Delay.pushSample(1, inR + (tempR * mFeedback)); // 3
            channelDataL[i] = (inL + tempL) * 0.5f; // 4
            channelDataR[i] = (inR + tempR) * 0.5f; // 4

            // Dry/Wet
            channelDataL[i] = (channelDataL[i] * (1.0f - dryWetMix)) + (wetSignal * dryWetMix);
            channelDataR[i] = (channelDataR[i] * (1.0f - dryWetMix)) + (wetSignal * dryWetMix);
        }
    }
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
}
