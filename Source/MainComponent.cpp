/*
  ==============================================================================

    MainComponent.cpp
    Created: 16 Jan 2017 3:00:15pm
    Author:  ChrisGr
Foo
  ==============================================================================
*/

#include "MainComponent.h"
//#include "PluginProcessor.h"

ApplicationCommandManager& getCommandManager();
ApplicationProperties& getAppProperties();

MainComponent::MainComponent(MIDIProcessor *p) :
    viewerFrame(p),
    scrollingNoteViewer(p),
    processor(p)
    {
        setLookAndFeel (&lookAndFeel);
        processor->reset(44.1);
        processor->setMidiDestination (MIDIProcessor::MidiDestination::output);
        ScopedPointer<XmlElement> savedAudioState (getAppProperties().getUserSettings()
                                                   ->getXmlValue ("audioDeviceState"));
        audioDeviceManager.initialise (256, 256, savedAudioState, true);
        audioDeviceManager.addMidiInputCallback (String(), this);
        audioDeviceManager.addAudioCallback (this);
        addAndMakeVisible(viewerFrame);
        setSize (1100, 300);
        for (int i=0;i<128;i++)
        {
            synth.addVoice(new::sfzero::Voice());
        }
        AudioFormatManager formatManager;
//        formatManager.registerBasicFormats	();
//        auto sfzFile = File ("/Users/chrisgr/Downloads/PatchArena_Marimba/PatchArena_marimba.sfz");
//        auto sfzFile = File ("/Users/chrisgr/Downloads/City Piano-SFZ/City Piano.sfz");
        
//        auto sound = new sfzero::Sound(sfzFile);
//        sound->loadRegions();
//        sound->loadSamples(&formatManager);
//        synth.clearSounds();
//        synth.addSound(sound);
    }
    
MainComponent::~MainComponent()
    {
        audioDeviceManager.removeMidiInputCallback (String(), this);
    }

    void MainComponent::paint(Graphics& g)
    {
    }


    
    //==============================================================================
    void MainComponent::resized()
    {
//        const float audioSetupCompRelativeHeight = 0.4f;
        
        Rectangle<int> r (getLocalBounds());
        
//        audioSetupComp.setBounds (r.removeFromTop (proportionOfHeight (audioSetupCompRelativeHeight)));
        viewerFrame.setBounds(r);
        viewerFrame.resized();
    }
    
    //==============================================================================
    void MainComponent::audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples)
    {
        AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
        buffer.clear();
        
        MidiBuffer incomingMidi;
        processor->synthMessageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
//        synth.renderNextBlock (buffer, incomingMidi, 0, numSamples);
    }
    
    void MainComponent::audioDeviceAboutToStart (AudioIODevice* device)
    {
        const double sampleRate = device->getCurrentSampleRate();
//        processor->reset (sampleRate);
        processor->synthMessageCollectorReset(sampleRate);
        synth.setCurrentPlaybackSampleRate (sampleRate);
    }
    
    void MainComponent::audioDeviceStopped()
    {
    }

    //==============================================================================
    void MainComponent::handleIncomingMidiMessage (MidiInput* /*source*/,
                                    const MidiMessage& message)
    {
//        visualiserInstrument.processNextMidiEvent (message);
        processor->addMessageToQueue (message);
    }



