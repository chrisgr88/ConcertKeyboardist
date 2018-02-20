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

//        testLabel = new Label;
//        testLabel->setSize(getWidth(), getHeight());
//        testLabel->setText("Press Me", dontSendNotification);
//        addAndMakeVisible(testLabel);
    }
    ~PreferencesComponent()
    {
    }
    void setTempoAdjustmentRate(double tar)
    {
        tempoAdjustmentRate = tar;
        tempoAdjustmentRateSlider->setValue (tempoAdjustmentRate, dontSendNotification);
    }
    void resized () override
    {
        Rectangle<int> b = getBounds().reduced(10, 40);
        tempoAdjustmentRateSlider->setBounds(b);
//        testLabel->setBounds(getBounds());
    }
    void paint (Graphics& g) override
    {
        g.drawText("Tempo Adjustment Rate", 5, 15, 200, 10, Justification::centred);
    }
    ScopedPointer<Slider> tempoAdjustmentRateSlider;
    double tempoAdjustmentRate = 0.77;
//    Label *testLabel;
};
