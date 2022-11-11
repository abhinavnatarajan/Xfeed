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
class XfeedAudioProcessorEditor  : public juce::AudioProcessorEditor,
    private juce::Slider::Listener, juce::ToggleButton::Listener
{
public:
    XfeedAudioProcessorEditor (XfeedAudioProcessor&);
    ~XfeedAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    XfeedAudioProcessor& audioProcessor;
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    
    juce::Label header, gainKnobLabel, angleKnobLabel;;
    juce::Slider gainKnob, angleKnob;
    juce::TextButton bypassButton;
    juce::LookAndFeel_V4 bypassButtonLookAndFeel;
    juce::String bypassButtonOffText, bypassButtonOnText;

    juce::Colour bgcolour, buttonOnColour, buttonOffColour, textColour, textBoxBackgroundColour;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XfeedAudioProcessorEditor)
};
