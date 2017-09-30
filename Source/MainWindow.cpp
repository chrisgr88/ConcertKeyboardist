/*
  ==============================================================================

    MainWindow.cpp
    Created: 19 Feb 2017 9:09:17am
    Author:  ChrisGr

  ==============================================================================
*/

#include "MainWindow.h"

ApplicationCommandManager& getCommandManager();
ApplicationProperties& getAppProperties();

    MainWindow::MainWindow (String name)
    : DocumentWindow (name, Colours::lightgrey, DocumentWindow::allButtons)
    {
        formatManager.addDefaultFormats();
        ScopedPointer<XmlElement> savedPluginList (getAppProperties().getUserSettings()->getXmlValue ("pluginList"));
        
        if (savedPluginList != nullptr)
            knownPluginList.recreateFromXml (*savedPluginList);
        
        pluginSortMethod = (KnownPluginList::SortMethod) getAppProperties().getUserSettings()
        ->getIntValue ("pluginSortMethod", KnownPluginList::sortByManufacturer);
        
        knownPluginList.addChangeListener (this);
        
        
        ckBlockClosing = false;
        startTimer(1000);
        //        getAppProperties().getUserSettings()->setValue ("audioDeviceState", 99);
        setUsingNativeTitleBar (true);
        mainComponent = new MainComponent(&midiProcessor);
        pViewerFrame = mainComponent->getViewerFrame();
        pViewerFrame->addActionListener(this);
        setContentOwned (mainComponent, true);
        setResizable(true, false);
        //        centreWithSize (getWidth(), getHeight());
        restoreWindowStateFromString (getAppProperties().getUserSettings()->getValue ("mainWindowPos"));
//        setUsingNativeTitleBar(false);
        setVisible (true);
        
        RecentlyOpenedFilesList recentFiles;
        recentFiles.restoreFromString (getAppProperties().getUserSettings()->getValue ("recentConcertKeyboardistFiles"));
        //        std::cout << "Startup " << recentFiles.getNumFiles() << " " << recentFiles.toString() << "\n";
        File fileToOpen;
        if (recentFiles.getNumFiles() > 0)
            fileToOpen = recentFiles.getFile (0);
        //        File test = File::getSpecialLocation (File::currentApplicationFile);
        //        Logger::writeToLog(test.getFullPathName());
        //        fileToOpen = test.getChildFile(("twinkle_twinkle.mid"));
        //        Logger::writeToLog(fileToOpen.getFullPathName());
        //        int nFiles = test.getParentDirectory().findChildFiles(iconFiles, File::findFiles, true, fileNameStart+"*");
        
        if (fileToOpen.existsAsFile())
            midiProcessor.loadSpecifiedFile(fileToOpen);
#if JUCE_MAC
        PopupMenu extra_menu;
        extra_menu.addItem (123, "About ConcertKeyboardist");
        extra_menu.addSeparator();
        extra_menu.addItem (0x999, "Preferences...");
        //        extra_menu.addCommandItem(&getCommandManager(), CommandIDs::fileOpen, "Preferences...");
        MainWindow::setMacMainMenu (this, &extra_menu);
#else
        setMenuBar (this);
#endif
        addKeyListener (this);
        addKeyListener (getCommandManager().getKeyMappings());
        getCommandManager().setFirstCommandTarget (this);
    }
    
    MainWindow::~MainWindow ()  {
        pluginListWindow = nullptr;
        knownPluginList.removeChangeListener (this);
        getAppProperties().getUserSettings()->setValue ("mainWindowPos", getWindowStateAsString());
#if JUCE_MAC
        setMacMainMenu (nullptr);
#else
        setMenuBar (nullptr);
#endif
    }

    void MainWindow::changeListenerCallback (ChangeBroadcaster* changed)
    {
        if (changed == &knownPluginList)
        {
            menuItemsChanged();
            
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

// This is sent actions from viewerFrame where the toolbar is implemented
    void MainWindow::actionListenerCallback (const String& message)
    {
//        std::cout <<"actionListenerCallback"<< message << "\n";
        if (message == "fileSave")
            perform (CommandIDs::fileSave);
        else if (message == "fileOpen")
            perform (CommandIDs::fileOpen);
        
        else if (message == "fileSaveAs")
            perform (CommandIDs::fileSaveAs);
        
        else if (message == "editUndo")
            perform (CommandIDs::editUndo);
        
        else if (message == "editRedo")
            perform (CommandIDs::editRedo);
        else if (message == "play")
            perform (CommandIDs::playPause);
        else if (message == "rewind")
            perform (CommandIDs::rewind);
        
        else if (message == "listenToSelection")
            perform (CommandIDs::listenToSelection);
        
        else if (message == "toggleActivity")
            perform (CommandIDs::toggleSelectedNotesActive);
        
//        else if (message == "marqueeSelectionAdd")
//            perform (CommandIDs::marqueeSelectionAdd);
//        else if (message == "marqueeSelectionRemove")
//            perform (CommandIDs::marqueeSelectionRemove);
//        else if (message == "markSelectedNote")
//            perform (CommandIDs::markSelectedNotes);
//        else if (message == "clearSelectedNotes")
//            perform (CommandIDs::clearSelectedNotes);
        else if (message == "clearAllSelection")
            perform (CommandIDs::clearAllSelection);
        
        else if (message.upToFirstOccurrenceOf(":",false,true) == "chain")
        {
            midiProcessor.sequenceObject.chainingInterval = String(message.fromLastOccurrenceOf(":", false, true)).getDoubleValue();
            perform (CommandIDs::chainSelectedNotes);
        }
        else if (message.upToFirstOccurrenceOf(":",false,true) == "humanizeVelocity")
        {
            const String hV = String(message.fromLastOccurrenceOf(":", false, true));
//            std::cout << "Performing HumanizeVelocity " <<hV<<"\n";
            midiProcessor.sequenceObject.setChordVelocityHumanize(hV, true);
            perform(CommandIDs::velHumanizeSelection);
        }
        else if (message.upToFirstOccurrenceOf(":",false,true) == "humanizeTime")
        {
            std::string htSpec = message.fromFirstOccurrenceOf(":", false, true).toStdString();
            std::regex justRand("^\\d+$");
            std::regex randAndSlope("^\\d+[//\\\\]\\d+$");
            std::regex randAndSlopeAndSeed("^\\d+[//\\\\]\\d+:\\d+$");
            std::regex randAndSeed("^\\d+:\\d+$");
            int slopeSign = 1;
            if (std::regex_match(htSpec, randAndSlope) || std::regex_match(htSpec, randAndSlopeAndSeed))
            {
                if (message.fromFirstOccurrenceOf(":", false, true).containsAnyOf("/"))
                    slopeSign = 1;
                else
                    slopeSign = -1;
            }
//            std::cout << htSpec << ": " << std::regex_match(htSpec, justRand) << " Matches justRand"<< '\n';
//            std::cout << htSpec << ": " << std::regex_match(htSpec, randAndSlope) << " Matches randAndSlope"<<" "<<slopeSign<< '\n';
//            std::cout << htSpec << ": " << std::regex_match(htSpec, randAndSlopeAndSeed)<<" Matches randAndSlopeAndSeed"<<" "<<slopeSign<< '\n';
//            std::cout << htSpec << ": " << std::regex_match(htSpec, randAndSeed) << " Matches randAndSeed"<< '\n';
            
            if (std::regex_match(htSpec, justRand) || std::regex_match(htSpec, randAndSlope) ||
                std::regex_match(htSpec, randAndSlopeAndSeed) || std::regex_match(htSpec, randAndSeed))
                midiProcessor.sequenceObject.setChordTimeHumanize(htSpec, true);
            perform(CommandIDs::timeHumanizeSelection);
        }
        else if (message == "addSustain")
        {
            perform (CommandIDs::addSustain);
        }
        else if (message == "deleteSustain")
        {
            perform (CommandIDs::deleteSustain);
        }
        else if (message == "addSoft")
        {
            perform (CommandIDs::addSoft);
        }
        else if (message == "deleteSoft")
        {
            perform (CommandIDs::deleteSoft);
        }
        else if (message == "_showChords")
        {
            perform (CommandIDs::showChords);
        }
        else if (message == "_editVelocities")
        {
            perform (CommandIDs::editVelocities);
        }
        else if (message == "create_chord")
        {
            perform (CommandIDs::create_chord);
//            midiProcessor.createChord();
        }
        else if (message == "delete_chord")
        {
            perform (CommandIDs::delete_chord);
//            midiProcessor.deleteChords(true);
        }
        else if (message == "help")
        {
            perform (CommandIDs::help);
            //            midiProcessor.deleteChords(true);
        }
    }
    
    bool MainWindow::keyPressed (const KeyPress& key, Component* originatingComponent)
    {
        //        std::cout <<"keyPressed in MainWindow\n";
        return false;
    }
    
    bool MainWindow::keyStateChanged (bool isKeyDown, Component* originatingComponent)
    {
        bool temp = pViewerFrame->keyStateChanged(isKeyDown, originatingComponent);
        return temp;
    }
    
    void MainWindow::showAudioSettings()
    {
        AudioDeviceSelectorComponent audioSettingsComp (mainComponent->audioDeviceManager, 0, 0, 0, 256, true, true, true, false);
        audioSettingsComp.setSize (500, 450);
        DialogWindow::LaunchOptions o;
        o.content.setNonOwned (&audioSettingsComp);
        o.dialogTitle                   = "Audio/MIDI Settings";
        o.componentToCentreAround       = this;
        o.dialogBackgroundColour        = Colours::azure;
        o.escapeKeyTriggersCloseButton  = true;
        o.useNativeTitleBar             = false;
        o.resizable                     = true;
        
        ckBlockClosing = true;
        o.runModal();
        ckBlockClosing = false;
        ScopedPointer<XmlElement> audioState (mainComponent->audioDeviceManager.createStateXml());
        getAppProperties().getUserSettings()->setValue ("audioDeviceState", audioState);
        getAppProperties().getUserSettings()->saveIfNeeded();
    }
//    
//    void MainWindow::showScoreSettings()
//    {
//        
//        //TODO Add call of this and make it contain the TracksComponent
//        //    ScopedPointer<DialogWindow> settingsWindow;
//        TracksComponent tracksComponent(&midiProcessor);
//        tracksComponent.setSize (897, 200);
//        DialogWindow::LaunchOptions dw;
//        dw.content.setNonOwned (&tracksComponent);
//        dw.dialogTitle                   = "Tracks";
//        dw.componentToCentreAround       = this;
//        dw.dialogBackgroundColour        = Colours::azure;
//        dw.escapeKeyTriggersCloseButton  = true;
//        dw.useNativeTitleBar             = true;
//        dw.resizable                     = true;
//        ckBlockClosing = true;
//        dw.runModal();
//        ckBlockClosing = false;
//        PropertiesFile* userSettings = getAppProperties().getUserSettings();
//        setResizable(true, false);
//        setResizeLimits(300, 400, 800, 1500);
//        setTopLeftPosition(60, 60);
//        restoreWindowStateFromString(userSettings->getValue("listWindowPos"));
//        setVisible(true);
//    }
    
    void MainWindow::menuBarActivated (bool isActive)
    {
//        std::cout <<"menuBarActivated\n";
    }
    
    StringArray MainWindow::getMenuBarNames()
    {
        const char* const names[] = {"File", "Edit", "Plugins", "Sequence", "Window", "Help", nullptr };
        
        return StringArray (names);
    }
    
    void MainWindow::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
    {
        const String category ("General");
        
        switch (commandID)
        {
            case CommandIDs::appAboutBox:
                result.setInfo ("About...", "About...", category, 0);
                break;
            case CommandIDs::audioMidiSettings:
                result.setInfo ("Audio and Midi Settings...", "Audio and Midi Settings", category, 0);
                result.defaultKeypresses.add (KeyPress (',', ModifierKeys::commandModifier, 0));
                break;
            case CommandIDs::showPluginListEditor:
                result.setInfo ("Manage available plug-Ins...", String(), category, 0);
                result.addDefaultKeypress ('p', ModifierKeys::commandModifier);
                break;
            case CommandIDs::fileOpen:
                result.setInfo ("Open...", "Open or import a file", category, 0);
                result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
                break;
            case CommandIDs::fileRecent:
                result.setInfo ("Open Recent", "Open a recently used file", category, 0);
                break;
            case CommandIDs::fileSave:
                result.setInfo ("Save", "Save the file", category, 0);
                result.addDefaultKeypress ('s', ModifierKeys::commandModifier);
                break;
            case CommandIDs::fileSaveAs:
                result.setInfo ("Save As...", "Save copy of file with new name", category, 0);
                result.addDefaultKeypress ('s', ModifierKeys::commandModifier|ModifierKeys::shiftModifier);
                break;
            case CommandIDs::playPause:
                result.setInfo ("Play/Pause", "Play or pause", category, 0);
                result.defaultKeypresses.add (KeyPress (' ', ModifierKeys::noModifiers, 0));
                break;
//            case CommandIDs::playFromCurrentPlayhead:
//                result.setInfo ("PlayFromLastPlayed", "Play From Last Played", category, 0);
//                result.defaultKeypresses.add (KeyPress (' ', ModifierKeys::shiftModifier, 0));
//                break;
            case CommandIDs::pause:
                result.setInfo ("Pause", "Pause", category, 0);
                result.defaultKeypresses.add (KeyPress (' ', ModifierKeys::noModifiers, 0));
                break;
//            case CommandIDs::playFromPreviousStart:
//                result.setInfo ("playFromPreviousPlayStart", "playFromPreviousPlayStart", category, 0);
//                result.defaultKeypresses.add (KeyPress ('-', ModifierKeys::noModifiers, 0));
//                break;
            case CommandIDs::listenToSelection:
                result.setInfo ("listenToSelection", "listenToSelection", category, 0);
                result.defaultKeypresses.add (KeyPress ('=', ModifierKeys::noModifiers, 0));
                break;
            case CommandIDs::rewind:
                result.setInfo ("Rewind", "Rewind", category, 0);
                result.defaultKeypresses.add (KeyPress ('r', ModifierKeys::noModifiers, 0));
                break;
            case CommandIDs::increaseTempo:
                result.setInfo ("IncreaseTempo", "Increase Tempo", category, 0);
                result.defaultKeypresses.add (KeyPress ('8', ModifierKeys::noModifiers, 0));
                break;
            case CommandIDs::decreaseTempo:
                result.setInfo ("DecreaseTempo", "Decrease Tempo", category, 0);
                result.defaultKeypresses.add (KeyPress ('7', ModifierKeys::noModifiers, 0));
                break;
            case CommandIDs::scoreSettings:
                result.setInfo ("Tracks...", "Tracks in this score", category, 0);
                result.addDefaultKeypress ('t', ModifierKeys::commandModifier);
                break;
            case CommandIDs::editUndo:
                result.setInfo ("Undo", "Undo last action", category, 0);
                result.addDefaultKeypress ('z', ModifierKeys::commandModifier);
                break;
            case CommandIDs::editRedo:
                result.setInfo ("Redo", "Redo last undo", category, 0);
                result.addDefaultKeypress ('z', ModifierKeys::commandModifier|ModifierKeys::shiftModifier);
                break;
            case CommandIDs::clearAllSelection:
                result.setInfo ("ClearAllSelection", "Clear Selection", category, 0);
                result.addDefaultKeypress (KeyPress::escapeKey, ModifierKeys::noModifiers);
                break;
            case CommandIDs::selectAll:
                result.setInfo ("SelectAll", "SelectAll", category, 0);
                result.addDefaultKeypress ('a', ModifierKeys::commandModifier);
                break;
//            case CommandIDs::marqueeSelectionAdd:
//                result.setInfo ("marqueeSelectionAdd", "marqueeSelectionAdd", category, 0);
//                result.addDefaultKeypress ('1', ModifierKeys::noModifiers);
//                break;
//            case CommandIDs::marqueeSelectionRemove:
//                result.setInfo ("marqueeSelectionRemove", "marqueeSelectionRemove", category, 0);
//                result.addDefaultKeypress ('2', ModifierKeys::noModifiers);
//                break;
//            case CommandIDs::markSelectedNotes:
//                result.setInfo ("markSelectedNotes", "markSelectedNotes", category, 0);
//                result.addDefaultKeypress ('3', ModifierKeys::noModifiers);
//                break;
//            case CommandIDs::clearSelectedNotes:
//                result.setInfo ("clearSelectedNotes", "clearSelectedNotes", category, 0);
//                result.addDefaultKeypress ('4', ModifierKeys::noModifiers);
//                break;
            case CommandIDs::toggleSelectedNotesActive:
                result.setInfo ("toggleSelectedNotesActive", "toggleSelectedNotesActive", category, 0);
                result.addDefaultKeypress ('t', ModifierKeys::noModifiers);
                break;
            case CommandIDs::setSelectedNotesActive:
                result.setInfo ("setSelectedNotesActive", "setSelectedNotesActive", category, 0);
                result.addDefaultKeypress ('a', ModifierKeys::noModifiers);
                break;
            case CommandIDs::setSelectedNotesInactive:
                result.setInfo ("setSelectedNotesInactive", "setSelectedNotesInactive", category, 0);
                result.addDefaultKeypress ('i', ModifierKeys::noModifiers);
                break;
            case CommandIDs::chainSelectedNotes:
                result.setInfo ("chainSelectedNotes", "chainSelectedNotes", category, 0);
                result.addDefaultKeypress ('c', ModifierKeys::noModifiers);
                break;
            case CommandIDs::velHumanizeSelection:
                result.setInfo ("velHumanizeSelection", "velHumanizeSelection", category, 0);
//                result.addDefaultKeypress ('8', ModifierKeys::commandModifier);
                break;
            case CommandIDs::timeHumanizeSelection:
                result.setInfo ("timeHumanizeSelection", "timeHumanizeSelection", category, 0);
//                result.addDefaultKeypress ('9', ModifierKeys::commandModifier);
                break;
                
            case CommandIDs::addSustain:
                result.setInfo ("addSustain", "addSustain", category, 0);
                result.addDefaultKeypress ('s', ModifierKeys::noModifiers);
                break;
            case CommandIDs::deleteSustain:
                result.setInfo ("deleteSustain", "deleteSustain", category, 0);
                result.addDefaultKeypress ('s', ModifierKeys::shiftModifier);
                break;
            case CommandIDs::addSoft:
                result.setInfo ("addSoft", "addSoft", category, 0);
                result.addDefaultKeypress ('f', ModifierKeys::noModifiers);
                break;
            case CommandIDs::deleteSoft:
                result.setInfo ("deleteSoft", "deleteSoft", category, 0);
                result.addDefaultKeypress ('f', ModifierKeys::shiftModifier);
                break;
            case CommandIDs::showChords:
                result.setInfo ("showChords", "showChords", category, 0);
                break;
            case CommandIDs::editVelocities:
                result.setInfo ("editVelocities", "Edit Velocities", category, 0);
                result.addDefaultKeypress ('6', ModifierKeys::noModifiers);
                break;
            case CommandIDs::create_chord:
                result.setInfo ("create_chord", "create_chord", category, 0);
                result.addDefaultKeypress ('d', ModifierKeys::noModifiers);
                break;
            case CommandIDs::delete_chord:
                result.setInfo ("delete_chord", "delete_chord", category, 0);
                result.addDefaultKeypress ('d', ModifierKeys::shiftModifier);
                break;
            case CommandIDs::previousTargetNote:
                result.setInfo ("PreviousTargetNote", "Go to Previous Target Note", category, 0);
                result.addDefaultKeypress (KeyPress::leftKey, ModifierKeys::noModifiers);
                break;
            case CommandIDs::previousMeasure:
                result.setInfo ("PreviousMeasure", "Go to Previous Measure", category, 0);
                result.addDefaultKeypress (KeyPress::leftKey, ModifierKeys::shiftModifier);
                break;
            case CommandIDs::nextTargetNote:
                result.setInfo ("NextTargetNote", "Go to Next Target Note", category, 0);
                result.addDefaultKeypress (KeyPress::rightKey, ModifierKeys::noModifiers);
                break;
            case CommandIDs::nextMeasure:
                result.setInfo ("NextMeasure", "Go to Next Measure", category, 0);
                result.addDefaultKeypress (KeyPress::rightKey, ModifierKeys::shiftModifier);
                break;
            case CommandIDs::toggleBookmark:
                result.setInfo ("Bookmark", "Toggle Bookmark", category, 0);
                result.addDefaultKeypress ('b', ModifierKeys::commandModifier);
                break;
            case CommandIDs::previousBookmark:
                result.setInfo ("PreviousBookmark", "Go to Previous Bookmark", category, 0);
                result.addDefaultKeypress (KeyPress::leftKey, ModifierKeys::commandModifier);
                break;
            case CommandIDs::nextBookmark:
                result.setInfo ("NextBookmark", "Go to Next Bookmark", category, 0);
                result.addDefaultKeypress (KeyPress::rightKey, ModifierKeys::commandModifier);
                break;
            case CommandIDs::help:
                result.setInfo ("Help", "Open Help in Browser", category, 0);
                break;
            case CommandIDs::dumpData:
                result.setInfo ("dumpData", "dumpData", category, 0);
                result.addDefaultKeypress ('d', ModifierKeys::commandModifier);
                break;
            default:
                break;
        }
    }
    
    PopupMenu MainWindow::getMenuForIndex (int topLevelMenuIndex, const String& menuName)
    {
        PopupMenu menu;
        
        if (topLevelMenuIndex == 0)
        {
            menu.addCommandItem (&getCommandManager(), CommandIDs::appAboutBox);
            
            // "File" menu
            menu.addCommandItem (&getCommandManager(), CommandIDs::fileOpen);
            
            RecentlyOpenedFilesList recentFiles;
            recentFiles.restoreFromString (getAppProperties().getUserSettings()->getValue ("recentConcertKeyboardistFiles"));
            String files = recentFiles.toString();
            //            std::cout << "Create menu " << recentFiles.getNumFiles() << " " << recentFiles.toString() << "\n";
            
            PopupMenu recentFilesMenu;
            recentFiles.createPopupMenuItems (recentFilesMenu, 100, true, true);
            menu.addSubMenu ("Open recent file", recentFilesMenu);
            
            menu.addCommandItem (&getCommandManager(), CommandIDs::fileSave);
            menu.addCommandItem (&getCommandManager(), CommandIDs::fileSaveAs);
            menu.addSeparator();
            menu.addCommandItem (&getCommandManager(), CommandIDs::audioMidiSettings);
            menu.addCommandItem (&getCommandManager(), CommandIDs::scoreSettings);
            menu.addSeparator();
            menu.addCommandItem (&getCommandManager(), StandardApplicationCommandIDs::quit);
        }
        else if (topLevelMenuIndex == 1)
        {
            menu.addCommandItem (&getCommandManager(), CommandIDs::editUndo);
            menu.addCommandItem (&getCommandManager(), CommandIDs::editRedo);
            //            menu.addCommandItem (&getCommandManager(), CommandIDs::setPlayheadToHere);
            menu.addCommandItem (&getCommandManager(), CommandIDs::playPause);
//            menu.addCommandItem (&getCommandManager(), CommandIDs::playFromCurrentPlayhead);
//            menu.addCommandItem (&getCommandManager(), CommandIDs::pause);
//            menu.addCommandItem (&getCommandManager(), CommandIDs::playFromPreviousStart);
            menu.addCommandItem (&getCommandManager(), CommandIDs::listenToSelection);
        }
        else if (topLevelMenuIndex == 2) // "Plugins" menu
        {
            PopupMenu pluginsMenu;
            addPluginsToMenu (pluginsMenu);
            menu.addSubMenu ("Load plugin", pluginsMenu);
            menu.addItem (250, "Unload plugin");
            menu.addSeparator();
            menu.addCommandItem (&getCommandManager(), CommandIDs::showPluginListEditor);
        }
        return menu;
    }
void MainWindow::addPluginsToMenu (PopupMenu& m) const
{
    knownPluginList.addToMenu (m, pluginSortMethod);
}
const PluginDescription* MainWindow::getChosenType (const int menuID) const
{
    int index = knownPluginList.getIndexChosenByMenu (menuID);
    if (index != -1)
        return knownPluginList.getType (index);
    else
        return NULL;
}
void MainWindow::menuItemSelected (int menuItemID, int topLevelMenuIndex)
    {
        if (topLevelMenuIndex==-1)
        {
            if (menuItemID==0x999)
                showAudioSettings();
            else if (menuItemID==123)
                std::cout <<"About\n";
        }
        else if (topLevelMenuIndex==0)
        {
            if (100<=menuItemID && menuItemID<=150)
            {
                RecentlyOpenedFilesList recentFiles;
                recentFiles.restoreFromString (getAppProperties().getUserSettings()->getValue ("recentConcertKeyboardistFiles"));
                File recent = recentFiles.getFile (menuItemID - 100);
                String files = recentFiles.toString();
                String path = recent.getFullPathName();
    //            std::cout << "Load recent file " << recentFiles.getNumFiles() << " " <<  path << " " << recentFiles.toString() << "\n";
                
                if (midiProcessor.sequenceObject.saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
                    midiProcessor.loadSpecifiedFile(recent);
                Component::toFront(true);
            }
        }
        else if (topLevelMenuIndex==2)
        {
            if (menuItemID == 250)
            {
                std::cout << "Unload plugin" <<"\n";
                //            if (auto* graphEditor = getGraphEditor())
                //                if (auto* filterGraph = graphEditor->graph.get())
                //                    filterGraph->clear();
            }
            else
            {
                const PluginDescription* desc = getChosenType (menuItemID);
                if (desc != NULL)
                {
                    std::cout << "Loading plugin "<<desc->descriptiveName <<"\n";
                    const PluginDescription *pPID = knownPluginList.getType(knownPluginList.getIndexChosenByMenu (menuItemID));
                    String errorMsg;
                    AudioPluginInstance* loaded = formatManager.createPluginInstance(*pPID, 500,500,errorMsg);
                    if (loaded)
                    {
                        std::cout << "Loaded plugin "<<getChosenType (menuItemID)->descriptiveName <<"\n";
                    }
                }
            }
        }
    }
    
    void MainWindow::closeButtonPressed()
    {
        JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
    void MainWindow::getAllCommands (Array <CommandID>& commands)
    {
        const CommandID ids[] = {
            CommandIDs::appAboutBox,
            CommandIDs::audioMidiSettings,
            CommandIDs::showPluginListEditor,
            CommandIDs::fileOpen,
            CommandIDs::fileRecent,
            CommandIDs::fileSave,
            CommandIDs::fileSaveAs,
            CommandIDs::playPause,
            CommandIDs::pause,
            CommandIDs::listenToSelection,
            CommandIDs::increaseTempo,
            CommandIDs::decreaseTempo,
            CommandIDs::scoreSettings,
            CommandIDs::editUndo,
            CommandIDs::editRedo,
            CommandIDs::clearAllSelection,
            CommandIDs::selectAll,
            CommandIDs::toggleSelectedNotesActive,
//            CommandIDs::marqueeSelectionAdd,
//            CommandIDs::marqueeSelectionRemove,
//            CommandIDs::markSelectedNotes,
//            CommandIDs::clearSelectedNotes,
            CommandIDs::setSelectedNotesActive,
            CommandIDs::setSelectedNotesInactive,
            CommandIDs::chainSelectedNotes,
            CommandIDs::velHumanizeSelection,
            CommandIDs::timeHumanizeSelection,
            CommandIDs::addSustain,
            CommandIDs::deleteSustain,
            CommandIDs::addSoft,
            CommandIDs::deleteSoft,
            CommandIDs::showChords,
            CommandIDs::editVelocities,
            CommandIDs::create_chord,
            CommandIDs::delete_chord,
            
            CommandIDs::rewind,
            CommandIDs::previousTargetNote,
            CommandIDs::previousMeasure,
            CommandIDs::nextTargetNote,
            CommandIDs::nextMeasure,
            CommandIDs::toggleBookmark,
            CommandIDs::previousBookmark,
            CommandIDs::nextBookmark,
            CommandIDs::help,
            CommandIDs::dumpData
        };
        commands.addArray (ids, numElementsInArray (ids));
    }
    
    bool MainWindow::perform (const InvocationInfo& info)
    {
        switch (info.commandID)
        {
            case CommandIDs::appAboutBox:
                if (!midiProcessor.isPlaying)
                    std::cout <<"About\n";
                break;
            case CommandIDs::audioMidiSettings:
                if (!midiProcessor.isPlaying)
                {
                    showAudioSettings();
                    Component::toFront(true);
                }
                break;
            case CommandIDs::showPluginListEditor:
                if (pluginListWindow == nullptr)
                    pluginListWindow = new PluginListWindow (*this, formatManager);
                
                pluginListWindow->toFront (true);
                break;
            case CommandIDs::fileOpen:
                if (!midiProcessor.isPlaying)
                {
                    ckBlockClosing = true;
                    if (midiProcessor.sequenceObject.saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
                        midiProcessor.loadFromUserSpecifiedFile();
                    Component::toFront(true);
                    ckBlockClosing = false;
                }
                break;
            case CommandIDs::fileRecent:
                //                std::cout <<"openRecent\n";
                break;
            case CommandIDs::fileSave:
                if (!midiProcessor.isPlaying)
                {
                    File file = midiProcessor.sequenceObject.getLastDocumentOpened();
                    if (!file.getFullPathName().endsWith("[ck].mid"))
                    {
                        midiProcessor.sequenceObject.saveAs(File(), true, true, true);

                        
                        RecentlyOpenedFilesList recentFiles;
                        recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                                       ->getValue ("recentConcertKeyboardistFiles"));
    //                    int num1 = recentFiles.getNumFiles();
                        recentFiles.removeFile(file);
                        getAppProperties().getUserSettings()
                        ->setValue ("recentConcertKeyboardistFiles", recentFiles.toString());
    //                    int num2 = recentFiles.getNumFiles();                    
                        

    //                    std::cout <<"N files before, after"<<num1<<" "<<num2<<"\n";
                    }
                    else
                        midiProcessor.sequenceObject.saveDocument(midiProcessor.sequenceObject.getLastDocumentOpened());
                    Component::toFront(true);
                }
                break;
            case CommandIDs::fileSaveAs:
            {
                if (!midiProcessor.isPlaying)
                {
                    File file = midiProcessor.sequenceObject.getLastDocumentOpened();
                    midiProcessor.sequenceObject.saveAs(File(), true, true, true);
                    Component::toFront(true);
                }
                break;
            }
                
            case CommandIDs::playPause:
                if (midiProcessor.playing())
                {
                    if (midiProcessor.isListening)
                    {
                        midiProcessor.play(false,"ZTL");
                    }
                    else
                    {
//                        const double nxtTick = midiProcessor.getStartTimeOfNextStep();
                        midiProcessor.play(false,"currentPlayhead");
//                        midiProcessor.tweenMove(nxtTick, 200);
                    }
                }
                else
                {
//                    midiProcessor.catchUp();
//                    const double nxtTick = midiProcessor.getStartTimeOfNextStep();
//                    midiProcessor.tweenMove(nxtTick, 2000);
                    midiProcessor.play(true,"ZTL");
                }
                break;
            case CommandIDs::pause:
                midiProcessor.play(false,"ZTL");
                break;
            case CommandIDs::listenToSelection:
                if (!midiProcessor.isPlaying || midiProcessor.waitingForFirstNote)
                {
                    midiProcessor.listenToSelection();
                }
                else
                {
                    midiProcessor.play(false,"ZTL");
                }
                break;
            case CommandIDs::rewind:
//                std::cout <<"Rewind\n";
                if (midiProcessor.isListening)
                {
                    midiProcessor.play(false,"ZTL");
                }
                else
                {
                    if (midiProcessor.isPlaying)
                        midiProcessor.play(false,"currentPlayhead");
                    if (midiProcessor.getTimeInTicks()==midiProcessor.lastStartTime)
                        midiProcessor.tweenMove(0, 200);
                    else
                        midiProcessor.tweenMove(midiProcessor.lastStartTime, 200);
                }
                break;
            case CommandIDs::increaseTempo:
                midiProcessor.sequenceObject.increaseTempo(1.03);
                std::cout <<"increaseTempo\n";
                break;
            case CommandIDs::decreaseTempo:
                midiProcessor.sequenceObject.decreaseTempo(0.97);
                std::cout <<"decreaseTempo\n";
                break;
            case CommandIDs::scoreSettings:
                std::cout <<"tracksWindow\n";
                if (!midiProcessor.isPlaying)
                {
                    if (tracksWindow == nullptr)
                        tracksWindow = new TracksWindow (*this, formatManager, &midiProcessor);
                    tracksWindow->toFront (true);
//                    showScoreSettings();
                    midiProcessor.buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, midiProcessor.getTimeInTicks());
                }
                break;
            case CommandIDs::editUndo:
                std::cout <<"editUndo\n";
                if(midiProcessor.undoMgr->canUndo())
                {
                    midiProcessor.undoMgr->undo();
                }
                break;
            case CommandIDs::editRedo:
                std::cout <<"editRedo\n";
                if(midiProcessor.undoMgr->canRedo())
                    midiProcessor.undoMgr->redo();
                break;

            case CommandIDs::selectAll:
                std::cout <<"selectAll\n";
                if (!midiProcessor.isPlaying)
                    pViewerFrame->noteViewer.selectAll();
                break;
                
//            case CommandIDs::marqueeSelectionAdd:
//                std::cout <<"marqueeSelectionAdd\n";
//                pViewerFrame->marqueeAddPressed();
//                break;
//            case CommandIDs::marqueeSelectionRemove:
//                std::cout <<"marqueeSelectionRemove\n";
//                pViewerFrame->marqueeRemovePressed();
//                break;
//            case CommandIDs::markSelectedNotes:
//                std::cout <<"markSelectedNotes\n";
//                pViewerFrame->markingAddPressed();
//                break;
//            case CommandIDs::clearSelectedNotes:
//                std::cout <<"clearSelectedNotes\n";
//                pViewerFrame->markingRemovePressed();
//                break;
            case CommandIDs::clearAllSelection:
                std::cout <<"clearAllSelection\n";
                if (!midiProcessor.isPlaying)
                {
                    pViewerFrame->noteViewer.clearSelectedNotes();
                    midiProcessor.variableTempoRatio = 1.0;
                    
                }
                break;
                
            case CommandIDs::toggleSelectedNotesActive:
                if (!midiProcessor.isPlaying)
                {
                    if (midiProcessor.copyOfSelectedNotes.size()>0)
                    {
                        midiProcessor.undoMgr->beginNewTransaction();
                        MIDIProcessor::ActionSetNoteActivity* action;
                        const bool setActive = !midiProcessor.getNoteActivity(midiProcessor.copyOfSelectedNotes[0]);
                        action = new MIDIProcessor::ActionSetNoteActivity(midiProcessor, setActive,
                                                                          midiProcessor.copyOfSelectedNotes);
                        midiProcessor.undoMgr->perform(action);
                    }
                }
                break;
            case CommandIDs::setSelectedNotesActive:
                if (!midiProcessor.isPlaying)
                {
                    if (midiProcessor.copyOfSelectedNotes.size()>0)
                    {
                        midiProcessor.undoMgr->beginNewTransaction();
                        MIDIProcessor::ActionSetNoteActivity* action;
                        action = new MIDIProcessor::ActionSetNoteActivity(midiProcessor, true,
                                                                          midiProcessor.copyOfSelectedNotes);
                        midiProcessor.undoMgr->perform(action);
                    }
                }
                break;
            case CommandIDs::setSelectedNotesInactive:
                if (!midiProcessor.isPlaying)
                {
                    if (midiProcessor.copyOfSelectedNotes.size()>0)
                    {
                        midiProcessor.undoMgr->beginNewTransaction();
                        MIDIProcessor::ActionSetNoteActivity* action;
                        action = new MIDIProcessor::ActionSetNoteActivity(midiProcessor, false,
                                                                          midiProcessor.copyOfSelectedNotes);
                        midiProcessor.undoMgr->perform(action);
                    }
                }
                break;
            case CommandIDs::chainSelectedNotes:
                std::cout <<"chainSelectedNotes\n";
                if (!midiProcessor.isPlaying)
                {
                    {
                        midiProcessor.undoMgr->beginNewTransaction();
                        MIDIProcessor::ActionChain* action;
                        // Passing -1 causes the chain command to use the interval from the toolbar
                        action = new MIDIProcessor::ActionChain(midiProcessor, -1, midiProcessor.copyOfSelectedNotes);
                        midiProcessor.undoMgr->perform(action);
                        midiProcessor.sequenceObject.setChangedFlag(true);
                        midiProcessor.catchUp();
                    }
                }
                break;
            case CommandIDs::timeHumanizeSelection:
                std::cout <<"timeHumanizeSelection\n";
                if (!midiProcessor.isPlaying)
                {
                    std::vector<std::shared_ptr<NoteWithOffTime>> pointersToSelectedNotes = pViewerFrame->noteViewer.stashSelectedNotes();
                    midiProcessor.timeHumanizeChords(midiProcessor.copyOfSelectedNotes);
                    pViewerFrame->noteViewer.restoreSelectedNotes(pointersToSelectedNotes);
                }
                break;
            case CommandIDs::velHumanizeSelection:
                std::cout <<"velHumanizeSelection\n";
                if (!midiProcessor.isPlaying)
                    midiProcessor.velocityHumanizeChords(midiProcessor.copyOfSelectedNotes);
//                    midiProcessor.humanizeChordNoteVelocities();
                break;
            case CommandIDs::addSustain:
                {
                    std::cout <<"addSustain\n";
                    if (!midiProcessor.isPlaying)
                        midiProcessor.addPedalChange(MIDIProcessor::sustPedal);
                }
                break;
            case CommandIDs::deleteSustain:
                {
                    std::cout <<"deleteSustain\n";
                    if (!midiProcessor.isPlaying)
                        midiProcessor.deletePedalChange(MIDIProcessor::sustPedal);
                }
                break;
            case CommandIDs::addSoft:
                std::cout <<"addSoft\n";
                if (!midiProcessor.isPlaying)
                    midiProcessor.addPedalChange(MIDIProcessor::softPedal);
                break;
            case CommandIDs::deleteSoft:
                std::cout <<"deleteSoft\n";
                if (!midiProcessor.isPlaying)
                    midiProcessor.deletePedalChange(MIDIProcessor::softPedal);
                break;
            case CommandIDs::showChords:
                std::cout <<"showChords\n";
                if (!midiProcessor.isPlaying)
                {
                    pViewerFrame->noteViewer.setShowingChords(!pViewerFrame->noteViewer.showingChords);
                    pViewerFrame->resized();
                    pViewerFrame->repaint();
                }
                break;
            case CommandIDs::editVelocities:
                std::cout <<"editVelocities\n";
                if (!midiProcessor.isPlaying)
                {
                    if (pViewerFrame->noteViewer.editingVelocities.getValue())
                        pViewerFrame->noteViewer.editingVelocities = false;
                    else
                        pViewerFrame->noteViewer.editingVelocities = true;
                }
                break;
            case CommandIDs::create_chord:
                std::cout <<"create_chord\n";
                if (!midiProcessor.isPlaying)
                    midiProcessor.createChord();
                break;
            case CommandIDs::delete_chord:
                std::cout <<"delete_chord\n";
                if (!midiProcessor.isPlaying)
                    midiProcessor.deleteChords(true);
                break;
            case CommandIDs::toggleBookmark:
                if (!midiProcessor.isPlaying)
                {
                    midiProcessor.catchUp();
                    midiProcessor.addRemoveBookmark (BOOKMARK_TOGGLE);
                    std::cout <<"toggleBookmark\n";
                }
                break;
                /////////////////////////
            case CommandIDs::previousTargetNote:
                //                std::cout <<"previousTargetNote\n";
                if (!midiProcessor.atZTL())
                    midiProcessor.catchUp();
                midiProcessor.playableStepForwardBack(false);
                break;
            case CommandIDs::previousMeasure:
                //                std::cout <<"previousMeasure\n";
                if (!midiProcessor.atZTL())
                    midiProcessor.catchUp();
                midiProcessor.measureForwardBack(false);
                break;
            case CommandIDs::nextTargetNote:
                //                std::cout <<"nextTargetNote\n";
                if (!midiProcessor.atZTL())
                    midiProcessor.catchUp();
                midiProcessor.playableStepForwardBack(true);
                break;
            case CommandIDs::nextMeasure:
                //                std::cout <<"NextMeasure\n";
                if (!midiProcessor.atZTL())
                    midiProcessor.catchUp();
                midiProcessor.measureForwardBack(true);
                break;
            case CommandIDs::previousBookmark:
            {
                //                std::cout <<"PreviousBookmark\n";
                if (!midiProcessor.atZTL())
                    midiProcessor.catchUp();
                midiProcessor.bookmarkForwardBack(false);
                //                pViewerFrame->setPlayheadToHere();
                break;
            }
            case CommandIDs::nextBookmark:
            {
                //                std::cout <<"NextBookmark\n";
                if (!midiProcessor.atZTL())
                    midiProcessor.catchUp();
                midiProcessor.bookmarkForwardBack(true);
                //                pViewerFrame->setPlayheadToHere();
                break;
            }
            case CommandIDs::help:
            {
                std::cout <<"Get Help\n";
                if (!midiProcessor.isPlaying)
                {
                    File::getSpecialLocation(File::currentApplicationFile);
                    String docPath = File::getSpecialLocation(File::currentApplicationFile).getChildFile("Contents/Resources/Documentation/EN/ckdoc.html").getFullPathName();
                    docPath = "file://" + docPath;
                    std::cout << "doc path " << docPath << "\n";
                    URL docURL = URL(docPath);
                    docURL.launchInDefaultBrowser();
                }
                break;
            }
            case CommandIDs::dumpData:
                midiProcessor.sequenceObject.dumpData(pViewerFrame->noteViewer.getSelectedNotes()); //Dump selection to console
            break;
            default:
                return false;
        }
        return true;
    }
    
    ApplicationCommandTarget* MainWindow::getNextCommandTarget()
    {
        return findFirstTargetParentComponent();
    }


