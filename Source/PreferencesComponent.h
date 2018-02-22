/*
  ==============================================================================

    PreferencesComponent.h
    Created: 18 Feb 2018 3:57:48pm
    Author:  ChrisGr

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class PreferencesComponent  : public Component, private ActionBroadcaster
{
public:
    PreferencesComponent()
    {
        tempoAdjustmentRateSlider = new Slider;
        tempoAdjustmentRateSlider->setSize(getWidth(), getHeight());
        tempoAdjustmentRateSlider->setRange (0.0, 1.0, 0.01);
        tempoAdjustmentRateSlider->setValue (tempoAdjustmentRate, dontSendNotification);
        tempoAdjustmentRateSlider->setSliderStyle (Slider::LinearBar);
        addAndMakeVisible(tempoAdjustmentRateSlider);
        
        maxTimeDeltaSlider = new Slider;
        maxTimeDeltaSlider->setSize(getWidth(), getHeight());
        maxTimeDeltaSlider->setRange (0.0, 0.2, 0.01);
        maxTimeDeltaSlider->setValue (tempoAdjustmentRate, dontSendNotification);
        maxTimeDeltaSlider->setSliderStyle (Slider::LinearBar);
        addAndMakeVisible(maxTimeDeltaSlider);
    }
    ~PreferencesComponent()
    {
    }
    void setTempoAdjustmentRate(double tar)
    {
        tempoAdjustmentRate = tar;
        tempoAdjustmentRateSlider->setValue (tempoAdjustmentRate, dontSendNotification);
    }
    void setMaxTimeDelta(double mtd)
    {
        maxTimeDelta = mtd;
        maxTimeDeltaSlider->setValue (maxTimeDelta, dontSendNotification);
    }
    void resized () override
    {
//        Rectangle<int> topHalf = getBounds().removeFromTop(getHeight()/2);
        Rectangle<int> tempoRateArea = getBounds().removeFromTop(getHeight()/2).reduced(10, 40);
        Rectangle<int> maxDeltaArea = getBounds().removeFromBottom(getHeight()/2).reduced(10, 40);
        tempoAdjustmentRateSlider->setBounds(tempoRateArea);
        maxTimeDeltaSlider->setBounds(maxDeltaArea);
//        testLabel->setBounds(getBounds());
    }
    void paint (Graphics& g) override
    {
        g.drawText("Tempo Adjustment Rate", 5, tempoAdjustmentRateSlider->getBounds().getY()-20, 200, 10, Justification::centred);
        g.drawText("Max Rate Change", 5, maxTimeDeltaSlider->getBounds().getY()-20, 200, 10, Justification::centred);
    }
    ScopedPointer<Slider> tempoAdjustmentRateSlider;
    ScopedPointer<Slider> maxTimeDeltaSlider;
    double tempoAdjustmentRate;
    double maxTimeDelta;
//    Label *testLabel;
};
