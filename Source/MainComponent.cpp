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
        thePlugin = nullptr;
        processor->sequenceObject.addActionListener(this);
        setLookAndFeel (&lookAndFeel);
        processor->reset(44.1);
        ScopedPointer<XmlElement> savedAudioState (getAppProperties().getUserSettings()
                                                   ->getXmlValue ("audioDeviceState"));
        audioDeviceManager.initialise (0, 2, savedAudioState, true);
        AudioProcessorGraph::AudioGraphIOProcessor midiInNode(AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
        AudioProcessorGraph::AudioGraphIOProcessor audioOutNode(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
        std::cout << "Audio Device Initialized " << audioDeviceManager.getCurrentAudioDevice()->getName() << "\n";
//        graph.addNode(&midiInNode,0);
//        graph.addNode(&audioOutNode,4);
//        graph.call
        
        const StringArray midiInputs (MidiInput::getDevices());
        for (int i = 0; i < midiInputs.size(); ++i)
        {
            if (audioDeviceManager.isMidiInputEnabled (midiInputs[i]))
            {
//                setMidiInput (i);
                processor->midiOutEnabled = true;
                std::cout <<"Midi "<<midiInputs[i]<<"\n";
                break;
            }
        }
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
        setLookAndFeel(nullptr);
        audioDeviceManager.closeAudioDevice();
    }

void MainComponent::paint(Graphics& g)
    {
    }

void MainComponent::actionListenerCallback (const String& message)
{
    //        std::cout <<"actionListenerCallback"<< message << "\n";
//    if (message.upToFirstOccurrenceOf(":",false,true) == "loadPlugin")
//    {
////        String pluginId = String(message.fromFirstOccurrenceOf(":", false, true));
////        const PluginDescription* desc = knownPluginList.getTypeForIdentifierString(pluginId);
////        if (desc != NULL)
////        {
////            loadPlugin(desc);
////            std::cout << "loadedPlugin in MainComponent: id is  " << pluginId << "\n";
//        std::cout << "here \n";
////        }
//    }
//    else
        if (message.upToFirstOccurrenceOf(":",false,true) == "pluginStateChange")
    {
        String pluginStateB64 = String(message.fromFirstOccurrenceOf(":", false, true));
        MemoryBlock m;
        m.fromBase64Encoding(pluginStateB64);
//        MD5 md = MD5(processor->sequenceObject.pluginState);
//        String checksum = md.toHexString();
//        std::cout <<"Loaded sysex plugState  checksum "<<checksum
//        <<" size "<<processor->sequenceObject.pluginState.getSize()<<"\n";
//        if (processor->sequenceObject.pluginState.getSize()>0)
//#include "TargetConditionals.h"
#if !(TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE)
        if (thePlugin!=nullptr)
            thePlugin->setStateInformation(m.getData(), (int)m.getSize());
#endif
    }
};

 void MainComponent::loadPlugin (String  pluginId )
{
    if (pluginId.length() > 0)
    {
        const PluginDescription* desc = knownPluginList.getTypeForIdentifierString(pluginId);
        if (desc != NULL)
            loadPlugin(desc);
    }
}
void MainComponent::loadPlugin (const PluginDescription* pluginDescription)
{
    std::cout << "Loading plugin "<<pluginDescription->descriptiveName <<"\n";
    juce::AudioIODevice *curDevice =  audioDeviceManager.getCurrentAudioDevice();
    if (curDevice==nullptr)
        std::cout <<"No audio device \n";
        
    const double sampRate = curDevice->getCurrentSampleRate();
    const double bufSz = audioDeviceManager.getCurrentAudioDevice()->getCurrentBufferSizeSamples();
    String errorMsg;
    processor->sequenceObject.thePlugin = nullptr;
    if (thePlugin)
    {
        thePlugin->suspendProcessing(true);
        audioDeviceManager.removeAudioCallback(&thePlayer);
//        thePlayer.setProcessor(nullptr);
//        thePlugin = nullptr;
    }
    
    AudioPluginInstance *pPlugin = formatManager.createPluginInstance(*pluginDescription, sampRate,bufSz,errorMsg);
    thePlugin = pPlugin;
    processor->sequenceObject.thePlugin = pPlugin;
    if (thePlugin)
    {
        std::cout << "Loaded plugin \n";
        processor->pluginEnabled = true;
    }
    else
        std::cout << "Plugin error "<<errorMsg<<"\n";
    
    //        graph.addNode(thePlugin,2);
    //        AudioProcessorGraph::AudioGraphIOProcessor midiInNode(AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
    //        AudioProcessorGraph::AudioGraphIOProcessor audioOutNode(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
    //        graph.addNode(&midiInNode,0);
    //        graph.addNode(&audioOutNode,4);
    //        graph.addConnection(0, 0, 2, 0);
    //        graph.addConnection(0, 2, 4, 0);
    thePlayer.setProcessor(&graph);
    thePlayer.setProcessor(thePlugin);
    audioDeviceManager.addAudioCallback (&thePlayer);
    thePlugin->suspendProcessing(false);
    MidiMessageCollector &mmc = thePlayer.getMidiMessageCollector();
    processor->pluginMessageCollector = &mmc;
    thePlugin->setPlayHead(processor);
    processor->pluginMessageCollectorReset(sampRate);
    thePlayer.getMidiMessageCollector().reset(sampRate);
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
    
