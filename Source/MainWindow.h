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
#include "AboutWindow.h"
#include <array>

#include "MainComponent.h"
#include "ViewerFrame.h"
#include "TracksComponent.h"
#include <string>
#include <regex>

/** A desktop window containing a plugin's UI. */
class PluginWindow  : public DocumentWindow
{
public:
    enum WindowFormatType
    {
        Normal = 0,
        Generic,
        Programs,
        Parameters,
        AudioIO,
        NumTypes
    };
    
    PluginWindow (AudioProcessorEditor*, AudioProcessor*, WindowFormatType);
    ~PluginWindow();
    
    static PluginWindow* getWindowFor (AudioProcessor*, WindowFormatType);
    
//    static void closeCurrentlyOpenWindowsFor (const uint32 nodeId);
    static void closeAllCurrentlyOpenWindows();
    
    void moved() override;
    void closeButtonPressed() override;
    
private:
    AudioProcessor* owner;
    WindowFormatType type;
    
    float getDesktopScaleFactor() const override     { return 1.0f; }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginWindow)
};

namespace CommandIDs
{
    static const int appAboutBox                = 0x20000;
    static const int audioMidiSettings          = 0x20050;
    static const int showPluginListEditor       = 0x20051;
    static const int showPlugWindow             = 0x20052;
//    static const int enablePlugin               = 0x20053;
    static const int enableMidiOut               = 0x20054;

    static const int showEditToolbar            = 0x21000;

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
    static const int graphVelocities     = 0x42019;
    static const int adjustVelocities     = 0x42020;
    static const int drawVelocities     = 0x42021;
    static const int create_chord       = 0x43010;
    static const int delete_chord       = 0x43012;

    static const int hide_measure_lines  = 0x44000;
    
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
public FileDragAndDropTarget,
public ChangeListener
{
public:
    MainWindow (String name);
    ~MainWindow ();
    
    virtual void activeWindowStatusChanged	() override
    {
//        std::cout << "activeWindowStatusChanged "<< isActiveWindow()<< "\n";
    }
    
    void timerCallback() override
    {
//        std::cout << "Active Window Process::isForegroundProcess()"<< Process::isForegroundProcess()<< "\n";
        Process::isForegroundProcess();
        if (!midiProcessor.fullPowerMode && (Process::isForegroundProcess()/* || isMouseOver(true)*/))
        {
//            std::cout << "Make Active \n";
//<<<<<<< HEAD
//            midiProcessor.appIsActive = true;
////            mainComponent->audioDeviceManager.restartLastAudioDevice();
//=======
            midiProcessor.fullPowerMode = true;
//            mainComponent->audioDeviceManager.restartLastAudioDevice();
//>>>>>>> 1be6f6cd44ea296389e2a93b53ac58f35a624112
        }
        else if (midiProcessor.fullPowerMode && !(Process::isForegroundProcess()/* || isMouseOver(true)*/) )
        {
//            std::cout << "Make InActive \n";
//<<<<<<< HEAD
//            midiProcessor.appIsActive = false;
////            mainComponent->audioDeviceManager.closeAudioDevice();
//=======
            midiProcessor.fullPowerMode = false;
//            mainComponent->audioDeviceManager.closeAudioDevice();
//>>>>>>> 1be6f6cd44ea296389e2a93b53ac58f35a624112
        }
    }
    
//    virtual void mouseDown (const MouseEvent& event) override
//    {
//        midiProcessor.fullPowerMode = true;
//    }
    
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
    
    void changeListenerCallback (ChangeBroadcaster* changed) override;
    void actionListenerCallback (const String& message) override;
    
    bool keyPressed (const KeyPress& key, Component* originatingComponent) override;
    
    bool keyStateChanged (bool isKeyDown, Component* originatingComponent) override;
    
    void showAudioSettings();
    
//    void showScoreSettings();
    
    void menuBarActivated (bool isActive) override;
    
    StringArray getMenuBarNames() override;
    
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName) override;
    
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    
    void closeButtonPressed() override;
    
    void getAllCommands (Array <CommandID>& commands) override;

    bool perform (const InvocationInfo& info) override;

    ApplicationCommandTarget* getNextCommandTarget() override;
    void addPluginsToMenu (PopupMenu&) const;
    const PluginDescription* getChosenType (const int menuID) const;
    const PluginDescription* pluginContextMenu (Rectangle<int> menuAt) const;
    
public:
    MIDIProcessor midiProcessor;
    ViewerFrame* pViewerFrame;
    ScopedPointer<MainComponent> mainComponent;

private:
//    AudioPluginFormatManager formatManager;
//    KnownPluginList knownPluginList;
//    KnownPluginList::SortMethod pluginSortMethod;
    class PluginListWindow;
    class TracksWindow;
    ScopedPointer<PluginListWindow> pluginListWindow;
    ScopedPointer<Component> pAboutWindow;
    String windowPosProperty;
    ScopedPointer<TracksWindow> tracksWindow;
    
    String chordTimeHumanizeSpec = String();
    String chordVelocityHumanizeSpec = String();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

//==============================================================================
class MainWindow::PluginListWindow  : public DocumentWindow
{
public:
    PluginListWindow (MainWindow& owner_, AudioPluginFormatManager& pluginFormatManager)
    : DocumentWindow ("Available Plugins",
                      LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                      DocumentWindow::minimiseButton | DocumentWindow::closeButton),
    owner (owner_)
    {
        const File deadMansPedalFile (getAppProperties().getUserSettings()
                                      ->getFile().getSiblingFile ("RecentlyCrashedPluginsList"));
        
        setContentOwned (new PluginListComponent (pluginFormatManager,
                                                  owner.mainComponent->knownPluginList,
                                                  deadMansPedalFile,
                                                  getAppProperties().getUserSettings(), true), true);
        
        setResizable (true, false);
        setResizeLimits (300, 400, 800, 1500);
        setTopLeftPosition (60, 60);
        
        restoreWindowStateFromString (getAppProperties().getUserSettings()->getValue ("listWindowPos"));
        setVisible (true);
    }
    
    ~PluginListWindow()
    {
        getAppProperties().getUserSettings()->setValue ("listWindowPos", getWindowStateAsString());
        
        clearContentComponent();
    }
    
    void closeButtonPressed()
    {
        owner.pluginListWindow = nullptr;
    }
    
private:
    MainWindow& owner;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListWindow)
};

//==============================================================================
class MainWindow::TracksWindow  : public DocumentWindow
{
public:
    TracksWindow (MainWindow& owner_, AudioPluginFormatManager& pluginFormatManager, MIDIProcessor *pMidiProc)
    : DocumentWindow ("Tracks",
                      LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                      DocumentWindow::minimiseButton | DocumentWindow::closeButton),
    owner (owner_)
    {
        proc = pMidiProc;
        setContentOwned (new TracksComponent (pMidiProc), false);        PropertiesFile* userSettings = getAppProperties().getUserSettings();
        setResizable(true, false);
        setResizeLimits(900, 150, 1200, 1800);
        setTopLeftPosition(60, 60);
        restoreWindowStateFromString(userSettings->getValue("listWindowPos"));
        setVisible (true);
    }
    
    ~TracksWindow()
    {
        clearContentComponent();
    }
    
    void closeButtonPressed()
    {
        owner.tracksWindow = nullptr;
    }
    
private:
    MainWindow& owner;
    MIDIProcessor *proc;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TracksWindow)
};

#endif  // MAINWINDOW_H_INCLUDED
