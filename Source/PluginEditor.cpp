/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQ2AudioProcessorEditor::SimpleEQ2AudioProcessorEditor (SimpleEQ2AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    hiCutFreqSliderAttachment(audioProcessor.apvts, "HiCut Freq", hiCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    hiCutSlopeSliderAttachment(audioProcessor.apvts, "HiCut Slope", hiCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }
    startTimer(60);
    setSize (600, 400);
}

SimpleEQ2AudioProcessorEditor::~SimpleEQ2AudioProcessorEditor()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->removeListener(this);
    }
}

//==============================================================================
void SimpleEQ2AudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto w = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& hicut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();
    std::vector<double> mags;

    mags.resize(w);

    for (int i = 0; i < w; ++i) 
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::Peak>())
        {
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        if(!lowcut.isBypassed<0>()){
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<1>()){
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<2>()){
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<3>()) {
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        if (!hicut.isBypassed<0>()) {
            mag *= hicut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!hicut.isBypassed<1>()) {
            mag *= hicut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!hicut.isBypassed<2>()) {
            mag *= hicut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!hicut.isBypassed<3>()) {
            mag *= hicut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i) {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

void SimpleEQ2AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto hiCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);


    hiCutFreqSlider.setBounds(hiCutArea.removeFromTop(hiCutArea.getHeight() * 0.5));
    hiCutSlopeSlider.setBounds(hiCutArea);

    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}

void SimpleEQ2AudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue) 
{
    parametersChanged.set(true);
}

void SimpleEQ2AudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        DBG("params changed!");
        // update the mono chain
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
        // signal a repaint
        repaint();
    }
}

std::vector<juce::Component*> SimpleEQ2AudioProcessorEditor::getComps()
{
    return 
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &hiCutFreqSlider,
        &lowCutSlopeSlider,
        &hiCutSlopeSlider
    };
}
