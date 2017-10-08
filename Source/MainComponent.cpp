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
        audioDeviceManager.initialise (0, 2, savedAudioState, true);
        const StringArray midiInputs (MidiInput::getDevices());
        for (int i = 0; i < midiInputs.size(); ++i)
        {
            if (audioDeviceManager.isMidiInputEnabled (midiInputs[i]))
            {
//                setMidiInput (i);
                std::cout <<"Midi "<<midiInputs[i]<<"\n";
                break;
            }
        }
//        const double sampRate = audioDeviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
        audioDeviceManager.addMidiInputCallback (String(), processor);
        //processor->synthMessageCollectorReset(sampRate);
        addAndMakeVisible(viewerFrame);
        setSize (1100, 300);
        formatManager.addDefaultFormats();
        ScopedPointer<XmlElement> savedPluginList (getAppProperties().getUserSettings()->getXmlValue ("pluginList"));
        
        if (savedPluginList != nullptr)
            knownPluginList.recreateFromXml (*savedPluginList);
        
        pluginSortMethod = (KnownPluginList::SortMethod) getAppProperties().getUserSettings()
        ->getIntValue ("pluginSortMethod", KnownPluginList::sortByManufacturer);
        knownPluginList.addChangeListener (this);
    }
    
MainComponent::~MainComponent()
    {
        knownPluginList.removeChangeListener (this);
        audioDeviceManager.closeAudioDevice();
    }

void MainComponent::paint(Graphics& g)
    {
    }

    //==============================================================================
void MainComponent::changeListenerCallback (ChangeBroadcaster* changed)
    {
        if (changed == &knownPluginList)
        {
            //TODO - notify Mainwindow that menuItemsChanged();
            // save the plugin list every time it gets chnaged, so that if we're scanning
            // and it crashes, we've still saved the previous ones
            ScopedPointer<XmlElement> savedPluginList (knownPluginList.createXml());

            if (savedPluginList != nullptr)
            {
                getAppProperties().getUserSettings()->setValue ("pluginList", savedPluginList);
                getAppProperties().saveIfNeeded();
            }
        }
    }

    void MainComponent::resized()
    {
        Rectangle<int> r (getLocalBounds());
        viewerFrame.setBounds(r);
        viewerFrame.resized();
    }
    
