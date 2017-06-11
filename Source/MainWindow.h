/*
  ==============================================================================

    MainWindow.h
    Created: 19 Feb 2017 9:09:17am
    Author:  ChrisGr

  ==============================================================================
*/

#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED
#include "../JuceLibraryCode/JuceHeader.h"
#include <array>

#include "MainComponent.h"
#include "ViewerFrame.h"
#include "TracksComponent.h"

namespace CommandIDs
{
    static const int appAboutBox                = 0x20000;
    static const int audioMidiSettings          = 0x20050;
    static const int fileOpen                   = 0x30000;
    static const int fileRecent                 = 0x30010; //To submenu
    static const int fileSave                   = 0x30040;
    static const int fileSaveAs                 = 0x30050;
    static const int scoreSettings              = 0x40060;
    static const int editUndo                   = 0x40010;
    static const int editRedo                   = 0x40020;
    static const int clearSelection             = 0x40030;
    static const int toggleSelectedNotesActive  = 0x41000;
    static const int chainSelectedNotes         = 0x41020; //Needs a chaining interval argument
    static const int velHumanizeSelection       = 0x41030; //Needs a vel profile argument (currently use default one)
    static const int timeHumanizeSelection      = 0x41040; //Needs a time argument
    static const int toggleBookmark             = 0x49000;
//    static const int setPlayheadToHere          = 0x50000;
    static const int playPause                  = 0x50001;
    static const int playFromCurrentPlayhead   = 0x50002;
    static const int playFromPreviousStart      = 0x50003;
    static const int listenToSelection          = 0x50005;
    static const int rewind                     = 0x50010;
    static const int increaseTempo              = 0x50020;
    static const int decreaseTempo              = 0x50030;
    //////////////
    static const int previousTargetNote         = 0x60010;
    static const int previousMeasure            = 0x60020;
    static const int nextTargetNote             = 0x60030;
    static const int nextMeasure                = 0x60040;
    static const int previousBookmark           = 0x60050;
    static const int nextBookmark               = 0x60060;
    //    static const int tempo                      = 0x50020;
}

ApplicationCommandManager& getCommandManager();
ApplicationProperties& getAppProperties();

//==============================================================================
class MainWindow    :
public DocumentWindow,
public MenuBarModel,
public ApplicationCommandTarget,
public KeyListener
{
public:
    MainWindow (String name)
    : DocumentWindow (name, Colours::lightgrey, DocumentWindow::allButtons)
    {
        
//        getAppProperties().getUserSettings()->setValue ("audioDeviceState", 99);
        setUsingNativeTitleBar (true);
        mainComponent = new MainComponent(&midiProcessor);
        pViewerFrame = mainComponent->getViewerFrame();
        setContentOwned (mainComponent, true);
        setResizable(true, false);
//        centreWithSize (getWidth(), getHeight());
        restoreWindowStateFromString (getAppProperties().getUserSettings()->getValue ("mainWindowPos"));
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
    
    ~MainWindow ()  {
        getAppProperties().getUserSettings()->setValue ("mainWindowPos", getWindowStateAsString());
#if JUCE_MAC
        setMacMainMenu (nullptr);
#else
        setMenuBar (nullptr);
#endif
    }
    
    bool keyPressed (const KeyPress& key, Component* originatingComponent) override
    {
//        std::cout <<"keyPressed in MainWindow\n";
        return false;
    }
    
    bool keyStateChanged (bool isKeyDown, Component* originatingComponent) override
    {
        bool temp = pViewerFrame->keyStateChanged(isKeyDown, originatingComponent);
        return temp;
    }
    
    void showAudioSettings()
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
        o.runModal();
        ScopedPointer<XmlElement> audioState (mainComponent->audioDeviceManager.createStateXml());
        getAppProperties().getUserSettings()->setValue ("audioDeviceState", audioState);
        getAppProperties().getUserSettings()->saveIfNeeded();
    }
    
    void showScoreSettings()
    {
        
        //TODO Add call of this and make it contain the TracksComponent
        //    ScopedPointer<DialogWindow> settingsWindow;
        TracksComponent tracksComponent(&midiProcessor);
        tracksComponent.setSize (1000, 200);
        DialogWindow::LaunchOptions o;
        o.content.setNonOwned (&tracksComponent);
        o.dialogTitle                   = "Score Settings";
        o.componentToCentreAround       = this;
        o.dialogBackgroundColour        = Colours::azure;
        o.escapeKeyTriggersCloseButton  = true;
        o.useNativeTitleBar             = false;
        o.resizable                     = true;
        o.runModal();
    }
    
    void menuBarActivated (bool isActive) override
    {
        std::cout <<"menuBarActivated\n";
    }
    
    StringArray getMenuBarNames() override
    {
        const char* const names[] = {"File", "Edit", "Sequence", "Window", "Help", nullptr };
        
        return StringArray (names);
    }
    
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
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
                result.setInfo ("playPause", "playPause", category, 0);
                result.defaultKeypresses.add (KeyPress (' ', ModifierKeys::noModifiers, 0));
                break;
            case CommandIDs::playFromCurrentPlayhead:
                result.setInfo ("PlayFromLastPlayed", "Play From Last Played", category, 0);
                result.defaultKeypresses.add (KeyPress (' ', ModifierKeys::shiftModifier, 0));
                break;
            case CommandIDs::playFromPreviousStart:
                result.setInfo ("playFromPreviousPlayStart", "playFromPreviousPlayStart", category, 0);
                result.defaultKeypresses.add (KeyPress ('-', ModifierKeys::noModifiers, 0));
                break;
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
                result.defaultKeypresses.add (KeyPress ('2', ModifierKeys::noModifiers, 0));
                break;
            case CommandIDs::decreaseTempo:
                result.setInfo ("DecreaseTempo", "Decrease Tempo", category, 0);
                result.defaultKeypresses.add (KeyPress ('1', ModifierKeys::noModifiers, 0));
                break;
            case CommandIDs::scoreSettings:
                result.setInfo ("Score Settings...", "Settings for this score", category, 0);
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
            case CommandIDs::clearSelection:
                result.setInfo ("ClearSelection", "Clear Selection", category, 0);
                result.addDefaultKeypress (KeyPress::escapeKey, ModifierKeys::noModifiers);
                break;
            case CommandIDs::toggleSelectedNotesActive:
                result.setInfo ("toggleSelectedNotesActive", "toggleSelectedNotesActive", category, 0);
                result.addDefaultKeypress ('a', ModifierKeys::noModifiers);
                result.addDefaultKeypress ('t', ModifierKeys::noModifiers);
                break;
            case CommandIDs::chainSelectedNotes:
                result.setInfo ("chainSelectedNotes", "chainSelectedNotes", category, 0);
                result.addDefaultKeypress ('c', ModifierKeys::noModifiers);
                break;
            case CommandIDs::velHumanizeSelection:
                result.setInfo ("velHumanizeSelection", "velHumanizeSelection", category, 0);
                result.addDefaultKeypress ('4', ModifierKeys::commandModifier);
                break;
            case CommandIDs::timeHumanizeSelection:
                result.setInfo ("timeHumanizeSelection", "timeHumanizeSelection", category, 0);
                result.addDefaultKeypress ('5', ModifierKeys::commandModifier);
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
            default:
                break;
        }
    }
    
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName) override
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
            menu.addCommandItem (&getCommandManager(), CommandIDs::playFromCurrentPlayhead);
            menu.addCommandItem (&getCommandManager(), CommandIDs::playFromPreviousStart);
            menu.addCommandItem (&getCommandManager(), CommandIDs::listenToSelection);
        }
        return menu;
    }
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override
    {
        if (topLevelMenuIndex==-1)
        {
            if (menuItemID==0x999)
                showAudioSettings();
            else if (menuItemID==123)
                std::cout <<"About\n";
        }
        else if (topLevelMenuIndex==1 && 100<=menuItemID && menuItemID<=150)
        {
            RecentlyOpenedFilesList recentFiles;
            recentFiles.restoreFromString (getAppProperties().getUserSettings()->getValue ("recentConcertKeyboardistFiles"));
            File recent = recentFiles.getFile (menuItemID - 100);
            String files = recentFiles.toString();
            String path = recent.getFullPathName();
            std::cout << "Load recent file " << recentFiles.getNumFiles() << " " <<  path << " " << recentFiles.toString() << "\n";
            
            if (midiProcessor.sequenceObject.saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
                midiProcessor.loadSpecifiedFile(recent);
            Component::toFront(true);
        }
    }
    
    void closeButtonPressed() override
    {
        JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
    void getAllCommands (Array <CommandID>& commands) override
    {
        const CommandID ids[] = {
            CommandIDs::appAboutBox,
            CommandIDs::audioMidiSettings,
            CommandIDs::fileOpen,
            CommandIDs::fileRecent,
            CommandIDs::fileSave,
            CommandIDs::fileSaveAs,
            CommandIDs::playPause,
            CommandIDs::playFromCurrentPlayhead,
            CommandIDs::playFromPreviousStart,
            CommandIDs::listenToSelection,
            CommandIDs::increaseTempo,
            CommandIDs::decreaseTempo,
            CommandIDs::scoreSettings,
            CommandIDs::editUndo,
            CommandIDs::editRedo,
            CommandIDs::clearSelection,
            CommandIDs::toggleSelectedNotesActive,
            CommandIDs::chainSelectedNotes,
            CommandIDs::velHumanizeSelection,
            CommandIDs::timeHumanizeSelection,
            CommandIDs::rewind,
            CommandIDs::previousTargetNote,
            CommandIDs::previousMeasure,
            CommandIDs::nextTargetNote,
            CommandIDs::nextMeasure,
            CommandIDs::toggleBookmark,
            CommandIDs::previousBookmark,
            CommandIDs::nextBookmark
        };
        commands.addArray (ids, numElementsInArray (ids));
    }
    
    bool perform (const InvocationInfo& info) override
    {
        switch (info.commandID)
        {
            case CommandIDs::appAboutBox:
                std::cout <<"About\n";
                break;
                
            case CommandIDs::audioMidiSettings:
                showAudioSettings();
                Component::toFront(true);
                break;
                
            case CommandIDs::fileOpen:
                if (midiProcessor.sequenceObject.saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
                    midiProcessor.loadFromUserSpecifiedFile();
                Component::toFront(true);
//                std::cout <<"fileOpen - Set focus to 'this'\n";
                break;
                
            case CommandIDs::fileRecent:
//                std::cout <<"openRecent\n";
                break;
                
            case CommandIDs::fileSave:
            {
                File file = midiProcessor.sequenceObject.getLastDocumentOpened();
                if (!file.getFullPathName().endsWith("[ck].mid"))
                    midiProcessor.sequenceObject.saveAs(File(), true, true, true);
                else
                    midiProcessor.sequenceObject.saveDocument(midiProcessor.sequenceObject.getLastDocumentOpened());
                Component::toFront(true);
                break;
            }
            case CommandIDs::fileSaveAs:
            {
                File file = midiProcessor.sequenceObject.getLastDocumentOpened();
                midiProcessor.sequenceObject.saveAs(File(), true, true, true);
                Component::toFront(true);
                break;
            }
//            case CommandIDs::setPlayheadToHere:
//                pViewerFrame->setPlayheadToHere();
//                break;
                
            case CommandIDs::playPause:
                midiProcessor.play(!midiProcessor.playing(),"ZTL");
                break;
            case CommandIDs::playFromCurrentPlayhead:
                midiProcessor.play(true,"currentPlayhead");
                break;
            case CommandIDs::playFromPreviousStart:
                if (midiProcessor.isPlaying)
                    midiProcessor.play(false,"ZTL");
                midiProcessor.play(true,"previousStart");
                break;
                
            case CommandIDs::listenToSelection:
                if (midiProcessor.isPlaying)
                    midiProcessor.play(false,"ZTL");
                midiProcessor.listenToSelection();
                break;
                
            case CommandIDs::rewind:
                std::cout <<"Rewind\n";
                if (!midiProcessor.isPlaying)
                    midiProcessor.rewind(0.0);
                break;
                
            case CommandIDs::increaseTempo:
                midiProcessor.sequenceObject.increaseTempo(1.03);
                std::cout <<"increaseTempo\n";
                break;
                
            case CommandIDs::decreaseTempo:
//                pViewerFrame->spaceKeyPressed();
                midiProcessor.sequenceObject.decreaseTempo(0.97);
                std::cout <<"decreaseTempo\n";
                break;
                
            case CommandIDs::scoreSettings:
                showScoreSettings();
                std::cout <<"scoreSettings\n";
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
                
            case CommandIDs::clearSelection:
                std::cout <<"clearSelection\n";
                if (!midiProcessor.isPlaying)
                {
                    pViewerFrame->noteViewer.clearSelectedNotes();
                    midiProcessor.variableTempoRatio = 1.0;
                    
                }
                break;
                
            case CommandIDs::toggleSelectedNotesActive:
                std::cout <<"toggleSelectedNotesActive\n";
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
                
            case CommandIDs::chainSelectedNotes:
            {
                std::cout <<"chainSelectedNotes\n";
                if (!midiProcessor.isPlaying)
                {
                    midiProcessor.undoMgr->beginNewTransaction();
                    MIDIProcessor::ActionChain* action;
                    if (midiProcessor.copyOfSelectedNotes.size()>0)
                    {
                        action = new MIDIProcessor::ActionChain(midiProcessor, -1, midiProcessor.copyOfSelectedNotes);
                        midiProcessor.undoMgr->perform(action);
                        midiProcessor.sequenceObject.setChangedFlag(true);
                        midiProcessor.catchUp();
                        midiProcessor.buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, midiProcessor.getTimeInTicks());
                    }
                }
            }
                break;
                
            case CommandIDs::velHumanizeSelection:
                std::cout <<"velHumanizeSelection\n";
                break;
                
            case CommandIDs::timeHumanizeSelection:
                std::cout <<"timeHumanizeSelection\n";
                break;
                
            case CommandIDs::toggleBookmark:
                midiProcessor.catchUp();
                midiProcessor.addRemoveBookmark (BOOKMARK_TOGGLE);
                std::cout <<"toggleBookmark\n";
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
                
            default:
                return false;
        }
        return true;
    }
    
    ApplicationCommandTarget* getNextCommandTarget() override
    {
        return findFirstTargetParentComponent();
    }
    
public:
    MIDIProcessor midiProcessor;
    ViewerFrame* pViewerFrame;
    ScopedPointer<MainComponent> mainComponent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};



#endif  // MAINWINDOW_H_INCLUDED
