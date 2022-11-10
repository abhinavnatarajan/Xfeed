/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
XfeedAudioProcessor::XfeedAudioProcessor(): wetLevel(1.0f), dryLevel(0.0f),
parameters(juce::AudioProcessorValueTreeState(*this, nullptr, juce::Identifier("Xfeed"), 
    {
        std::make_unique<juce::AudioParameterFloat>(
        "gaindB", // parameterID
        "Gain", // parameter name
        static_cast<float>(minGaindB),   // minimum value
        static_cast<float>(maxGaindB),   // maximum value
        static_cast<float>(defaultGaindB)), // default value
        std::make_unique<juce::AudioParameterFloat>(
        "angle", // parameterID
        "Angle", // parameter name
        static_cast<float>(minAngle),   // minimum value
        static_cast<float>(maxAngle),   // maximum value
        static_cast<float>(defaultAngle)), // default value
        std::make_unique<juce::AudioParameterBool>(
        "bypass", // parameterID
        "Bypass", // parameter name
        false) // default value
    }))
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    gaindB = static_cast<juce::AudioParameterFloat*>(parameters.getParameter("gaindB"));
    angle = static_cast<juce::AudioParameterFloat*>(parameters.getParameter("angle"));
    bypass = static_cast<juce::AudioParameterBool*>(parameters.getParameter("bypass"));
    setLatencySamples(0);   
}

XfeedAudioProcessor::~XfeedAudioProcessor()
{
}

//==============================================================================
const juce::String XfeedAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool XfeedAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool XfeedAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool XfeedAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double XfeedAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int XfeedAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int XfeedAudioProcessor::getCurrentProgram()
{
    return 0;
}

void XfeedAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String XfeedAudioProcessor::getProgramName (int index)
{
    return {};
}

void XfeedAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void XfeedAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need.
    updateFilterCoeffs(static_cast<float>(sampleRate));
    auto spec = juce::dsp::ProcessSpec(sampleRate, samplesPerBlock, 2);
    for (auto channel = 0; channel < 2; channel++) {
        highShelf[channel].prepare(spec);
        lowShelf[channel].prepare(spec);
        allPass[channel].prepare(spec);
        highShelf[channel].reset();
        lowShelf[channel].reset();
        allPass[channel].reset();
    }
    wetLevel.reset(sampleRate, 0.25f);
    dryLevel.reset(sampleRate, 0.25f);
    if (*bypass) {
        wetLevel.setCurrentAndTargetValue(0.0f);
        dryLevel.setCurrentAndTargetValue(1.0f);
    }
    else {
        wetLevel.setCurrentAndTargetValue(1.0f);
        dryLevel.setCurrentAndTargetValue(0.0f);
    }
    mainBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);
    mainBlock = juce::dsp::AudioBlock<float>(mainBuffer);
    auxBuffer = juce::AudioBuffer<float>(2, samplesPerBlock);
    auxBlock = juce::dsp::AudioBlock<float>(auxBuffer);
}

void XfeedAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    for (auto channel = 0; channel < 2; channel++) {
        highShelf[channel].reset();
        lowShelf[channel].reset();
        allPass[channel].reset();
        mainBlock = juce::dsp::AudioBlock<float>();
        mainBuffer = juce::AudioBuffer<float>();
        auxBlock = juce::dsp::AudioBlock<float>();
        auxBuffer = juce::AudioBuffer<float>();
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool XfeedAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void XfeedAudioProcessor::processBlock (juce::AudioBuffer<float>& ioBuffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto ioBlock = juce::dsp::AudioBlock<float>(ioBuffer);
    auto numSamples = ioBuffer.getNumSamples();
    mainBlock.copyFrom(ioBlock);
    auxBuffer.copyFrom(0, 0, ioBuffer, 1, 0, numSamples); // copy in-R to aux-L
    auxBuffer.copyFrom(1, 0, ioBuffer, 0, 0, numSamples); // copy in-L to aux-R
    for (auto channel = 0; channel < 2; channel++) {
        auto mainChannelBlock = mainBlock.getSingleChannelBlock(channel);
        auto auxChannelBlock = auxBlock.getSingleChannelBlock(channel);
        auto mainContext = juce::dsp::ProcessContextReplacing<float> (mainChannelBlock);
        auto auxContext = juce::dsp::ProcessContextReplacing<float> (auxChannelBlock);
        highShelf[channel].process(mainContext);
        lowShelf[channel].process(auxContext);
        allPass[channel].process(auxContext);
    }
    mainBlock += auxBlock;
    mainBlock *= gain;
    ioBlock.replaceWithSumOf(mainBlock.multiplyBy(wetLevel),
        ioBlock.multiplyBy(dryLevel));
}

//==============================================================================
bool XfeedAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* XfeedAudioProcessor::createEditor()
{
    return new XfeedAudioProcessorEditor (*this);
}

//==============================================================================
void XfeedAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xml(parameters.copyState().createXml());
    copyXmlToBinary(*xml, destData);
}

void XfeedAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
        
}

float XfeedAudioProcessor::getITD() const
{
    constexpr float deg2rad = juce::MathConstants<float>::pi / 180.0f, invvel = 1.0f / 340.0f, headwidth = 0.15f, dist = 1.0f;
    float a = *angle;
    float theta = a * deg2rad;
    float sinthetaby2 = std::sin(theta * 0.5f);
    float d2 = std::sqrt(dist * dist + headwidth * headwidth * 0.25f + headwidth * dist * sinthetaby2);
    float d1 = std::sqrt(dist * dist + headwidth * headwidth * 0.25f - headwidth * dist * sinthetaby2);
    return (d2 - d1) * invvel;
}

float XfeedAudioProcessor::biquadPhaseDelay(const juce::dsp::IIR::Coefficients<float>::Ptr coefs, float sampleRate) const
{
    auto temp = coefs.getObject()->getRawCoefficients();
    auto b0 = temp[0], b1 = temp[1], b2 = temp[2], a0 = temp[3], a1 = temp[4], a2 = temp[5];
    auto alpha = b0 * a1 + b1 * a2 - a0 * b1 - a1 * b2;
    auto beta = b0 * a2 - a0 * b2;
    auto gamma = a0 * b0 + a1 * b1 + a2 * b2;
    auto delta = a1 * b0 + a2 * b1 + a0 * b1 + a1 * b2;
    auto eps = a2 * b0 + a0 * b2;
    auto res = -(alpha + 2.0f * beta) / (gamma + delta + eps);
    return res / sampleRate;
}

juce::dsp::IIR::Coefficients<float>::Ptr XfeedAudioProcessor::makeLowShelfCoeffs(float sampleRate, float f0, float shelfgain, float S, float postgain) const
{
    // Basic biquad low shelf with coefficients taken from RBJ's cookbook
    constexpr float twopi = juce::MathConstants<float>::pi * 2.0f;
    float A = std::pow(10.0f, (shelfgain / 40.0f));
    float w0 = twopi * f0 / sampleRate;
    float cosw0 = std::cos(w0);
    float Qinv = std::sqrt((A + 1.0f / A) * (1.0f / S - 1.0f) + 2.0f);
    float twortAa = std::sqrt(A) * std::sin(w0) * Qinv;
    float Ap1 = A + 1.0f, Am1 = A - 1.0f;
    float Ap1cosw0 = Ap1 * cosw0, Am1cosw0 = Am1 * cosw0;
    float b0 = A * (Ap1 - Am1 * cosw0 + twortAa);
    float b1 = 2.0f * A * (Am1 - Ap1cosw0);
    float b2 = A * (Ap1 - Am1cosw0 - twortAa);
    float a0 = Ap1 + Am1cosw0 + twortAa;
    float a1 = -2.0f * (Am1 + Ap1cosw0);
    float a2 = Ap1 + Am1cosw0 - twortAa;
    float filtgain = juce::Decibels::decibelsToGain(postgain);
    b0 *= filtgain; b1 *= filtgain; b2 *= filtgain;
    juce::dsp::IIR::Coefficients<float>::Ptr coefs(new juce::dsp::IIR::Coefficients<float>(b0, b1, b2, a0, a1, a2));
    return coefs;
}

juce::dsp::IIR::Coefficients<float>::Ptr XfeedAudioProcessor::makeHighShelfCoeffs(float sampleRate, float f0, float shelfgain, float S, float postgain) const
{
    // Basic biquad high shelf with coefficients taken from RBJ's cookbook
    constexpr float twopi = juce::MathConstants<float>::pi * 2.0f;
    float A = std::pow(10.0f, (shelfgain / 40.0f));
    float w0 = twopi * f0 / sampleRate;
    float cosw0 = std::cos(w0);
    float Qinv = std::sqrt((A + 1.0f / A) * (1.0f / S - 1.0f) + 2.0f);
    float twortAa = std::sqrt(A) * std::sin(w0) * Qinv;
    float Ap1 = A + 1.0f, Am1 = A - 1.0f;
    float Ap1cosw0 = Ap1 * cosw0, Am1cosw0 = Am1 * cosw0;
    float b0 = A * (Ap1 + Am1cosw0 + twortAa);
    float b1 = -2.0f * A * (Am1 + Ap1cosw0);
    float b2 = A * (Ap1 + Am1cosw0 - twortAa);
    float a0 = Ap1 - Am1cosw0 + twortAa;
    float a1 = 2.0f * (Am1 - Ap1cosw0);
    float a2 = Ap1 - Am1cosw0 - twortAa;
    float filtgain = juce::Decibels::decibelsToGain(postgain);
    b0 *= filtgain; b1 *= filtgain; b2 *= filtgain;
    juce::dsp::IIR::Coefficients<float>::Ptr coefs(new juce::dsp::IIR::Coefficients<float>(b0, b1, b2, a0, a1, a2));
    return coefs;
}

juce::dsp::IIR::Coefficients<float>::Ptr XfeedAudioProcessor::makeAllPassCoeffs(float sampleRate, float delaytime) const
{
    // Basic first order all pass filter
    float a = (1.0f - delaytime * 0.5f * sampleRate) / (1.0f + delaytime * 0.5f * sampleRate);
    juce::dsp::IIR::Coefficients<float>::Ptr coefs(new juce::dsp::IIR::Coefficients<float>(a * a, 2.0f * a, 1.0f, 1.0f, 2.0f * a, a * a));
    return coefs;
}

void XfeedAudioProcessor::updateFilterCoeffs(float sampleRate)
{
    auto ITD = getITD();
    auto Slo = 0.4f, gainlo = 9.0f, f0lo = std::min(1.0f / ITD * 0.5f, 2000.0f);
    auto Shi = 0.4f, gainhi = 3.0f, f0hi = f0lo / 3.0f;
    highShelfCoeffs = makeHighShelfCoeffs(sampleRate, f0hi, gainhi, Shi, - gainhi);
    lowShelfCoeffs = makeLowShelfCoeffs(sampleRate, f0lo, gainlo, Slo, - gainhi - gainlo);
    auto lowShelfDelay = biquadPhaseDelay(lowShelfCoeffs, sampleRate), highShelfDelay = biquadPhaseDelay(highShelfCoeffs, sampleRate);
    auto delaytime = std::max(0.0f, ITD - (lowShelfDelay));
    allPassCoeffs = makeAllPassCoeffs(sampleRate, delaytime);
    for (auto channel = 0; channel < 2; channel++) {
        highShelf[channel].coefficients = highShelfCoeffs;
        lowShelf[channel].coefficients = lowShelfCoeffs;
        allPass[channel].coefficients = allPassCoeffs;
    }
    gain = juce::Decibels::decibelsToGain(static_cast<float>(*gaindB));
}

void XfeedAudioProcessor::updateBypassRamps() {
    if (*bypass) {
        wetLevel.setTargetValue(0.0f);
        dryLevel.setTargetValue(1.0f);
    }
    else {
        wetLevel.setTargetValue(1.0f);
        dryLevel.setTargetValue(0.0f);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new XfeedAudioProcessor();
}
