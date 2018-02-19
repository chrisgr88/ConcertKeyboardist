/*
  ==============================================================================

    PreferencesComponent.h
    Created: 18 Feb 2018 3:57:48pm
    Author:  ChrisGr

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class PreferencesComponent  : public Component, private ChangeListener
{
public:
    PreferencesComponent()
    {
        tempoAdjustmentRate = new Slider;
        tempoAdjustmentRate->setSize(getWidth(), getHeight());
        tempoAdjustmentRate->setRange (0, 10, 1);
        tempoAdjustmentRate->setValue (5, dontSendNotification);
        tempoAdjustmentRate->setSliderStyle (Slider::LinearBar);
        addAndMakeVisible(tempoAdjustmentRate);

//        testLabel = new Label;
//        testLabel->setSize(getWidth(), getHeight());
//        testLabel->setText("Press Me", dontSendNotification);
//        addAndMakeVisible(testLabel);
    }
    ~PreferencesComponent()
    {
    }
    void changeListenerCallback (ChangeBroadcaster*) override
    {
        
    }
    void resized () override
    {
        Rectangle<int> b = getBounds().reduced(10, 40);
        tempoAdjustmentRate->setBounds(b);
//        testLabel->setBounds(getBounds());
    }
    void paint (Graphics& g) override
    {
        //        g.drawText("Hello", 30, 50, 50, 30, Justification::centred);
    }
    ScopedPointer<Slider> tempoAdjustmentRate;
//    Label *testLabel;
};
