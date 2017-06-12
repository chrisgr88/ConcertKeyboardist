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
public FileBrowserListener
{
public:
    ViewerFrame (MIDIProcessor *p);
    ~ViewerFrame();
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    double timeInTicks;
    double prevTimeInTicks = -1.0;
    ModifierKeys mods;
    
    void timerCallback() override;
    
    void setPlayheadToHere()
    {
        processor->setLastPlayedSeqStep();
        processor->catchUp();
//        processor->play(!processor->playing(),"current");
    }
    
//    void playFromCurrentPosition()
//    {
//        processor->play(!processor->playing(),"current");
//    }
    
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
                msg.setTimeStamp(99.0); //Value doesn't matter.
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
                msg.setTimeStamp(99.0);
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
    virtual void textEditorReturnKeyPressed (TextEditor&) override;
    
//    void setProcessor(MIDIProcessor *p)
//    {
//        processor = p;
//        noteViewer.setProcessor(p);
//    }
    
    bool is_number(const std::string& s)
    {
        return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
    }
    ScrollingNoteViewer noteViewer;
    
    int toolbarHeight = 60;
private:
    MIDIProcessor *processor;
    
    ScopedPointer<FileBrowserComponent> fileBrowser;
    ScopedPointer<WildcardFileFilter> fileFilter;
    ScopedPointer<TextEditor> textEditor;
    String playableKeys;
    Array<bool> keysThatAreDown; //String containing characters of keys that are down (for playing from computer keyboard)
    
    TextButton playStopButton;
    TextButton rewindButton;
    Slider tempoSlider;
    Label scaledTempo;
    Label realtimeTempo;
    Label commandLabel;
    Label scoreNameLabel;
    Label fileNameLabel;
    Label hoverStepInfo;
//    ScopedPointer<ResizableCornerComponent> resizer;
    ComponentBoundsConstrainer resizeLimits;
    
    Toolbar toolbar;
    
//==============================================================================
//==============================================================================
//==============================================================================
    class DemoToolbarItemFactory   : public ToolbarItemFactory, public ComboBoxListener, public ChangeBroadcaster
    {
    public:
        DemoToolbarItemFactory(ViewerFrame *pVF) :
            pViewerFrame(pVF)
        {
            std::cout << "pViewerFrame " << pViewerFrame << "\n";
        }
        //==============================================================================
        // Each type of item a toolbar can contain must be given a unique ID. These
        // are the ones we'll use in this demo.
        enum DemoToolbarItemIds
        {
            doc_open        = 1,
            doc_save        = 2,
            doc_saveAs      = 3,
            edit_undo       = 4,
            edit_redo       = 5,
            _makeActive     = 6,
            _makeInactive    = 7,
            _chain          = 8,
            _play           = 9,
            _stop           = 10,
            _playPause      = 11,
            _rewind         = 12,
            _listen         = 13,
            _rePlay         = 14,
            customComboBox  = 15
        };
        
        void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
        {
            std::cout << "in DemoToolbarItemFactory comboBoxChanged " << comboBoxThatHasChanged << "\n";
            sendChangeMessage();
        }
        
        void getAllToolbarItemIds (Array<int>& ids) override
        {
            // This returns the complete list of all item IDs that are allowed to
            // go in our toolbar. Any items you might want to add must be listed here. The
            // order in which they are listed will be used by the toolbar customisation panel.
            
            ids.add (doc_open);
            ids.add (doc_save);
            ids.add (doc_saveAs);
            ids.add (edit_undo);
            ids.add (edit_redo);
            ids.add (_makeActive);
            ids.add (_makeInactive);
            ids.add (_chain);
            ids.add (_play);
            ids.add (_stop);
            ids.add (_rewind);
            ids.add (_playPause);
            ids.add (_listen);
            ids.add (_rePlay);
            ids.add (separatorBarId);
            ids.add (spacerId);
            ids.add (flexibleSpacerId);
        }
        
        void getDefaultItemSet (Array<int>& ids) override
        {
            // This returns an ordered list of the set of items that make up a
            // toolbar's default set. Not all items need to be on this list, and
            // items can appear multiple times (e.g. the separators used here).
//            ids.add (doc_new);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (doc_open);
            ids.add (doc_save);
            ids.add (doc_saveAs);
            ids.add (separatorBarId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (separatorBarId);
            ids.add (edit_undo);
            ids.add (edit_redo);
            ids.add (separatorBarId);
            ids.add (_makeActive);
            ids.add (_makeInactive);
            ids.add (_chain);
            ids.add (separatorBarId);
            ids.add (flexibleSpacerId);
            ids.add (separatorBarId);
            ids.add (_rewind);
            ids.add (_play);
            ids.add (_stop);
            ids.add (_listen);
            ids.add (_rePlay);
            ids.add (separatorBarId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
            ids.add (spacerId);
        }
        
        ToolbarItemComponent* createItem (int itemId) override
        {
            switch (itemId)
            {
                case doc_open:      return createButtonFromZipFileSVG (itemId, "Open", "document-open.svg");
                case doc_save:      return createButtonFromZipFileSVG (itemId, "Save", "document-save.svg");
                case doc_saveAs:    return createButtonFromZipFileSVG (itemId, "Save As", "document-save-as.svg");
                case edit_undo:         return createButtonFromZipFileSVG (itemId, "Undo", "edit-undo.svg");
                case edit_redo:         return createButtonFromZipFileSVG (itemId, "Redo", "edit-redo.svg");
                    
                case _makeActive:        return createButtonFromZipFileSVG (itemId, "Make Active", "makeActive.svg");
                case _makeInactive:        return createButtonFromZipFileSVG (itemId, "Make Inactive.svg", "makeInactive.svg");
                    
                case _chain:        return createButtonFromZipFileSVG (itemId, "Chain", "chain.svg");
                case _play:        return createButtonFromZipFileSVG (itemId, "Play", "media-playback-start.svg");
                case _stop:        return createButtonFromZipFileSVG (itemId, "Stop", "media-playback-stop.svg");
                    
                case _rewind:        return createButtonFromZipFileSVG (itemId, "Rewind", "media-seek-backward.svg");
                case _listen:        return createButtonFromZipFileSVG (itemId, "Listen", "Music.svg");
                case _rePlay:        return createButtonFromZipFileSVG (itemId, "Play Again", "replay.svg");
                    
                case customComboBox:
                {
                    CustomToolbarComboBox *ccb = new CustomToolbarComboBox (itemId);
//                    ccb->addListener(pViewerFrame);
                    return ccb;
                }
                default:                break;
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
                        iconNames.add (icons.getEntry(i)->filename);
                        iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));
                    }
                }
            }
            
            Drawable* image = iconsFromZipFile [iconNames.indexOf (filename)]->createCopy();
            ToolbarButton * tb = new ToolbarButton (itemId, text, image, 0);
            return tb;//new ToolbarButton (itemId, text, image, 0);
        }
       
    public:
        class CustomToolbarComboBox : public ToolbarItemComponent
        {
        public:
            CustomToolbarComboBox (const int toolbarItemId)
            : ToolbarItemComponent (toolbarItemId, "Custom Toolbar Item", false),
            comboBox ("demo toolbar combo box")
            {
                ToolbarItemComponent::addAndMakeVisible (comboBox);

                for (int i = 1; i < 20; ++i)
                    comboBox.addItem ("Toolbar ComboBox item " + String (i), i);

                comboBox.setSelectedId (1);
                comboBox.setEditableText (true);
            }

            bool getToolbarItemSizes (int /*toolbarDepth*/, bool isVertical,
                                      int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;

                preferredSize = 250;
                minSize = 80;
                maxSize = 300;
                return true;
            }

            void paintButtonArea (Graphics&, int, int, bool, bool) override
            {
            }

            void contentAreaChanged (const Rectangle<int>& newArea) override
            {
                comboBox.setSize (newArea.getWidth() - 2,
                                  jmin (newArea.getHeight() - 2, 22));
                
                comboBox.setCentrePosition (newArea.getCentreX(), newArea.getCentreY());
            }
            
        ComboBox comboBox;
        private:
//            ComboBox comboBox;
//            ViewerFrame *pViewerFrame;
        };
        ViewerFrame *pViewerFrame;
    };

    DemoToolbarItemFactory factory;
    DemoToolbarItemFactory::CustomToolbarComboBox *pCCB;
    
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViewerFrame)
};

#endif  // ViewerFrame_H_INCLUDED
