/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider 
{
    CustomRotarySlider() : juce::Slider(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {

    }
};

struct ResponseCurveComponent : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    ResponseCurveComponent(SimpleEQ2AudioProcessor&);
    ~ResponseCurveComponent();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { };
    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    SimpleEQ2AudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };

    MonoChain monoChain;
};

//==============================================================================
/**
*/
class SimpleEQ2AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleEQ2AudioProcessorEditor (SimpleEQ2AudioProcessor&);
    ~SimpleEQ2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQ2AudioProcessor& audioProcessor;

    CustomRotarySlider peakFreqSlider,
        peakGainSlider,
        peakQualitySlider,
        lowCutFreqSlider,
        hiCutFreqSlider,
        lowCutSlopeSlider,
        hiCutSlopeSlider;

    ResponseCurveComponent responseCurveComponent;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment peakFreqSliderAttachment,
        peakGainSliderAttachment,
        peakQualitySliderAttachment,
        lowCutFreqSliderAttachment,
        hiCutFreqSliderAttachment,
        lowCutSlopeSliderAttachment,
        hiCutSlopeSliderAttachment;

    std::vector<juce::Component*> getComps();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQ2AudioProcessorEditor)
};
