/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CW2DelayAudioProcessorEditor::CW2DelayAudioProcessorEditor(CW2DelayAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), audioProcessor(p), treeState(vts)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(300, 300);
    // delayTime
    delayTimeValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (treeState, "delaytime", delayTimeDial);
    delayTimeDial.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayTimeDial.setRange(10.0f, 3000.0f, 500.0f);
    delayTimeDial.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(&delayTimeDial);
    // Feedback
    feedbackValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "feedback", feedbackDial);
    feedbackDial.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    feedbackDial.setRange(0.0f, 1.10f, 0.0f);
    feedbackDial.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(&feedbackDial);
    // dryWet
    dryWetValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "dryWet", dryWetDial);
    dryWetDial.setSliderStyle(juce::Slider::LinearHorizontal);
    dryWetDial.setRange(0.0f, 1.0f, 0.1f);
    dryWetDial.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(&dryWetDial);

    // BPM Sync
    syncToTempoButton.setButtonText("Sync to Tempo");
    addAndMakeVisible(syncToTempoButton);
    syncToTempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(treeState, "syncToTempo", syncToTempoButton);

    divisionBox.addItemList({ "1/4", "1/8", "1/8T", "1/16", "1/16T" }, 1);
    addAndMakeVisible(divisionBox);
    divisionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(treeState, "noteDivision", divisionBox);
}

CW2DelayAudioProcessorEditor::~CW2DelayAudioProcessorEditor()
{
}

//==============================================================================
void CW2DelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::purple);
    g.setColour(juce::Colours::silver);
    // Title Text
    g.setFont(30);
    g.drawFittedText("The PingPongaLong mk1", 5, 20, 290, 10, juce::Justification::centred, 1, 0.0f);
    // Delay, Feedback, and Dry/Wet labels
    g.setFont(20);
    g.drawFittedText("Dt", 25, 125, 70, 10, juce::Justification::centred, 1, 0.0f);
    g.drawFittedText("F", 195, 125, 70, 10, juce::Justification::centred, 1, 0.0f);
    g.drawFittedText("Dry/Wet", 100, 190, 100, 10, juce::Justification::centred, 1, 0.0f);
}

void CW2DelayAudioProcessorEditor::resized()
{
    delayTimeDial.setBounds(10, 40, 100, 100);
    feedbackDial.setBounds(180, 40, 100, 100);
    dryWetDial.setBounds(50, 130, 200, 100);
    syncToTempoButton.setBounds(10, 220, 150, 25);
    divisionBox.setBounds(170, 220, 100, 25);
  

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}


