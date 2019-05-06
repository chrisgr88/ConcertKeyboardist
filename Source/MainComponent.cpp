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

MainComponent::MainComponent(MIDIProcessor *p) : thePlayer(false),
    viewerFrame(p),
    scrollingNoteViewer(p),
    processor(p)
    {
        thePlugin = nullptr;
        processor->sequenceObject.addActionListener(this);
        setLookAndFeel (&lookAndFeel);
        processor->reset(44.1);
        
//        std::cout << "audioDeviceState " << getAppProperties().getUserSettings()->getXmlValue ("audioDeviceState") <<"\n";
        ScopedPointer<XmlElement> savedAudioState (getAppProperties().getUserSettings()
                                                   ->getXmlValue ("audioDeviceState"));
        
        if (savedAudioState != nullptr)
            audioDeviceManager.initialise (0, 2, savedAudioState, true);
        else
            audioDeviceManager.initialise (0, 2, nullptr, true);
        
//        AudioProcessorGraph::AudioGraphIOProcessor midiInNode(AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
//        AudioProcessorGraph::AudioGraphIOProcessor audioOutNode(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
//        std::cout << "Audio Device Initialized " << audioDeviceManager.getCurrentAudioDevice()->getName() << "\n";
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
#if JUCE_MAC || JUCE_IOS
        processor->midiOutEnabled = true; //For virtual midi port
#endif
        audioDeviceManager.addMidiInputCallback (String(), processor);
        addAndMakeVisible(viewerFrame);
        setSize (1100, 300);
#if JUCE_IOS
        formatManager.addFormat(new AudioUnitPluginFormat());
#else
        formatManager.addDefaultFormats();
#endif
        ScopedPointer<XmlElement> savedPluginList (getAppProperties().getUserSettings()->getXmlValue ("pluginList"));
        
        if (savedPluginList != nullptr)
            knownPluginList.recreateFromXml (*savedPluginList);
        
        pluginSortMethod = (KnownPluginList::SortMethod) getAppProperties().getUserSettings()
        ->getIntValue ("pluginSortMethod", KnownPluginList::sortByManufacturer);
        knownPluginList.addChangeListener (this);
        
        processor->defaultMidiOutput  = nullptr;
        if (audioDeviceManager.getDefaultMidiOutput() != nullptr)
        {
            std::cout << "midi out " <<audioDeviceManager.getDefaultMidiOutput()-> getName() << "\n";
            processor->defaultMidiOutput = audioDeviceManager.getDefaultMidiOutput();
        }
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

#if !(TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE)
        if (thePlugin!=nullptr)
            thePlugin->setStateInformation(m.getData(), (int)m.getSize());
#endif
    }
};

//bool MainComponent::loadSFZero ()
//{
//    std::cout << "Loading SFZero " <<"\n";
//    juce::AudioIODevice *curDevice =  audioDeviceManager.getCurrentAudioDevice();
//    if (curDevice==nullptr)
//    {
//        std::cout <<"No audio device so restart audioDeviceManager\n";
//        audioDeviceManager.restartLastAudioDevice();
//    }
//    curDevice =  audioDeviceManager.getCurrentAudioDevice();
//    if (curDevice==nullptr)
//    {
//        std::cout <<"Still No audio device \n";
//        return false;
//    }
//    bool result;
//    AudioFormatManager formatManager;
//    formatManager.registerBasicFormats    ();
////    auto sfzFile = File ("/Users/chrisgr/Downloads/PatchArena_Marimba/PatchArena_marimba.sfz");
////    auto sfzFile = File ("/Users/chrisgr/Downloads/City Piano-SFZ/City Piano.sfz");
//    bool exists = sfzFile.exists();
//    if (exists)
//    {
////        auto sound = new sfzero::Sound(sfzFile);
////        sound->loadRegions();
////        sound->loadSamples(&formatManager);
////        sfZeroSynth.clearSounds();
////        for (int i = 0; i < 128; ++i)
////        {
////            sfZeroSynth.addVoice(new sfzero::Voice());
////        }
////        sfZeroSynth.addSound(sound);
////        if (sfZeroSynth.getNumSounds()==0)
////            result = false;
////        else
////            result = true;
////        juce::AudioIODevice *curDevice =  audioDeviceManager.getCurrentAudioDevice();
////        const double sampleRate = curDevice->getCurrentSampleRate();
////        processor->sampleRate = sampleRate;
////        processor->synthMessageCollector.reset(sampleRate);
////        sfZeroSynth.setCurrentPlaybackSampleRate (sampleRate);
////        audioDeviceManager.addAudioCallback(this);
//    }
//    else
//        result = false;
//    std::cout <<"Loaded loadSFZero "<< result<<"\n";
//    return result;
//}

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
    {
        std::cout <<"No audio device so restart audioDeviceManager\n";
        audioDeviceManager.restartLastAudioDevice();
    }
    curDevice =  audioDeviceManager.getCurrentAudioDevice();
    if (curDevice==nullptr)
    {
        std::cout <<"Still No audio device \n";
        return;
    }
    
    const double sampRate = curDevice->getCurrentSampleRate();
    const int bufSz = audioDeviceManager.getCurrentAudioDevice()->getCurrentBufferSizeSamples();
    String errorMsg;
    processor->sequenceObject.pThePlugin = nullptr;
    if (thePlugin)
    {
        thePlugin->suspendProcessing(true);
        audioDeviceManager.removeAudioCallback(&thePlayer);
        //        thePlayer.setProcessor(nullptr);
        //        thePlugin = nullptr;
    }
    
    AudioPluginInstance *pPlugin = formatManager.createPluginInstance(*pluginDescription, sampRate,bufSz,errorMsg);
    //=======
    //    AudioProcessor::BusesLayout bl = pPlugin->getBusesLayout();
    //    int nInputs = bl.getMainInputChannels();
    //    int nOutputs = bl.getMainOutputChannels();
    ////    pPlugin->disableNonMainBuses();
    //    pPlugin->setPlayConfigDetails(nInputs,nOutputs,sampRate,bufSz);
    thePlugin = pPlugin;
    processor->sequenceObject.pThePlugin = pPlugin;
    if (thePlugin)
    {
        std::cout << "Loaded plugin \n";
        processor->pluginEnabled = true;
        //        processor->midiOutEnabled = false;
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
    //    thePlayer.setProcessor(&graph);
    thePlayer.setProcessor(thePlugin);
    audioDeviceManager.addAudioCallback (&thePlayer);
    thePlugin->suspendProcessing(false);
    MidiMessageCollector &mmc = thePlayer.getMidiMessageCollector();
    processor->pluginMessageCollector = &mmc;
    thePlugin->setPlayHead(processor);
    processor->messageCollectorReset(sampRate);
    thePlayer.getMidiMessageCollector().reset(sampRate);
}

void MainComponent::unLoadPlugin ()
{
    std::cout << "unLoading plugin " <<"\n";
    juce::AudioIODevice *curDevice =  audioDeviceManager.getCurrentAudioDevice();
    if (curDevice==nullptr)
    {
        std::cout <<"No audio device so restart audioDeviceManager\n";
        audioDeviceManager.restartLastAudioDevice();
    }
    curDevice =  audioDeviceManager.getCurrentAudioDevice();
    if (curDevice==nullptr)
    {
        std::cout <<"Still No audio device \n";
        return;
    }
    
//    const double sampRate = curDevice->getCurrentSampleRate();
//    const double bufSz = audioDeviceManager.getCurrentAudioDevice()->getCurrentBufferSizeSamples();
//    String errorMsg;

    if (processor->sequenceObject.pThePlugin)
    {
        processor->sequenceObject.pThePlugin->suspendProcessing(true);
        audioDeviceManager.removeAudioCallback(&thePlayer);
    }
    processor->sequenceObject.pThePlugin = nullptr;
    processor->pluginEnabled = false;
    processor->midiOutEnabled = true;
    
//    thePlugin = processor->sequenceObject.pThePlugin;
//    if (thePlugin)
//    {
//        std::cout << "Loaded plugin \n";
//        processor->pluginEnabled = true;
//        //        processor->midiOutEnabled = false;
//    }
//    else
//        std::cout << "Plugin error "<<errorMsg<<"\n";
    

//    thePlayer.setProcessor(&graph);
//    thePlayer.setProcessor(thePlugin);
//    audioDeviceManager.addAudioCallback (&thePlayer);
//    thePlugin->suspendProcessing(false);
//    MidiMessageCollector &mmc = thePlayer.getMidiMessageCollector();
//    processor->pluginMessageCollector = &mmc;
//    thePlugin->setPlayHead(processor);
//    processor->messageCollectorReset(sampRate);
//    thePlayer.getMidiMessageCollector().reset(sampRate);
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

void MainComponent::audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                           float** outputChannelData, int numOutputChannels,
                                           int numSamples)
{
    //noteCounter++;
    AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
    buffer.clear();
    
    MidiBuffer incomingMidi;
    processor->synthMessageCollector.removeNextBlockOfMessages(incomingMidi, numSamples);
//    sfZeroSynth.renderNextBlock (buffer, incomingMidi, 0, numSamples);
}

void MainComponent::audioDeviceAboutToStart (AudioIODevice* device)
{
    const double sampleRate = device->getCurrentSampleRate();
    processor->sampleRate = sampleRate;
//    processor->messageCollectorReset(sampleRate);
//    sfZeroSynth.setCurrentPlaybackSampleRate (sampleRate);
}

void MainComponent::audioDeviceStopped()
{
}
    
