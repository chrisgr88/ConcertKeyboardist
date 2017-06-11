//
//  MainComponent.cpp
//  PerfectPlayer
//
//
//
#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"

    //==============================================================================
    MainComponent::MainComponent()
    : audioSetupComp (audioDeviceManager, 0, 0, 0, 256, true, true, true, false)
    {
        setLookAndFeel (&lookAndFeel);
        setSize (880, 720);
        audioDeviceManager.initialise (0, 2, 0, true, String(), 0);
        audioDeviceManager.addMidiInputCallback (String(), this);
        audioDeviceManager.addAudioCallback (this);
        
        addAndMakeVisible (audioSetupComp);
        
        //        synth.setVoiceStealingEnabled (false);
    }
    
    MainComponent::~MainComponent()
    {
        audioDeviceManager.removeMidiInputCallback (String(), this);
    }
    
    //==============================================================================
    void MainComponent::resized()
    {
        const float audioSetupCompRelativeWidth = 0.55f;
        
        Rectangle<int> r (getLocalBounds());
        
        //        visualiserViewport.setBounds (r.removeFromBottom (visualiserCompHeight));
        
        audioSetupComp.setBounds (r.removeFromLeft (proportionOfWidth (audioSetupCompRelativeWidth)));
    }
    
    //==============================================================================
    void MainComponent::audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples)
    {
        AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
        buffer.clear();
        
        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
        //        synth.renderNextBlock (buffer, incomingMidi, 0, numSamples);
    }
    
    void MainComponent::audioDeviceAboutToStart (AudioIODevice* device)
    {
        const double sampleRate = device->getCurrentSampleRate();
        midiCollector.reset (sampleRate);
        //        synth.setCurrentPlaybackSampleRate (sampleRate);
    }
    
    void MainComponent::audioDeviceStopped()
    {
    }

    //==============================================================================
    void MainComponent::handleIncomingMidiMessage (MidiInput* /*source*/,
                                    const MidiMessage& message)
    {
        midiCollector.addMessageToQueue (message);
    }


