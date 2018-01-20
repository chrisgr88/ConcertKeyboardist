/*
  ==============================================================================

    ViewerComponent.h
    Created: 17 Jan 2017 12:59:58pm
    Author:  ChrisGr

  ==============================================================================
*/

#ifndef ViewerFrame_H_INCLUDED
#define ViewerFrame_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
//#include "PluginProcessor.h"
#include "MIDIProcessor.h"
#include "ScrollingNoteViewer.h"
#include "Sequence.h"

//    static Value editVelocitiesFlag;
//==============================================================================
/**
 */
class ViewerFrame  :
public Component,
public ChangeListener,
public ChangeBroadcaster,
private Timer,
private Button::Listener,
private Slider::Listener,
private TextEditor::Listener,
public FileBrowserListener,
public ActionBroadcaster
{
public:
    ViewerFrame (MIDIProcessor *p);
    ~ViewerFrame();
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    double timeInTicks;
    double prevTimeInTicks = -1.0;
    int separatorLineWidth = 1.5;
    ModifierKeys mods;
    
    void timerCallback() override;
    
    void playFromLastPlayed()
    {
        processor->play(!processor->playing(),"lastPlayed");
    }

    bool keyStateChanged(bool isKeyDown, Component* originatingComponent)
    {
        if (!processor->isPlaying)
            return true;
        bool handledKey = false;
        for (int i=0;i<playableKeys.length();i++)
        {
            KeyPress kp = KeyPress(playableKeys[i],ModifierKeys::noModifiers, playableKeys[i]);
            if (kp.isCurrentlyDown() && !keysThatAreDown[i])
            {
                MidiMessage msg;
                if (playableKeys[i] == 's')
                    msg = MidiMessage::controllerEvent(1, 0x40, 127);
                else
                    msg = MidiMessage::noteOn(1, 60+i, (uint8)127);
                msg.setTimeStamp(99); //Value doesn't matter.
//                if (msg.isNoteOn())
//                    std::cout << "Kbd NoteOn " << (int)msg.getNoteNumber() << " " << (int)msg.getVelocity() << "\n";
//                else
//                    std::cout << "Controller " << (int)msg.getControllerNumber() << " " << (int)msg.getControllerValue() << "\n";
                msg.setChannel(16); //Channel 16 indicates notes from the computer keyboardk
                processor->addMessageToQueue(msg);
                keysThatAreDown.set(i,true);
                handledKey = true;
            }
            else if (!KeyPress::isKeyCurrentlyDown(playableKeys[i]) && keysThatAreDown[i])
            {
                MidiMessage msg;
                if (playableKeys[i] == 'r')
                    ;
                else if (playableKeys[i] == 's')
                    msg = MidiMessage::controllerEvent(1, 64, 0);
                else
                    msg = MidiMessage::noteOff(1, 60+i, (uint8)60);
                msg.setTimeStamp(99);
//                std::cout << "Kbd NoteOff " << (int)msg.getNoteNumber() << " " << (int)msg.getVelocity() << "\n";
                msg.setChannel(16); //Channel 16 indicates notes from the computer keyboardk
                processor->addMessageToQueue(msg);
                keysThatAreDown.set(i,false);
                handledKey = true;
            }
            else
                handledKey = true;
        }
        //        for (int i=0;i<keysThatAreDown.size();i++)
//            std::cout << "Key is down " << i << " " << keysThatAreDown[i] << "\n";
        return handledKey;
    }
    
    void changeListenerCallback (ChangeBroadcaster*) override;
    
    void buttonClicked (Button* button) override;
    void valueChanged (Value& value);
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    
    /** Callback when the user selects a different file in the browser. */
    virtual void selectionChanged() override;
    /** Callback when the user clicks on a file in the browser. */
    virtual void fileClicked (const File& file, const MouseEvent& e) override;
    /** Callback when the user double-clicks on a file in the browser. */
    virtual void fileDoubleClicked (const File& file) override;
    /** Callback when the browser's root folder changes. */
    virtual void browserRootChanged (const File& newRoot) override;
    
    bool is_number(const std::string& s)
    {
        return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !isdigit(c); }) == s.end();
    }
    ScrollingNoteViewer noteViewer;
    
    int toolbarHeight = 60;
    Toolbar mainToolbar;
    Toolbar altToolbar;

    void removeItem(int itemId)
    {
        int position=-1;
        for (int i=0; i < altToolbar.getNumItems(); i++)
            if (altToolbar.getItemId(i) == itemId)
                position=i;
        altToolbar.removeToolbarItem(position);
    }
    void replaceItem(int itemId, int newItemId)
    {
        int position=-1;
        for (int i=0; i < altToolbar.getNumItems(); i++)
            if (altToolbar.getItemId(i) == itemId)
                position=i;
        if (position!=-1)
        {
            altToolbar.removeToolbarItem(position);
            altToolbar.addItem(altToolbarFactory, newItemId, position);
            altToolbar.getItemComponent(position)->addListener(this);
        }
    }

private:
    MIDIProcessor *processor;
    
    ScopedPointer<FileBrowserComponent> fileBrowser;
    ScopedPointer<WildcardFileFilter> fileFilter;
    ScopedPointer<TextEditor> textEditor;
    String playableKeys = String();
    Array<bool> keysThatAreDown; //String containing characters of keys that are down (for playing from computer keyboard)

    float noteViewerBottom;
    
    TextButton playStopButton;
    TextButton rewindButton;
//    Slider tempoSlider;
    Label commandLabel;
    Label scoreNameLabel;
    Label fileNameLabel;
    Label hoverStepInfo;
    Label scoreTempoLabel;
    Label adjustedTempoLabel;
    ComponentBoundsConstrainer resizeLimits;

    bool altToolbarVisible = false;
    //<#foo#>
//==============================================================================
    class MainToolbarItemFactory   : public ToolbarItemFactory, public ComboBox::Listener, public ChangeBroadcaster
    {
    public:
        MainToolbarItemFactory(ViewerFrame *pVF) :
            pViewerFrame(pVF)
        {
//            std::cout << "pViewerFrame " << pViewerFrame << "\n";
        }
        
        ScrollingNoteViewer *pViewer;
        //==============================================================================
        // Each type of item a toolbar can contain must be given a unique ID. These
        // are the ones we'll use in this demo.
        enum ToolbarItemIds
        {
            edit_undo       = 4,
            edit_redo       = 5,
            _toggleActivity     = 25,
            _chain          = 8,
            _chordEditToggle = 9,
            _showVelocities  = 10,
            _drawVelocities  = 30,
            _adjustVelocities  = 31,
            customComboBox  = 11,
            _addSustain     = 12,
            _addSoft        = 13,
            _deleteSustain  = 14,
            _deleteSoft     = 15,
            create_chord    = 16,
            delete_chord        = 17,
            _humanizeTime       = 18,
            _humanizeTimeBox    = 19,
            _humanizeVelocity   = 20,
            _humanizeVelocityBox = 21
        };
        
        void comboBoxChanged (ComboBox* comboBoxThatHasChanged)
        {
            std::cout << "in MainToolbarItemFactory comboBoxChanged " << comboBoxThatHasChanged << "\n";
            sendChangeMessage();
        }
        
        void getAllToolbarItemIds (Array<int>& ids) override
        {
            ids.add (edit_undo);
            ids.add (edit_redo);
            ids.add (_toggleActivity);
            ids.add (_chain);
            ids.add (_addSustain);
            ids.add (_addSoft);
            ids.add (_deleteSustain);
            ids.add (_deleteSoft);
            ids.add (_chordEditToggle);
            ids.add (create_chord);
            ids.add (delete_chord);
            ids.add (_showVelocities);
            ids.add (_drawVelocities);
            ids.add (_adjustVelocities);
            ids.add (_humanizeTime);
            ids.add(customComboBox);
            ids.add (_humanizeVelocity);
            ids.add (_humanizeTimeBox);
            ids.add (_humanizeVelocityBox);
            ids.add (separatorBarId);
            ids.add (spacerId);
            ids.add (flexibleSpacerId);
        }
        
        void getDefaultItemSet (Array<int>& ids) override
        {
//            for (int n=0;n<12;n++)
//                ids.add (spacerId);
            ids.add (separatorBarId);
            ids.add (edit_undo);
            ids.add (edit_redo);
            ids.add (separatorBarId);
            ids.add (_toggleActivity);
            ids.add (separatorBarId);
            ids.add (_chain);
            ids.add(customComboBox);
            ids.add (separatorBarId);
            ids.add (_showVelocities);
            ids.add (_adjustVelocities);
            ids.add (_drawVelocities);
            ids.add (separatorBarId);
            ids.add (_chordEditToggle);
            ids.add (create_chord);
            ids.add (delete_chord);
            ids.add (_humanizeTime);
            ids.add (_humanizeTimeBox);
            ids.add (separatorBarId);
            ids.add (_humanizeVelocity);
            ids.add (_humanizeVelocityBox);
            ids.add (separatorBarId);
            ids.add (_addSustain);
            ids.add (_deleteSustain);
            ids.add (_addSoft);
            ids.add (_deleteSoft);
            ids.add (separatorBarId);
            ids.add (flexibleSpacerId);
        }
        
        ToolbarItemComponent* createItem (int itemId) override
        {
            switch (itemId)
            {
                case edit_undo:         return createButtonFromZipFileSVG (itemId, "Undo", "edit-undo.svg");
                case edit_redo:         return createButtonFromZipFileSVG (itemId, "Redo", "edit-redo.svg");
                case _toggleActivity:        return createButtonFromZipFileSVG (itemId, "Toggle selected target notes", "toggleActivityTool.svg");
                case _chain:        return createButtonFromZipFileSVG (itemId, "Autocreate target notes after note-free time intervals", "chain.svg");
                case _addSustain: return createButtonFromZipFileSVG (itemId, "Add a sustain bar", "addSustain.svg");
                case _deleteSustain: return createButtonFromZipFileSVG (itemId, "Delete a sustain bar", "deleteSustain.svg");
                case _addSoft: return createButtonFromZipFileSVG (itemId, "Add a soft bar", "addSoft.svg");
                case _deleteSoft: return createButtonFromZipFileSVG (itemId, "Delete a soft bar", "deleteSoft.svg");
                case _humanizeTime: return createButtonFromZipFileSVG (itemId, "Humanize chord note times", "humanizeStartTimes.svg");
                case _humanizeVelocity: return createButtonFromZipFileSVG (itemId, "Set chord note velocities as proportion of top note", "humanizeVelocities.svg");
                case _chordEditToggle:
                {
                    ToolbarButton *chordEditButton = createButtonFromZipFileSVG (itemId, "Show chords",
                        "chordEditToggle.svg", "chordEditToggle-pressed.svg");
                    chordEditButton->setClickingTogglesState(true);
                    return chordEditButton;
                }
                case create_chord: return  createButtonFromZipFileSVG (itemId, "Mark selected notes as a single chord", "createChord.svg");
                case delete_chord: return createButtonFromZipFileSVG (itemId, "Delete selected chords", "deleteChord.svg");
                case _showVelocities:
                {
                    ToolbarButton *showVelButton = createButtonFromZipFileSVG (itemId, "Show velocity graph of selected notes",
                        "showVelocityGraph.svg", "showVelocityGraph-pressed.svg");
                    showVelButton->getToggleStateValue().referTo(pViewer->showingVelocities);
                    showVelButton->setClickingTogglesState(true);
                    return showVelButton;
                }
                case _adjustVelocities:
                {
                    ToolbarButton *adjVelButton = createButtonFromZipFileSVG (itemId, "Drag up/down the velocities of all selected notes",
                                    "adjustVelocities.svg", "adjustVelocities-pressed.svg");
                    adjVelButton->getToggleStateValue().referTo(pViewer->adjustingVelocities);
                    adjVelButton->setClickingTogglesState(true);
                    return adjVelButton;
                }
                case _drawVelocities:
                {
                    ToolbarButton *drawVelButton = createButtonFromZipFileSVG (itemId, "Sketch a velocity graph for selected notes",
                                    "drawVelocities.svg", "drawVelocities-pressed.svg");
                    drawVelButton->getToggleStateValue().referTo(pViewer->drawingVelocities);
                    drawVelButton->setClickingTogglesState(true);
                    return drawVelButton;
                }
                case customComboBox:
                {
                    NumberComboBox *numCombo = new NumberComboBox(itemId);
                    numCombo->setTooltip("Size of break preceding autocreated target notes");
                    return numCombo;
                }
                case _humanizeVelocityBox:
                {
                    ChainAmountBox *txtBox = new ChainAmountBox (itemId);
                    txtBox->textBox.setTooltip("Chord note velocities as proportion of top note (comma separated list)");
                    return txtBox;
                }
                    
                case _humanizeTimeBox:
                {
                    ChainAmountBox *txtBox = new ChainAmountBox (itemId);
                    txtBox->textBox.setTooltip("Maximum note start time humanization in milliseconds");
                    return txtBox;
                }
                default:
                    break;
            }
            return nullptr;
        }
        
    private:
        StringArray iconNames;
        OwnedArray<Drawable> iconsFromZipFile;
        ToolbarButton* createButtonFromZipFileSVG (const int itemId, const String& text,
                                                   const String& filename, const String& filenamePressed = "")
        {
            if (iconsFromZipFile.size() == 0)
            {
                MemoryInputStream iconsFileStream (BinaryData::icons_zip, BinaryData::icons_zipSize, false);
                ZipFile icons (&iconsFileStream, false);
                
                for (int i = 0; i < icons.getNumEntries(); ++i)
                {
                    ScopedPointer<InputStream> svgFileStream (icons.createStreamForEntry (i));
                    if (svgFileStream != nullptr)
                    {
                        iconNames.add (icons.getEntry(i)->filename);
                        iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));
                        
                        if (filenamePressed.length()>0)
                        {
                            iconNames.add (icons.getEntry(i)->filename);
                            iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));
                        }
                    }
                }
            }
            Drawable* image = iconsFromZipFile [iconNames.indexOf (filename)]->createCopy();
            Drawable* imagePressed = NULL;
            if (filenamePressed.length()>0)
                imagePressed = iconsFromZipFile [iconNames.indexOf (filenamePressed)]->createCopy();
            ToolbarButton * tb = new ToolbarButton (itemId, text, image, imagePressed);
            tb->setTooltip(text);
            return tb;
        }
    public:
        //=================================================
        class ChainAmountBox : public ToolbarItemComponent, private TextEditor::Listener
        {
        public:
            ChainAmountBox (const int toolbarItemId)
            : ToolbarItemComponent (toolbarItemId, "Chaining Interval", false)
            {
                ToolbarItemComponent::addAndMakeVisible (textBox);
                textBox.setMultiLine (false);
                textBox.setReturnKeyStartsNewLine (false);
                textBox.setReadOnly (false);
                textBox.setScrollbarsShown (false);
                textBox.setCaretVisible (true);
//                textBox.setFont (Font (11));
                textBox.setPopupMenuEnabled (true);
                textBox.addListener (this);
                textBox.setColour (TextEditor::ColourIds::backgroundColourId, Colour(Colours::lightgrey));
                textBox.setColour (TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey));
                textBox.setBounds (180, 40, 20, 20);
            }
            void setWidth(int w=35)
            {
                width = w;
            }
            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical,
                                      int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;
                
                preferredSize = width;
                minSize = width;
                maxSize = width;
                return true;
            }
            void paintButtonArea (Graphics&, int, int, bool, bool) override
            {
            }
            void textEditorTextChanged(TextEditor&) 
            {
//                std::cout << "Entering text\n";
                returnPressed = false;
            }
            void textEditorReturnKeyPressed (TextEditor&)
            {
//                std::cout << "Return pressed\n";
                returnPressed = true;
            }
            void contentAreaChanged (const Rectangle<int>& newArea) override
            {
                textBox.setSize (newArea.getWidth() - 2, jmin (newArea.getHeight() - 2, 22));
                textBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
            }
            TextEditor textBox;
            int width;
            bool returnPressed = false;
        };
        
    public:
        class NumberComboBox : public ToolbarItemComponent, private ComboBox::Listener
        {
        private:
            int startupCount = 1;
            bool valChanged;
            ComboBox comboBox;
        public:

            NumberComboBox (const int toolbarItemId) : ToolbarItemComponent (toolbarItemId, "Custom Toolbar Item", false),
                comboBox ("demo toolbar combo box")
            {
                addAndMakeVisible (comboBox);
                comboBox.addListener(this);
                setValues(0.1, 9.9, 0.1, 1);
                comboBox.setSelectedId(10,NotificationType::dontSendNotification);
                comboBox.setEditableText (true);
                comboBox.setTooltip("Minimum size note separation preceding autocreated target notes (in sixteenths)");
                clearChangeFlag();
            }
            void setValues (double min, double max, double step, int decimalPlaces=0)
            {
                int position = 1;
                for (auto val=min; val<=max; val+=step, position++)
                    comboBox.addItem (String (val,1), position);
            }
            bool newValue()
            {
                return valChanged;
            }
            void clearChangeFlag()
            {
                valChanged = false;
            }
            double getValue()
            {
                return comboBox.getText().getDoubleValue();
            }
            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical, int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;
                preferredSize = 50;
                minSize =50;
                maxSize = 50;
                return true;
            }
            void paintButtonArea (Graphics&, int, int, bool, bool) override
            {
            }
            virtual void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
            {
                valChanged = true;
                std::cout <<  "combo box value "<<comboBoxThatHasChanged->getText()<<"\n";
            }
            void contentAreaChanged (const Rectangle<int>& newArea) override
            {
                comboBox.setSize (newArea.getWidth() - 2,  jmin (newArea.getHeight() - 2, 22));
                comboBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
                comboBox.setColour(ComboBox::ColourIds::backgroundColourId , Colours::darkgrey);
                comboBox.setColour(ComboBox::ColourIds::textColourId , Colours::white);
            }
        };
        
        //=================================================
        class CustomIncDecBox : public ToolbarItemComponent
        {
        public:
            CustomIncDecBox (const int toolbarItemId)
            : ToolbarItemComponent (toolbarItemId, "Chaining Interval", false)
            {
                ToolbarItemComponent::addAndMakeVisible (incDecBox);
                
                incDecBox.setRange (0, 600.0, 1);
                incDecBox.setValue (120, dontSendNotification);
                incDecBox.setSliderStyle (Slider::IncDecButtons);
                incDecBox.setBounds (180, 40, 20, 20);
                incDecBox.setTextBoxStyle(Slider::TextBoxLeft, false, 30, 20);
            }
            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical,
                                      int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;
                
                preferredSize = 90;
                minSize = 90;
                maxSize =90;
                return true;
            }
            void paintButtonArea (Graphics&, int, int, bool, bool) override
            {
            }
            void contentAreaChanged (const Rectangle<int>& newArea) override
            {
                incDecBox.setSize (newArea.getWidth() - 2, jmin (newArea.getHeight() - 2, 22));
                incDecBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
            }
            Slider incDecBox;
        };
        
        ViewerFrame *pViewerFrame;
    };
    
    //==============================================================================
    class AltToolbarItemFactory   : public ToolbarItemFactory, public ComboBox::Listener, public ChangeBroadcaster
    {
    public:
        AltToolbarItemFactory(ViewerFrame *pVF) :
        pViewerFrame(pVF)
        {
//            std::cout << "pViewerFrame " << pViewerFrame << "\n";
        }
        //==============================================================================
        // Each type of item a toolbar can contain must be given a unique ID. These
        // are the ones we'll use in this demo.
        enum ToolbarItemIds
        {
            doc_open        = 1,
            doc_save        = 2,
            doc_saveAs      = 3,
            loadPlugin      = 23,
            editPlugin      = 24,
            audioSettings    = 25,
            scoreInfo      = 26,
            _play           = 5,
            _stop           = 6,
            _playPause      = 7,
            _rewind         = 8,
            _listen         = 9,
            scoreTempo      = 10,
            scaledTempo      = 11,
            adjustedTempo   = 12,
            addTempoChange = 13,
            removeTempoChange = 14,
            addBookmark = 15,
            removeBookmark = 16,
            showEditToolbar = 17,
            _help                = 22
        };
        
        void comboBoxChanged (ComboBox* comboBoxThatHasChanged) 
        {
            std::cout << "in MainToolbarItemFactory comboBoxChanged " << comboBoxThatHasChanged << "\n";
            sendChangeMessage();
        }
        
        void getAllToolbarItemIds (Array<int>& ids) override
        {
            ids.add (doc_open);
            ids.add (doc_save);
            ids.add (doc_saveAs);
            ids.add (loadPlugin);
            ids.add (editPlugin);
            ids.add (audioSettings);
            ids.add (scoreInfo);
            ids.add (_help);
            ids.add (_play);
            ids.add (_stop);
            ids.add (_playPause);
            ids.add (_rewind);
            ids.add (_listen);
            ids.add (scoreTempo);
            ids.add (scaledTempo);
            ids.add (adjustedTempo);
            ids.add (addTempoChange);
            ids.add (removeTempoChange);
            ids.add (addBookmark);
            ids.add (removeBookmark);
            ids.add (showEditToolbar);
            ids.add (separatorBarId);
            ids.add (spacerId);
            ids.add (flexibleSpacerId);
        }
        
        void getDefaultItemSet (Array<int>& ids) override
        {
            ids.add (separatorBarId);
            ids.add (doc_open);
            ids.add (doc_save);
            ids.add (doc_saveAs);
            ids.add (loadPlugin);
            ids.add (editPlugin);
            ids.add (separatorBarId);
            ids.add (audioSettings);
            ids.add (scoreInfo);
            ids.add (showEditToolbar);
            ids.add (addBookmark);
            ids.add (_listen);
            ids.add (separatorBarId);
            ids.add (_rewind);
            ids.add (_play);
            ids.add (_stop);
            ids.add (_playPause);
//            ids.add (separatorBarId);
            ids.add (adjustedTempo);
            for (int n=0;n<3;n++)
                ids.add (spacerId);
            ids.add (addTempoChange);
            ids.add (separatorBarId);
            ids.add (_help);
            ids.add (separatorBarId);
            ids.add (flexibleSpacerId);
        }
        
        ToolbarItemComponent* createItem (int itemId) override
        {
            switch (itemId)
            {
                case doc_open:          return createButtonFromZipFileSVG (itemId, "Open", "document-open.svg");
                case doc_save:          return createButtonFromZipFileSVG (itemId, "Save", "document-save.svg");
                case doc_saveAs:        return createButtonFromZipFileSVG (itemId, "Save as", "document-save-as.svg");
                case loadPlugin:        return createButtonFromZipFileSVG (itemId, "Load plugin", "LoadPluginButton.svg");
                case editPlugin:        return createButtonFromZipFileSVG (itemId, "Show plugin window", "EditPluginButton.svg");
                case audioSettings:     return createButtonFromZipFileSVG (itemId, "Show audio settings", "AudioSettingsTool.svg");
                case scoreInfo:         return createButtonFromZipFileSVG (itemId, "Show tracks Information", "ScoreInfoTool.svg");
                case _help:             return createButtonFromZipFileSVG (itemId, "Open help in browser", "help.svg");
                case _play:             return createButtonFromZipFileSVG (itemId, "Set ready to play", "media-playback-start.svg");
                case _stop:             return createButtonFromZipFileSVG (itemId, "Stop playing", "media-playback-stop.svg");
                case _rewind:           return createButtonFromZipFileSVG (itemId, "Rewind", "media-seek-backward.svg");
                case _listen:           return createButtonFromZipFileSVG (itemId, "Listen (auto-play)", "Music.svg");
                case addTempoChange:    return createButtonFromZipFileSVG (itemId, "Add tempo change marker","AddTempoAdjustment.svg");
                case removeTempoChange: return createButtonFromZipFileSVG (itemId, "Remove tempo change marker","RemoveTempoAdjustment.svg");
                case addBookmark:       return createButtonFromZipFileSVG (itemId, "Add bookmark","AddBookmark.svg");
                case removeBookmark:    return createButtonFromZipFileSVG (itemId, "Remove bookmark","RemoveBookmark.svg");
                case showEditToolbar:    return createButtonFromZipFileSVG (itemId, "Show or hide editing tools","ShowEditToolbar.svg");
                case scoreTempo:
                {
                    ScoreTempo *scoreTempo = new ScoreTempo(itemId);
                    scoreTempo->setTooltip("Suggested tempo");
                    return scoreTempo;
                }
                case adjustedTempo:
                {
                    AdjustedTempo *tempoMultiplier = new AdjustedTempo (itemId);
                    tempoMultiplier->setTooltip("Tempo (drag up/down to change)");
                    return tempoMultiplier;
                }
                default:
                    break;
            }
            return nullptr;
        }
        
    private:
        StringArray iconNames;
        OwnedArray<Drawable> iconsFromZipFile;
        
        // This is a little utility to create a button with one of the SVG images in
        // our embedded ZIP file "icons.zip"
        ToolbarButton* createButtonFromZipFileSVG (const int itemId, const String& text, const String& filename)
        {
            if (iconsFromZipFile.size() == 0)
            {
                // If we've not already done so, load all the images from the zip file..
                MemoryInputStream iconsFileStream (BinaryData::icons_zip, BinaryData::icons_zipSize, false);
                ZipFile icons (&iconsFileStream, false);
                
                for (int i = 0; i < icons.getNumEntries(); ++i)
                {
                    ScopedPointer<InputStream> svgFileStream (icons.createStreamForEntry (i));
                    
                    if (svgFileStream != nullptr)
                    {
//                        std::cout << "file " << icons.getEntry(i)->filename<<"\n";
                        iconNames.add (icons.getEntry(i)->filename);
                        iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));
                    }
                }
            }
            
            Drawable* image = iconsFromZipFile [iconNames.indexOf (filename)]->createCopy();
            ToolbarButton * tb = new ToolbarButton (itemId, text, image, 0);

            tb->setTooltip(text);
            return tb;//new ToolbarButton (itemId, text, image, 0);
        }
    public:
        //=================================================
        class ScoreTempo : public ToolbarItemComponent, private TextEditor::Listener
        {
        public:
            ScoreTempo (const int toolbarItemId)
            : ToolbarItemComponent (toolbarItemId, "Score Tempo", false)
            {
                ToolbarItemComponent::addAndMakeVisible (textBox);
                textBox.setMultiLine (false);
                textBox.setReturnKeyStartsNewLine (false);
                textBox.setReadOnly (true);
                textBox.setScrollbarsShown (false);
                textBox.setCaretVisible (false);
                textBox.setMouseClickGrabsKeyboardFocus(false);
                textBox.setPopupMenuEnabled (false);
                textBox.setFont (Font (Font::getDefaultSerifFontName(), 19.00f, Font::plain));
                textBox.setColour (TextEditor::ColourIds::backgroundColourId, Colour(32,64,200));
                textBox.setColour (TextEditor::ColourIds::textColourId, Colour(206,206,206));
                textBox.setBounds (180, 30, 80, 10);
                textBox.setTooltip("Tempo from midi file");
            }
            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical,
                                      int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;
                
                preferredSize = 45;
                minSize = 45;
                maxSize = 45;
                return true;
            }
            void paintButtonArea (Graphics&, int, int, bool, bool) override
            {
            }
            void textEditorTextChanged(TextEditor&)
            {
                //                std::cout << "Entering text\n";
                returnPressed = false;
            }
            void textEditorReturnKeyPressed (TextEditor&)
            {
                //                std::cout << "Return pressed\n";
                returnPressed = true;
            }
            void contentAreaChanged (const Rectangle<int>& newArea) override
            {
                textBox.setSize (newArea.getWidth() - 2, jmin (newArea.getHeight() - 2, 22));
                textBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
            }
            TextEditor textBox;
            bool returnPressed = false;
        };
        
        //=================================================
        class ScaledTempo : public ToolbarItemComponent, private TextEditor::Listener
        {
        public:
            ScaledTempo (const int toolbarItemId)
            : ToolbarItemComponent (toolbarItemId, "Scaled Tempo", false)
            {
                ToolbarItemComponent::addAndMakeVisible (textBox);
                textBox.setMultiLine (false);
                textBox.setReturnKeyStartsNewLine (false);
                textBox.setReadOnly (true);
                textBox.setScrollbarsShown (false);
                textBox.setCaretVisible (false);
                textBox.setMouseClickGrabsKeyboardFocus(false);
                textBox.setPopupMenuEnabled (false);
                textBox.setFont (Font (19.00f, Font::plain));
                textBox.setColour (TextEditor::ColourIds::backgroundColourId, Colour(198,198,198));
                textBox.setColour (TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey));
                textBox.setBounds (180, 30, 80, 10);
                textBox.setTooltip("Scaled tempo");
            }
            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical,
                                      int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;
                
                preferredSize = 45;
                minSize = 45;
                maxSize = 45;
                return true;
            }
            void paintButtonArea (Graphics&, int, int, bool, bool) override
            {
            }
            void textEditorTextChanged(TextEditor&) 
            {
                //                std::cout << "Entering text\n";
                returnPressed = false;
            }
            void textEditorReturnKeyPressed (TextEditor&) 
            {
                //                std::cout << "Return pressed\n";
                returnPressed = true;
            }
            void contentAreaChanged (const Rectangle<int>& newArea) override
            {
                textBox.setSize (newArea.getWidth() - 2, jmin (newArea.getHeight() - 2, 22));
                textBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
            }
            TextEditor textBox;
            bool returnPressed = false;
        };
        
        class DraggableNumberBox : public TextEditor, public ChangeBroadcaster
        {
        public:
            DraggableNumberBox ()
            {
                mouseIsDown = false;
            }
            void setRange (double mn=1.0, double mx=100.0, int dp=1)
            {
                max=mx;
                min=mn;
                decimalPlaces=dp;
            }
            void mouseDown (const MouseEvent& e) override
            {
                startValue = preciseValue;
                mouseIsDown = true;
            }
            void mouseUp (const MouseEvent& e) override
            {
                mouseIsDown = false;
            }
            void mouseDrag (const MouseEvent& e) override
            {
                double newVal = startValue-e.getDistanceFromDragStartY();
                if (newVal<min) newVal=min;
                if (newVal>max) newVal=max;
                preciseValue = newVal;
                setText(String(newVal,decimalPlaces));
                sendChangeMessage();
            }
            void setValueWhenMouseNotDown (double val)
            {
//                if (!mouseIsDown)
                preciseValue = val;
                setText(String(val,decimalPlaces));
//                    setText(text);
            }
            double getValue()
            {
                return preciseValue;
            }
            bool mouseIsDown;
            double preciseValue;
            double startValue;
            double max, min;
            int decimalPlaces;
        };
        
        //=================================================
        class AdjustedTempo : public ToolbarItemComponent, private TextEditor::Listener, public ChangeListener
        {
        public:
            AdjustedTempo (const int toolbarItemId)
            : ToolbarItemComponent (toolbarItemId, "Tempo Multiplier", false)
            {
                ToolbarItemComponent::addAndMakeVisible (numberBox);
                numberBox.setMultiLine (false);
                numberBox.setReturnKeyStartsNewLine (false);
                numberBox.setReadOnly (true);
                numberBox.setScrollbarsShown (false);
                numberBox.setCaretVisible (true);
                numberBox.setMouseClickGrabsKeyboardFocus(false);
                //                textBox.setFont (Font (11));
                numberBox.setPopupMenuEnabled (true);
                numberBox.addListener (this);
                numberBox.setText("");
                numberBox.setBounds (180, 45, 20, 18);
                numberBox.setRange (10,300.0,0);
                numberBox.setFont (Font (18.00f, Font::plain));
                numberBox.setColour (TextEditor::ColourIds::textColourId, Colours::black);
                numberBox.setColour (TextEditor::ColourIds::backgroundColourId, Colour(Colours::lightgrey).brighter());
                numberBox.setTooltip("Tempo (drag up/down to change)");
                numberBox.decimalPlaces = 1;
            }
            
            void setWidth(int width)
            {
                
            }
            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical,
                                      int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;
                preferredSize = 55;
                minSize = 55;
                maxSize = 55;
                numberBox.addChangeListener(this);
                return true;
            }
            void paintButtonArea (Graphics&, int, int, bool, bool) override
            {
            }
            void textEditorTextChanged(TextEditor&) 
            {
                //                std::cout << "Entering text\n";
                changed = false;
            }
            void changeListenerCallback (ChangeBroadcaster* source) override
            {
                //                std::cout << "CHANGE\n";
                changed = true;
            }
            void setValue(double val)
            {
                numberBox.setValueWhenMouseNotDown(val);
            }
            void textEditorReturnKeyPressed (TextEditor&) 
            {
                double newVal = numberBox.getText().getDoubleValue();
                if (newVal<numberBox.min) newVal=newVal<numberBox.min;
                if (newVal>newVal<numberBox.max) newVal=newVal<numberBox.max;
                numberBox.setText(String(newVal,0));
                changed = true;
            }
            void contentAreaChanged (const Rectangle<int>& newArea) override
            {
                numberBox.setSize (newArea.getWidth() - 2, jmin (newArea.getHeight() - 2, 22));
                numberBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
            }
            DraggableNumberBox numberBox;
            bool changed = true;
        }; //AdjustedTempo
        
        ViewerFrame *pViewerFrame;
    };
    
    void clearSelectingNotes()
    {
        noteViewer.clearingSelectedNotes = false;
        noteViewer.markingSelectedNotes = false;
        noteViewer.clearingSelectedNotes = false;
        noteViewer.markingSelectedNotes = false;
    }

    MainToolbarItemFactory mainFactory;
    AltToolbarItemFactory altToolbarFactory;
    MainToolbarItemFactory::NumberComboBox *pChainAmountComboBox;
    AltToolbarItemFactory::ScoreTempo *pScoreTempo;
    AltToolbarItemFactory::ScaledTempo *pScaledTempo;
    AltToolbarItemFactory::AdjustedTempo *pAdjustedTempo;
    MainToolbarItemFactory::ChainAmountBox *pHumanizeVelocity;
    MainToolbarItemFactory::ChainAmountBox *pHumanizeStartTime;
    
    double chainAmount;
    String humanizeVelocityAmount = String();
    String humanizeTimeAmount = String();
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViewerFrame)
    void replaceToolbatItem(int replaceThis, int withThis) const;
};

#endif  // ViewerFrame_H_INCLUDED
