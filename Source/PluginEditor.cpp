/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <string>

//==============================================================================
XfeedAudioProcessorEditor::XfeedAudioProcessorEditor (XfeedAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    // Set colours
    bgcolour = juce::Colours::lightblue;
    buttonOffColour = juce::Colour(0xff228b22);
    buttonOnColour = juce::Colour(0xff42a2c8);
    textColour = juce::Colour(0xff1e1e1e);
    textBoxBackgroundColour = juce::Colour(0xff263238);
    getLookAndFeel().setColour(juce::Label::textColourId, textColour); // labels
    getLookAndFeel().setColour(juce::Slider::textBoxBackgroundColourId, textBoxBackgroundColour); // slider text box background
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, buttonOffColour); // button engaged
    getLookAndFeel().setColour(juce::TextButton::buttonOnColourId, buttonOnColour); // button disengaged
    getLookAndFeel().setColour(juce::ComboBox::outlineColourId, juce::Colour(0x00000000)); // button outline

    // Header
    addAndMakeVisible(&header);
    std::string pluginName = JucePlugin_Name, pluginVersion = JucePlugin_VersionString;
    header.setText(pluginName + " " + pluginVersion, juce::dontSendNotification);
    header.setJustificationType(juce::Justification::centred);
    header.setFont(juce::Font(24.0f, juce::Font::FontStyleFlags::plain));

    // Knobs
    constexpr static float pi = juce::MathConstants<float>::pi;
    constexpr auto knobsMinAngle = (- 135.0f + 360.0f) * pi / 180.0f, knobsMaxAngle = (135.0f + 360.0f) * pi / 180.0f;

    // Gain knob
    gainKnob.setSliderStyle(juce::Slider::Rotary);
    gainKnob.setRange(p.minGaindB, p.maxGaindB, 0.1);
    gainKnob.setRotaryParameters(knobsMinAngle, knobsMaxAngle, true);
    gainKnob.setTextValueSuffix("dB");
    gainKnob.setValue(p.defaultGaindB);
    gainKnob.setPopupMenuEnabled(true);
    gainKnob.setPopupDisplayEnabled(false, false, this);
    gainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
    gainKnob.setDoubleClickReturnValue(true, p.defaultGaindB);
    gainKnob.addListener(this);
    addAndMakeVisible(&gainKnob);

    // Gain knob label
    gainKnobLabel.setText("Gain", juce::dontSendNotification);
    addAndMakeVisible(&gainKnobLabel);
    gainKnobLabel.setJustificationType(juce::Justification::centred);
    gainKnobLabel.setFont(juce::Font(18.0f, juce::Font::FontStyleFlags::plain));

    // Angle knob
    angleKnob.setSliderStyle(juce::Slider::Rotary);
    angleKnob.setRange(p.minAngle, p.maxAngle, 1.0);
    angleKnob.setRotaryParameters(knobsMinAngle, knobsMaxAngle, true);
    angleKnob.setTextValueSuffix(juce::CharPointer_UTF8("\xc2\xb0")); // degree symbol
    angleKnob.setValue(p.defaultAngle);
    gainKnob.setPopupMenuEnabled(true);
    angleKnob.setPopupDisplayEnabled(false, false, this);
    angleKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);
    angleKnob.setDoubleClickReturnValue(true, p.defaultAngle);
    addAndMakeVisible(&angleKnob);
    angleKnob.addListener(this);

    // Angle knob label
    angleKnobLabel.setText("Angle", juce::dontSendNotification);
    addAndMakeVisible(&angleKnobLabel);
    angleKnobLabel.setJustificationType(juce::Justification::centred);
    angleKnobLabel.setFont(juce::Font(18.0f, juce::Font::FontStyleFlags::plain));

    // Bypass button
    bypassButton.setToggleable(true);
    bypassButton.setToggleState(false, juce::sendNotification);
    bypassButton.setClickingTogglesState(true);
    bypassButton.setButtonText("Bypass");
    addAndMakeVisible(&bypassButton);
    bypassButton.addListener(this);

    setSize(300, 230); // last so that everything logo ptr is not empty when resized() is called
}

XfeedAudioProcessorEditor::~XfeedAudioProcessorEditor()
{
}

void XfeedAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainKnob) {
        *(audioProcessor.gaindB) = static_cast<float>(gainKnob.getValue());
    }
    else if (slider == &angleKnob) {
        *(audioProcessor.angle) = static_cast<float>(angleKnob.getValue());
    }
    audioProcessor.updateFilterCoeffs(static_cast<float>(audioProcessor.getSampleRate()));
}

void XfeedAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &bypassButton) {
        if (button->getToggleState()) {
            *(audioProcessor.bypass) = true;
            button->setButtonText("Bypassed");
        }
        else {
            *(audioProcessor.bypass) = false;
            button->setButtonText("Bypass");
        }
        audioProcessor.updateBypassRamps();
    }
}

//==============================================================================
void XfeedAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(bgcolour);
}

void XfeedAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    constexpr static int headerHeight = 30,
        knobWidth = 100, knobHeight = 100,
        knobLabelWidth = 75, knobLabelHeight = 30,
        bypassButtonHeight = 30, bypassButtonWidth = 80,
        bypassButtonPadding = 10;
    // calculate header height and width

    juce::Rectangle area = getLocalBounds();
    int topBottomMargin = static_cast<int>(std::floor((area.getHeight() - knobHeight - knobLabelHeight - headerHeight - bypassButtonHeight - bypassButtonPadding) / 2));
    // Header
    area.removeFromTop(topBottomMargin);
    area.removeFromBottom(topBottomMargin);
    header.setBounds(area.removeFromTop(headerHeight));
    // Bypass button
    juce::Rectangle bypassButtonArea = area.removeFromBottom(bypassButtonHeight + bypassButtonPadding);
    bypassButtonArea.removeFromTop(bypassButtonPadding);
    int leftRightMargin = static_cast<int>(std::floor((bypassButtonArea.getWidth() - bypassButtonWidth) / 2));
    bypassButtonArea.removeFromLeft(leftRightMargin);
    bypassButtonArea.removeFromRight(leftRightMargin);
    bypassButton.setBounds(bypassButtonArea);
    // Gain knob
    leftRightMargin = static_cast<int>(std::floor((area.getWidth() - 2 * knobWidth) / 2));
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    juce::Rectangle gainKnobArea = area.removeFromLeft(knobWidth);
    gainKnobLabel.setBounds(gainKnobArea.removeFromTop(knobLabelHeight));
    gainKnob.setBounds(gainKnobArea);
    // Angle knob
    juce::Rectangle angleKnobArea = area.removeFromLeft(knobWidth);
    angleKnobLabel.setBounds(angleKnobArea.removeFromTop(knobLabelHeight));
    angleKnob.setBounds(angleKnobArea);
}
