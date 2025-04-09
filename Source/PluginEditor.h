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
    
 // This reference is provided as a quick way for your editor to
 // access the processor object that created it.
    CW2DelayAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& treeState;
    
    juce::Slider delayTimeDial;
    juce::Slider feedbackDial;
    juce::Slider dryWetDial;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> feedbackValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> dryWetValue;
    
    //BPM sync UI
    juce::ToggleButton syncToTempoButton;
    juce::ComboBox divisionBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncToTempoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CW2DelayAudioProcessorEditor)
};
