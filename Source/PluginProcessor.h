/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class CW2DelayAudioProcessor  : public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener //initialises tree listener


{
public:
    //==============================================================================
    CW2DelayAudioProcessor();
    ~CW2DelayAudioProcessor() override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    juce::AudioBuffer<float> delayBuffer; // declare Delay buffer
    int writePosition = 0; // declare write Position

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged(const juce::String& parameterID, float newValue) override; //declares function 

private:
    juce::AudioProcessorValueTreeState treeState; //initialisation of Parameter Tree
    juce::dsp::DelayLine<float> CW2Delay{ 22050 };

    float mDelayLine = 1000.0f;
    float mFeedback = 0.3f;
    float inGain;
    float dryWet;
    float currentBPM = 120.0f;

    // Dry/Wet Simple
    // set up float pointer for Input gain
    juce::AudioParameterFloat* inputGain;

    // dry wet mix control
    juce::AudioParameterFloat* dryWetMix;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CW2DelayAudioProcessor)
    
};
