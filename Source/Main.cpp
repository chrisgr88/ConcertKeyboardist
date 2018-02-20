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

extern bool ckBlockClosing;
#include "../JuceLibraryCode/JuceHeader.h"
#include "../DateHeader.h"
#include <array>

#include "MainComponent.h"
#include "MIDIProcessor.h"
#include "MainWindow.h"

//==============================================================================
class PPApplication  : public JUCEApplication
{
public:
    //==============================================================================
    PPApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return false; }
    
    ApplicationCommandManager commandManager;
    ScopedPointer<ApplicationProperties> appProperties;
    LookAndFeel_V3 lookAndFeel;
    
    //==============================================================================
    void initialise (const String&) override
    {
        PropertiesFile::Options options;
        options.applicationName     = "ConcertKeyboardist";
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Preferences";
        ckBlockClosing = false;
        appProperties = new ApplicationProperties();
        appProperties->setStorageParameters (options);
        
        LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);
        
        mainWindow = new MainWindow (getApplicationName());

        commandManager.registerAllCommandsForTarget (this);
        commandManager.registerAllCommandsForTarget (mainWindow);
        
        //App properties - saved in app settings file
        getAppProperties().getUserSettings()->setValue ("buildDate", __CK_BUILD_DATE);
        getAppProperties().getUserSettings()->setValue ("shortHash", __CK_SHORT_HASH);
        String vis = getAppProperties().getUserSettings()->getValue ("editToolbarVisible","notSet");
        if (vis=="notSet")
            getAppProperties().getUserSettings()->setValue ("editToolbarVisible","false");
        
        String shortHash = getAppProperties().getUserSettings()->getValue ("shortHash");
        String buildDate = getAppProperties().getUserSettings()->getValue("buildDate");
        std::cout << "IDs "<<shortHash<<" "<<buildDate<<"\n";
    }
    
    void anotherInstanceStarted (const String& commandLine) override
    {
        String withNoQuotes = commandLine.removeCharacters("\"");
        File f = File::getCurrentWorkingDirectory().getChildFile(withNoQuotes);
        mainWindow->midiProcessor.loadSpecifiedFile(f);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        appProperties = nullptr;
        LookAndFeel::setDefaultLookAndFeel (nullptr);
    }

    void systemRequestedQuit() override
    {
        if (!ckBlockClosing)
        {
            if (mainWindow->midiProcessor.sequenceObject.saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
                quit();
        }
    }

private:
    //==============================================================================
    ScopedPointer<MainWindow> mainWindow;
};

static PPApplication& getApp()
{
    return *dynamic_cast<PPApplication*>(PPApplication::getInstance());
}
ApplicationCommandManager& getCommandManager()
{
    return getApp().commandManager;
}
ApplicationProperties& getAppProperties()
{
    return *getApp().appProperties;
}

//==============================================================================
START_JUCE_APPLICATION (PPApplication)
