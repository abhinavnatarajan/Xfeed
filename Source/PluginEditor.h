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
    
    //Drawable svg_drawable_play;
    juce::Label header;
    juce::Slider gainKnob;
    juce::Label gainKnobLabel;
    juce::Slider angleKnob;
    juce::Label angleKnobLabel;
    juce::TextButton bypassButton;

    juce::Colour bgcolour, buttonOnColour, buttonOffColour, textColour, textBoxBackgroundColour;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XfeedAudioProcessorEditor)
};
