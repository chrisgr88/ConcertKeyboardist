/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/


#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED
#include "../JuceLibraryCode/JuceHeader.h"
#include "ViewerFrame.h"
#include "MIDIProcessor.h"

class MainComponent :
    public Component,
//    private AudioIODeviceCallback,
//    private MidiInputCallback,
    public ChangeListener
{
public:
    //==============================================================================
    MainComponent(MIDIProcessor *p);
    ~MainComponent();
    
    ViewerFrame* getViewerFrame()
    {
        return &viewerFrame;
    };
    
    void paint(Graphics& g) override;

    //==============================================================================
    void resized() override;

    //==============================================================================
//    void audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
//                                float** outputChannelData, int numOutputChannels,
//                                int numSamples) override;
//
//    void audioDeviceAboutToStart (AudioIODevice* device) override;
//
//    void audioDeviceStopped() override;
    
    void setMidiInput (int index);
    
    void loadPlugin(const PluginDescription* pPID)
    {
        std::cout << "Loading plugin "<<pPID->descriptiveName <<"\n";
        String errorMsg;
        ScopedPointer<AudioPluginInstance> loaded = formatManager.createPluginInstance(*pPID, 500,500,errorMsg);
        if (loaded)
        {
            std::cout << "Loaded plugin \n";
        }
        audioDeviceManager.closeAudioDevice();
        thePlugin = formatManager.createPluginInstance(*pPID, 500,500,errorMsg);
        thePlayer.setProcessor(thePlugin); //This is a midi input callback for the processor
        MidiMessageCollector &mmc = thePlayer.getMidiMessageCollector();
        processor->pluginMessageCollector = &mmc;
        thePlugin->setPlayHead(processor);
        audioDeviceManager.addAudioCallback (&thePlayer);
            audioDeviceManager.addMidiInputCallback (String(), &thePlayer);

        ScopedPointer<XmlElement> savedAudioState (getAppProperties().getUserSettings()
                                                   ->getXmlValue ("audioDeviceState"));
        audioDeviceManager.restartLastAudioDevice();
        const double sampRate = audioDeviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
        processor->synthMessageCollectorReset(sampRate);
        thePlayer.getMidiMessageCollector().reset(sampRate);
        audioDeviceManager.initialise (0, 2, savedAudioState, true);
    }
    AudioPluginFormatManager formatManager;
    KnownPluginList knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;
    AudioDeviceManager audioDeviceManager;
    AudioProcessorPlayer thePlayer;
    ScopedPointer<AudioPluginInstance> thePlugin;
    int lastInputIndex = 0;
    
    void changeListenerCallback (ChangeBroadcaster* changed) override;
//    {
//        if (changed == &knownPluginList)
//        {
//            menuItemsChanged();
//            
//            // save the plugin list every time it gets chnaged, so that if we're scanning
//            // and it crashes, we've still saved the previous ones
//            ScopedPointer<XmlElement> savedPluginList (knownPluginList.createXml());
//            
//            if (savedPluginList != nullptr)
//            {
//                getAppProperties().getUserSettings()->setValue ("pluginList", savedPluginList);
//                getAppProperties().saveIfNeeded();
//            }
//        }
//    }
    
private:
    //==============================================================================
//    void handleIncomingMidiMessage (MidiInput* /*source*/,
//                                    const MidiMessage& message) override;
    //==============================================================================
    LookAndFeel_V3 lookAndFeel;

//    AudioDeviceSelectorComponent audioSetupComp;
    ViewerFrame viewerFrame;
    ScrollingNoteViewer scrollingNoteViewer;

    MIDIProcessor *processor; //This was previously "PluginProcessor"
    double sampleRate;
    int blockSize;
//    sfzero::Synth synth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
