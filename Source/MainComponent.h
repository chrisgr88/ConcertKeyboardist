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
    public AudioIODeviceCallback,
//    private MidiInputCallback,
    public ChangeListener,
    public ActionListener,
    public ActionBroadcaster
{
public:
    //==============================================================================
    MainComponent(MIDIProcessor *p);
    ~MainComponent();
    
    ViewerFrame* getViewerFrame()
    {
        return &viewerFrame;
    };
    
    void actionListenerCallback (const String& message) override;
    
    void paint(Graphics& g) override;

    //==============================================================================
    void resized() override;
    
    void audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override;
    
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    
    void audioDeviceStopped() override;
    
    void setMidiInput (int index);
    
    void loadPlugin (String  pluginId);
    void loadPlugin (const PluginDescription* pluginDescription);
    bool loadSFZero ();
    void unLoadPlugin ();
    
    AudioPluginFormatManager formatManager;
    KnownPluginList knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;
    AudioDeviceManager audioDeviceManager;
    AudioProcessorPlayer thePlayer;
    ScopedPointer<AudioPluginInstance> thePlugin;

//    AudioProcessorGraph graph;
    
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
    sfzero::Synth sfZeroSynth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
