/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CW2DelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CW2DelayAudioProcessorEditor (CW2DelayAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~CW2DelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider delayTime;
    juce::Slider feedback;
    juce::Slider dryWet;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> feedbackValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> dryWetValue;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CW2DelayAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& treeState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CW2DelayAudioProcessorEditor)
};
