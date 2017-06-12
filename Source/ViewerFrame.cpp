/*
  ==============================================================================

    ViewerFrame.cpp
    Created: 17 Jan 2017 12:59:58pm
    Author:  ChrisGr

  ==============================================================================
*/

#include "ViewerFrame.h"
//#include "PluginProcessor.h"
//==============================================================================
ViewerFrame::ViewerFrame (MIDIProcessor *p) :
noteViewer(p),
processor(p),
factory(this)
{
//    std::cout << "ViewerComponent isBroughtToFrontOnMouseClick " << isBroughtToFrontOnMouseClick()  <<"\n";
//    addKeyListener(this);
    noteViewer.addChangeListener(this);
    p->addChangeListener(this);
//    p->sequenceObject.addChangeListener(this);
    addAndMakeVisible(noteViewer);
    noteViewer.setOpaque(true);
//    std::cout << "ViewerComponent hasKeyboardFocus " <<  hasKeyboardFocus(true) <<"\n";
    setWantsKeyboardFocus (true);
//    std::cout << "ViewerComponent hasKeyboardFocus " <<  hasKeyboardFocus(true) <<"\n";
//    grabKeyboardFocus();
    
//    addAndMakeVisible (resizer = new ResizableCornerComponent (this, &resizeLimits));
//    resizeLimits.setSizeLimits (150, 300, 1600, 1000);
    addAndMakeVisible(toolbar);
    toolbar.addDefaultItems (factory);
    factory.addChangeListener(this);
    
    for (int i=0; i<toolbar.getNumItems(); i++)
    {
        std::cout <<i<< "Listener " << toolbar.getItemComponent(i) <<"\n";
        int id = toolbar.getItemId(i);
        if (id == DemoToolbarItemFactory::DemoToolbarItemIds::customComboBox)
            pCCB = (DemoToolbarItemFactory::CustomToolbarComboBox *) toolbar.getItemComponent(i);
        else
            toolbar.getItemComponent(i)->addListener(this);
    }
    
    //button1Info = new ToolbarButton
//    ToolbarItemComponent comp = toolbar.getItemComponent(0);
    
//    addAndMakeVisible (tempoSlider);
//    tempoSlider.setRange (0.1, 3.0, 0.01);
//    tempoSlider.setSliderStyle (Slider::LinearBar);
//    //    tempoSlider.setTextBoxStyle (Slider::NoTextBox, true, 80, 14);
//    tempoSlider.setTextBoxIsEditable(false);
//    tempoSlider.addListener (this);
//    
//    addAndMakeVisible (scaledTempo);
//    scaledTempo.setFont (Font (12.00f, Font::plain));
//    scaledTempo.setJustificationType (Justification::centredRight);
//    scaledTempo.setColour (Label::textColourId, Colours::white);
//    
//    addAndMakeVisible (realtimeTempo);
//    realtimeTempo.setFont (Font (12.00f, Font::plain));
//    realtimeTempo.setJustificationType (Justification::centredRight);
//    realtimeTempo.setColour (Label::textColourId, Colours::white);
//    
//    rewindButton.setButtonText ("Rewind");
//    rewindButton.setTriggeredOnMouseDown (false);
//    rewindButton.addListener (this);
//    addAndMakeVisible (rewindButton);
//    
//    playStopButton.setButtonText ("Play");
//    playStopButton.setTriggeredOnMouseDown (false);
//    playStopButton.addListener (this);
//    addAndMakeVisible (playStopButton);
//    
//    addAndMakeVisible (commandLabel);
//    //    commandLabel.setText ("Command:", dontSendNotification);
//    commandLabel.setFont (Font (12.00f, Font::plain));
//    commandLabel.setJustificationType (Justification::centredRight);
//    commandLabel.setColour (Label::textColourId, Colours::white);
    
    addAndMakeVisible (hoverStepInfo);
    hoverStepInfo.setFont (Font (12.00f, Font::plain));
    hoverStepInfo.setJustificationType (Justification::left);
    hoverStepInfo.setColour (Label::textColourId, Colours::white);
    
//    addAndMakeVisible (fileNameLabel);
//    fileNameLabel.setFont (Font (12.00f, Font::plain));
//    fileNameLabel.setJustificationType (Justification::horizontallyCentred);
//    fileNameLabel.setColour (Label::textColourId, Colours::white);
    
//    addAndMakeVisible (textEditor = new TextEditor ("new text editor"));
//    textEditor->setMultiLine (false);
//    textEditor->setReturnKeyStartsNewLine (false);
//    textEditor->setReadOnly (false);
//    textEditor->setScrollbarsShown (false);
//    textEditor->setCaretVisible (true);
//    textEditor->setFont (Font (11));
//    textEditor->setPopupMenuEnabled (true);
//    textEditor->setText (String());
//    textEditor->addListener (this);
//    textEditor->setBorder((BorderSize<int>) 1);
//    textEditor->setColour(TextEditor::outlineColourId, Colours::black);
    
    playableKeys = "qwertyuiopasdfghjklzxcvbnm;',./[]";
    
//    playingKeys = "kl";
    
    for (int i=0;i<playableKeys
         .length();i++)
keysThatAreDown.add(false);
    
//    setSize (1000,300);
//    std::cout << "ViewerComponent isBroughtToFrontOnMouseClick " << isBroughtToFrontOnMouseClick()  <<"\n";
    startTimer(100);
}

ViewerFrame::~ViewerFrame()
{
//    rewindButton.removeListener (this);
//    playStopButton.removeListener (this);
//    tempoSlider.removeListener(this);
//    textEditor = nullptr;
}

void focusGained (ScrollingNoteViewer::FocusChangeType cause)
{
//    std::cout << "ViewerComponent gained focus" <<"\n";
}

void focusLost (ScrollingNoteViewer::FocusChangeType cause)
{
//    std::cout << "ViewerComponent lost focus" <<"\n";
}

void ViewerFrame::timerCallback()
{
    if(processor->sequenceObject.propertiesChanged)
    {
//        std::cout << "Properties changed" <<"\n";
        String txt = processor->sequenceObject.getScoreFileName();
//        fileNameLabel.setText(processor->sequenceObject.getScoreFileName(), dontSendNotification);
//        tempoSlider.setValue(processor->sequenceObject.getTempoMultiplier(), dontSendNotification);
        repaint();
        processor->sequenceObject.propertiesChanged = false;
    }
//    static juce::Value selectedId;
//    if (selectedId != pCCB->comboBox.getSelectedId())
//        std::cout << "getSelectedId" <<pCCB->comboBox.getSelectedId()<<"\n";
//    selectedId = pCCB->comboBox.getSelectedId();
//    scaledTempo.setText(String(processor->getTempo(),1), dontSendNotification);
//    realtimeTempo.setText(String(processor->getRealTimeTempo(),1), dontSendNotification);
}

void ViewerFrame::changeListenerCallback (ChangeBroadcaster* cb)
{
    if (cb == processor)
    {
        String txt = processor->sequenceObject.getScoreFileName();
//        fileNameLabel.setText(processor->sequenceObject.getScoreFileName(), dontSendNotification);
//        tempoSlider.setValue(processor->sequenceObject.getTempoMultiplier(), dontSendNotification);
        repaint();
    }
    else if (cb == &noteViewer)
    {
        //        std::cout << "hoverStep " << viewer.getHoverStep() << "\n";
        hoverStepInfo.setText(noteViewer.getHoverInfo(), dontSendNotification);
    }
}

void ViewerFrame::buttonClicked (Button* button)
{
//    std::cout << "Button Press " << button << "\n";
//    toolbar.getItemComponent(1);
    int i=0;
    for (i=0;i<toolbar.getNumItems();i++)
    {
        if (button == toolbar.getItemComponent(i))
            break;
    }
    
    int id = toolbar.getItemId(i);
    
    if (DemoToolbarItemFactory::DemoToolbarItemIds::doc_open == id)
    {
        std::cout << "open " << "\n";
//        CommandIDs::fileOpen;
    }
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::doc_save == id)
    {
        std::cout << "save " << "\n";
    }
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::doc_saveAs == id)
    {
        std::cout << "saveAs " << "\n";
    }
    
    
//    if (button == &rewindButton)
//    {
//        processor->rewind(0);
//    }
//    else if (button == &playStopButton)
//    {
//        if (processor->playing())
//        {
////            noteViewer.setEditable(true);
//            processor->play(false,"current");
//        }
//        else
//        {
////            noteViewer.setEditable(false);
//            processor->play(true,"current");
//        }
//    }
    unfocusAllComponents();
}

//void ViewerFrame::valueChanged (Value& value)
//{
//    std::cout << "valueChanged " << "\n";
//}

void ViewerFrame::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //TODO Change handling of tempo to use a "tempoMultiplier" property of Sequence that adjusts the instantaneous fixed tempo in the sequence file
    //Have tempoMultiplier be what the tempo slider adjusts
    //Decide how this relates to timeIncrement and variableTimeIncrement and code that behaviour
    //Enable use of all tempo adjustments in the original score file?
//    if (sliderThatWasMoved == &tempoSlider)
//    {
////        Component *comp = getCurrentlyFocusedComponent();
////        std::cout <<"Focus Component " << comp << "\n";
//        processor->sequenceObject.setTempoMultiplier(tempoSlider.getValue(), true);
////        processor->setTimeIncrement(processor->sequenceObject.timeIncrement); //Transfers time increment to processor
//        
//    }
//    unfocusAllComponents();
}

/** Callback when the user selects a different file in the browser. */
void ViewerFrame::selectionChanged() {};

/** Callback when the user clicks on a file in the browser. */
void ViewerFrame::fileClicked (const File& file, const MouseEvent& e)
{
};

/** Callback when the user double-clicks on a file in the browser. */
void ViewerFrame::fileDoubleClicked (const File& file) {
    if (!file.isDirectory())
    {
        String fn = file.getFullPathName();
        processor->sequenceObject.setScoreFile(file);
        processor->buildSequenceAsOf(Sequence::loadFile, Sequence::doNotRetainEdits, 0.0);
        fileBrowser->setVisible(false);
        Rectangle<int> r = noteViewer.getBounds();
        r.setLeft(noteViewer.getKeysWidth());
        r.setWidth(getWidth()-noteViewer.getKeysWidth());
        noteViewer.setBounds(r);
    }
};

void ViewerFrame::browserRootChanged (const File& newRoot) {}; /** Callback when the browser's root folder changes. */

void ViewerFrame::textEditorReturnKeyPressed (TextEditor& editor)
{
    processor->play(false, "");
    StringArray cmd;
    cmd.addTokens(editor.getTextValue().toString(), false);
    if (cmd.size()>0)
    {
        if (cmd[0]=="c") //Chain
        {
            std::cout << "nMeasures " << processor->sequenceObject.measureTimes.size() << "\n";
            double interval;
            if (cmd.size()>1)
                interval = cmd[1].getDoubleValue();

            processor->undoMgr->beginNewTransaction();
            MIDIProcessor::ActionChain* action;
//            if (processor->copyOfSelectedNotes.size()>0)
//            {
                action = new MIDIProcessor::ActionChain(*processor,interval,processor->copyOfSelectedNotes);
                processor->undoMgr->perform(action);
                processor->sequenceObject.setChangedFlag(true);
                processor->catchUp();
                processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getTimeInTicks());
//            }
            
//            processor->sequenceObject.chain(processor->copyOfSelectedNotes, interval);
//            processor->sequenceObject.setChangedFlag(true);
//            processor->catchUp();
//            processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getTimeInTicks());
        }
        if (cmd[0]=="ht") //Set chord note time humanize
        {
            const String amount = cmd.size()>1?cmd[1]:"";
            const double humT = amount.getFloatValue();
            if (0 <= humT)
            {
                processor->sequenceObject.setChordTimeHumanize(humT, true);
                processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getSequenceReadHead());
            }
        }
        if (cmd[0]=="hv") //Set chord note velocity humanize
        {
            const String amount = cmd.size()>1?cmd[1]:"";
            const double humV = amount.getFloatValue();
            if (0 <= humV && humV <= 1.0)
                processor->sequenceObject.setChordVelocityHumanize(humV, false);
            processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getSequenceReadHead());
        }
        else if (cmd[0]=="vr") //Velocity ratio.  Sets exprVelToScoreVelRatio
        {
            if (cmd.size()==2)
            {
                double ratio = cmd[1].getFloatValue();
                if (ratio<=0.0)
                    ratio = 0.0;
                else if (ratio>1.0)
                    ratio = 1.0;
                processor->sequenceObject.setExprVelToOriginalValRatio(ratio, true);
            }
        }
        else if (cmd[0]=="d") //Dump data on some number of steps to console
        {
            int first = 0;
            int last = cmd[2].getIntValue();
            int nn = -1;
            if (cmd.size()>1)
                first = cmd[1].getIntValue();
            if (cmd.size()>2)
                last = cmd[2].getIntValue();
            if (cmd.size()>3)
                nn = cmd[3].getIntValue();
            processor->sequenceObject.dumpData(first, last, nn);
        }
    }
    unfocusAllComponents();
}

//==============================================================================
void ViewerFrame::paint (Graphics& g)
{
//    g.setColour(Colours::grey);
    g.setColour(Colours::purple);
    g.fillRect(0,0,getWidth(),noteViewer.getToolbarHeight()); //Command bar
    g.drawImageAt(noteViewer.getKeysImage(), 0, noteViewer.getToolbarHeight()); //Keyboard
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (getAppProperties().getUserSettings()->getValue ("recentConcertKeyboardistFiles"));
//    fileNameLabel.setText(recentFiles.getFile(0).getFileName(), juce::NotificationType::dontSendNotification);
}

void ViewerFrame::resized()
{
    if (toolbar.isVertical())
        toolbar.setBounds (getLocalBounds().removeFromLeft (noteViewer.getToolbarHeight()));
    else
        toolbar.setBounds (getLocalBounds().removeFromTop  (noteViewer.getToolbarHeight()));
    
    noteViewer.setBounds(noteViewer.getKeysWidth(), noteViewer.getToolbarHeight(),
                     getWidth()-noteViewer.getKeysWidth(), getHeight()-noteViewer.getToolbarHeight());

//    rewindButton.setBounds(62, 0, 60, noteViewer.getToolbarHeight()-1);
//    playStopButton.setBounds(124, 0, 60, noteViewer.getToolbarHeight()-1);
//    tempoSlider.setBounds(186, 0, 200, noteViewer.getToolbarHeight()-1);
//    scaledTempo.setBounds(385, 1, 40, noteViewer.getToolbarHeight()-1);
//    realtimeTempo.setBounds(425, 1, 40, noteViewer.getToolbarHeight()-1);
//    commandLabel.setBounds(464,1,50,13);
//    textEditor->setBounds (510,1,150, noteViewer.getToolbarHeight()-1);
    hoverStepInfo.setBounds(getWidth()-350, 0, 180, noteViewer.getToolbarHeight()-1);
//    fileNameLabel.setBounds(getWidth()-250, 0, 280, noteViewer.getToolbarHeight()-1);
}
