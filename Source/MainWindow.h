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
#include <string>
#include <regex>

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
    static const int selectAll                  = 0x40032;
    static const int toggleSelectedNotesActive  = 0x41000;
    static const int setSelectedNotesActive     = 0x41001;
    static const int setSelectedNotesInactive   = 0x41002;
    static const int clearAllSelection          = 0x40030;    
    static const int marqueeSelectionAdd        = 0x41003; //Add notes to selection by dragging a marquee around them
    static const int marqueeSelectionRemove     = 0x41004;
    static const int markSelectedNotes          = 0x41005; //Add notes to selection by dragging over heads
    static const int clearSelectedNotes         = 0x41006;
    static const int chainSelectedNotes         = 0x41020; //Needs a chaining interval argument
    static const int velHumanizeSelection       = 0x41030; //Needs a vel profile argument (currently use default one)
    static const int timeHumanizeSelection      = 0x41040; //Needs a time argument
    static const int addSustain         = 0x42010;
    static const int deleteSustain      = 0x42012;
    static const int addSoft            = 0x42014;
    static const int deleteSoft         = 0x42016;
    static const int showChords         = 0x42018;
    static const int editVelocities     = 0x42019;
    static const int create_chord       = 0x43010;
    static const int delete_chord       = 0x43012;
    
        static const int toggleBookmark             = 0x49000;
    static const int playPause                  = 0x50001;
    static const int pause                      = 0x50003;
    static const int listenToSelection          = 0x50006;
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
    
    static const int help               = 0x70000;
    
    static const int dumpData                   = 0x90000;
    //    static const int tempo                      = 0x50020;
}

ApplicationCommandManager& getCommandManager();
ApplicationProperties& getAppProperties();

//==============================================================================
class MainWindow    :
public DocumentWindow,
public MenuBarModel,
public ApplicationCommandTarget,
public KeyListener,
public ActionListener,
public ActionBroadcaster,
public Timer,
public FileDragAndDropTarget
{
public:
    MainWindow (String name);
    ~MainWindow ();
    
    void timerCallback() override
    {
//        std::cout << "MainWindow " << isActiveWindow() << "\n";
        midiProcessor.appIsActive = isActiveWindow();
    }
    
    TooltipWindow tooltipWindow;    
    
    bool ckBlockClosing;
    
    bool isInterestedInFileDrag (const StringArray& files) override
    {
        return true;
    }
    
    void filesDropped (const StringArray& files, int x, int y) override
    {
        String path = files[0];
        String withNoQuotes = path.removeCharacters("\"");
        File f = File::getCurrentWorkingDirectory().getChildFile(withNoQuotes);
        midiProcessor.loadSpecifiedFile(f);
    }
    
    void actionListenerCallback (const String& message) override;
    
    bool keyPressed (const KeyPress& key, Component* originatingComponent) override;
    
    bool keyStateChanged (bool isKeyDown, Component* originatingComponent) override;
    
    void showAudioSettings();
    
    void showScoreSettings();
    
    void menuBarActivated (bool isActive) override;
    
    StringArray getMenuBarNames() override;
    
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName) override;
    
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    
    void closeButtonPressed() override;
    
    void getAllCommands (Array <CommandID>& commands) override;

    bool perform (const InvocationInfo& info) override;

    ApplicationCommandTarget* getNextCommandTarget() override;

public:
    MIDIProcessor midiProcessor;
    ViewerFrame* pViewerFrame;
    ScopedPointer<MainComponent> mainComponent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

#endif  // MAINWINDOW_H_INCLUDED
