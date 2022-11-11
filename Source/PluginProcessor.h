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
class XfeedAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    XfeedAudioProcessor();
    ~XfeedAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

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

    // these are double rather than float because they are used by the GUI knobs, which accepts only double
    constexpr static double maxGaindB = 12.0, minGaindB = -12.0, defaultGaindB = -1.0;
    constexpr static double maxAngle = 90.0, minAngle = 0.0, defaultAngle = 60.0;
    constexpr static double maxDist = 5.0, minDist = 0.25, defaultDist = 1.0;
    juce::AudioParameterFloat* gaindB;
    juce::AudioParameterFloat* angle;
    juce::AudioParameterBool* bypass;
    
    void updateFilterCoeffs(float sampleRate);
    void updateBypassRamp();

private:
    float gain;
    std::array< juce::dsp::IIR::Filter<float>, 2 > highShelf, lowShelf, allPass;
    juce::dsp::IIR::Coefficients<float>::Ptr highShelfCoeffs, lowShelfCoeffs, allPassCoeffs;
    juce::dsp::AudioBlock<float> mainBlock, auxBlock;
    juce::AudioBuffer<float> mainBuffer, auxBuffer;
    juce::SmoothedValue< float, juce::ValueSmoothingTypes::Linear > wetLevel, dryLevel;
    juce::AudioProcessorValueTreeState parameters;
    
    float getITD() const;
    float biquadPhaseDelay(const juce::dsp::IIR::Coefficients<float>::Ptr coefs, float sampleRate) const;
    juce::dsp::IIR::Coefficients<float>::Ptr makeLowShelfCoeffs(float sampleRate, float f0, float shelfgain, float S, float postgain) const;
    juce::dsp::IIR::Coefficients<float>::Ptr makeHighShelfCoeffs(float sampleRate, float f0, float shelfgain, float S, float postgain) const;
    juce::dsp::IIR::Coefficients<float>::Ptr makeAllPassCoeffs(float sampleRate, float delaytime) const;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XfeedAudioProcessor)
};
