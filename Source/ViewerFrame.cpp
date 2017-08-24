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
mainFactory(this),
altToolbarFactory(this)
{
    noteViewer.addChangeListener(this);
    p->addChangeListener(this);
    addAndMakeVisible(noteViewer);
    noteViewer.setOpaque(true);
    setWantsKeyboardFocus (true);
    
    addAndMakeVisible(mainToolbar);
    mainToolbar.addDefaultItems (mainFactory);
    mainToolbar.setColour(Toolbar::ColourIds::backgroundColourId, Colours::lightgrey);
    mainFactory.addChangeListener(this);

    addAndMakeVisible(altToolbar);
    altToolbar.addDefaultItems (altToolbarFactory);
    altToolbar.setColour(Toolbar::ColourIds::backgroundColourId, Colours::lightgrey);
    altToolbarFactory.addChangeListener(this);
    
    for (int i=0; i<mainToolbar.getNumItems(); i++)
    {
        int id = mainToolbar.getItemId(i);
        if (id == MainToolbarItemFactory::ToolbarItemIds::chainAmountBox)
        {
            pChainAmountBox = (MainToolbarItemFactory::ChainAmountBox *) mainToolbar.getItemComponent(i);
            pChainAmountBox->textBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey));
            pChainAmountBox->textBox.setText("12");
            chainAmount = 12.0;
        }
        else if (id == MainToolbarItemFactory::ToolbarItemIds::tempoMultiplier)
        {
            pTempoMultiplier = (MainToolbarItemFactory::TempoMultiplier *) mainToolbar.getItemComponent(i);
        }
        else if (id == MainToolbarItemFactory::ToolbarItemIds::realTimeTempo)
        {
            pRealTimeTempo = (MainToolbarItemFactory::RealTimeTempo *) mainToolbar.getItemComponent(i);
            pRealTimeTempo->numberBox.setFont (Font (19.00f, Font::bold));
            pRealTimeTempo->numberBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey).brighter());
        }
        else if (id == MainToolbarItemFactory::ToolbarItemIds::scoreTempo)
        {
            pScoreTempo = (MainToolbarItemFactory::ScoreTempo *) mainToolbar.getItemComponent(i);
        }
        else
            mainToolbar.getItemComponent(i)->addListener(this);
    }
    for (int i=0; i<altToolbar.getNumItems(); i++)
    {
        int id = altToolbar.getItemId(i);
        if (id == AltToolbarItemFactory::ToolbarItemIds::humTimeBox)
        {
            pHumanizeStartTime = (AltToolbarItemFactory::ChainAmountBox *) altToolbar.getItemComponent(i);
            pHumanizeStartTime->textBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey));
            pHumanizeStartTime->textBox.setText("3");
            humanizeTimeAmount = 3.0;
        }
        altToolbar.getItemComponent(i)->addListener(this);
    }
    
    addAndMakeVisible (scoreTempoInfo);
    scoreTempoInfo.setFont (Font (20.00f, Font::bold));
    scoreTempoInfo.setJustificationType (Justification::left);
    scoreTempoInfo.setColour (Label::textColourId, Colours::grey);
    
    addAndMakeVisible (hoverStepInfo);
    hoverStepInfo.setFont (Font (18.00f, Font::plain));
    hoverStepInfo.setJustificationType (Justification::left);
    hoverStepInfo.setColour (Label::textColourId, Colours::darkgrey);
    
    playableKeys = "qwertyuiopasdfghjklzxcvbnm;',./[]";

    for (int i=0;i<playableKeys
         .length();i++)
keysThatAreDown.add(false);
    
//    std::cout << "ViewerComponent isBroughtToFrontOnMouseClick " << isBroughtToFrontOnMouseClick()  <<"\n";
    startTimer(100);
}

ViewerFrame::~ViewerFrame()
{

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
        String txt = processor->sequenceObject.getScoreFileName();
        getTopLevelComponent()->setName ("Concert Keyboardist - " + processor->sequenceObject.getScoreFileName());
        repaint();
        processor->sequenceObject.propertiesChanged = false;
    }
    
    if (pChainAmountBox->returnPressed)
    {
        std::cout << "Return pressed - chainAmount" <<pChainAmountBox->textBox.getText().getDoubleValue()<<"\n";
        chainAmount = pChainAmountBox->textBox.getText().getDoubleValue();
        sendActionMessage("chain:"+String(chainAmount));
        grabKeyboardFocus();
        pChainAmountBox->returnPressed = false;
    }
    
//    if (pHumanizeVelocity->returnPressed)
//    {
//        std::cout << "Return pressed - HumanizeVelocity " <<pHumanizeVelocity->textBox.getText().getDoubleValue()<<"\n";
//        humanizeVelocityAmount = pHumanizeVelocity->textBox.getText().getDoubleValue();
//        sendActionMessage("humanizeVel:"+String(humanizeVelocityAmount));
//        grabKeyboardFocus();
//        pHumanizeVelocity->returnPressed = false;
//    }
    
    if (pHumanizeStartTime->returnPressed)
    {
        std::cout << "HumanizeStartTime " <<pHumanizeStartTime->textBox.getText().getDoubleValue()<<"\n";
        humanizeTimeAmount = pHumanizeStartTime->textBox.getText().getDoubleValue();
        sendActionMessage("humanizeTime:"+String(humanizeTimeAmount));
        grabKeyboardFocus();
        pHumanizeStartTime->returnPressed = false;
    }
    double scoreTempo = processor->sequenceObject.getTempo(processor->getTimeInTicks());
    if (pRealTimeTempo->changed)
    {
        processor->sequenceObject.setTempoMultiplier(pRealTimeTempo->numberBox.getText().getDoubleValue()/scoreTempo, true);
//        std::cout << "tempoChanged " <<pRealTimeTempo->numberBox.getText().getDoubleValue()
//        << " setMult "<<pRealTimeTempo->numberBox.getText().getDoubleValue()/scoreTempo<<"\n";
        pRealTimeTempo->changed = false;
        grabKeyboardFocus();
    }
    else
    {
        pRealTimeTempo->setValue(std::round(scoreTempo*processor->sequenceObject.getTempoMultiplier()));
    }
//    double rtt = std::round(processor->getRealTimeTempo());
//    std::cout << "rtt " <<rtt<<"\n";
//    pRealTimeTempo->setValue(rtt);
//    pTempoMultiplier->setValue(processor->sequenceObject.getTempoMultiplier());
//    std::cout << "TempoMultiplier " <<processor->sequenceObject.getTempoMultiplier()<<"\n";
//    if (!processor->playing())
//        pRealTimeTempo->setValue(std::round(processor->getRealTimeTempo()));
//    pScoreTempo->textBox.setText(String(std::round(scoreTempo)));
    scoreTempoInfo.setText(String(std::round(scoreTempo)), dontSendNotification);
    
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
    for (i=0;i<mainToolbar.getNumItems();i++)
    {
        if (button == mainToolbar.getItemComponent(i))
            break;
    }
    if (i==mainToolbar.getNumItems()) //If not in main toolbar
        for (i=0;i<altToolbar.getNumItems();i++)
        {
            if (button == altToolbar.getItemComponent(i))
                break;
        }
    int id = mainToolbar.getItemId(i);
    if (button->getParentComponent() == (juce::Component*) &mainToolbar)
    {
        if (MainToolbarItemFactory::ToolbarItemIds::doc_open == id)
            sendActionMessage("fileOpen");
        else if(MainToolbarItemFactory::ToolbarItemIds::doc_save == id)
            sendActionMessage("fileSave");
        else if(MainToolbarItemFactory::ToolbarItemIds::doc_saveAs == id)
            sendActionMessage("fileSaveAs");
        
        else if(MainToolbarItemFactory::ToolbarItemIds::edit_undo == id)
            sendActionMessage("editUndo");
        else if(MainToolbarItemFactory::ToolbarItemIds::edit_redo == id)
            sendActionMessage("editRedo");
        else if(MainToolbarItemFactory::ToolbarItemIds::_play == id)
            sendActionMessage("play");
        else if(MainToolbarItemFactory::ToolbarItemIds::_stop == id)
            sendActionMessage("pause");
        else if(MainToolbarItemFactory::ToolbarItemIds::_rewind == id)
            sendActionMessage("rewind");
        else if(MainToolbarItemFactory::ToolbarItemIds::_listen == id)
            sendActionMessage("listenToSelection");
        else if(MainToolbarItemFactory::ToolbarItemIds::_makeActive == id)
            sendActionMessage("setSelectedNotesActive");
        else if(MainToolbarItemFactory::ToolbarItemIds::_makeInactive == id)
            sendActionMessage("setSelectedNotesInactive");
        else if(MainToolbarItemFactory::ToolbarItemIds::_chain == id)
        {
            chainAmount = pChainAmountBox->textBox.getText().getDoubleValue();
            sendActionMessage("chain:"+String(chainAmount));
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_humanizeTime == id)
        {
            humanizeTimeAmount = pHumanizeStartTime->textBox.getText().getDoubleValue();
            sendActionMessage("humanizeTime:"+String(humanizeTimeAmount));
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_chordEditToggle == id)
        {
            altToolbarVisible = !altToolbarVisible;
            noteViewer.showingChords = altToolbarVisible;
            altToolbar.setVisible(altToolbarVisible);
            resized();
            repaint();
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_editVelocities == id)
        {
            std::cout << "_editVelocities\n";
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_addSustain == id)
        {
            std::cout << "addSustain\n";
            sendActionMessage("addSustain");
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_deleteSustain == id)
        {
            std::cout << "deleteSustain\n";
            sendActionMessage("deleteSustain");
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_addSoft == id)
        {
            std::cout << "addSoft\n";
            sendActionMessage("addSoft");
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_deleteSoft == id)
        {
            std::cout << "deleteSoft\n";
            sendActionMessage("deleteSoft");
        }
    }
    else if (button->getParentComponent() == (juce::Component*) &altToolbar)
    {
        if (AltToolbarItemFactory::ToolbarItemIds::create_chord == id)
        {
            std::cout << "create_chord\n";
            sendActionMessage("create_chord");
        }
        else if(AltToolbarItemFactory::ToolbarItemIds::delete_chord == id)
        {
            std::cout << "delete_chord\n";
            sendActionMessage("delete_chord");
        }
        else if(AltToolbarItemFactory::ToolbarItemIds::_humanizeTime == id)
        {
            humanizeTimeAmount = pHumanizeStartTime->textBox.getText().getDoubleValue();
            sendActionMessage("humanizeTime:"+String(humanizeTimeAmount));
        }
    }
    unfocusAllComponents();
}

void ViewerFrame::sliderValueChanged (Slider* sliderThatWasMoved)
{
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

//void ViewerFrame::textEditorReturnKeyPressed (TextEditor& editor)
//{
//    processor->play(false, "");
//    StringArray cmd;
//    cmd.addTokens(editor.getTextValue().toString(), false);
//    if (cmd.size()>0)
//    {
//        if (cmd[0]=="c") //Chain
//        {
//            std::cout << "nMeasures " << processor->sequenceObject.measureTimes.size() << "\n";
//            double interval;
//            if (cmd.size()>1)
//                interval = cmd[1].getDoubleValue();
//
//            processor->undoMgr->beginNewTransaction();
//            MIDIProcessor::ActionChain* action;
////            if (processor->copyOfSelectedNotes.size()>0)
////            {
//                action = new MIDIProcessor::ActionChain(*processor,interval,processor->copyOfSelectedNotes);
//                processor->undoMgr->perform(action);
//                processor->sequenceObject.setChangedFlag(true);
//                processor->catchUp();
//                processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getTimeInTicks());
////            }
//            
////            processor->sequenceObject.chain(processor->copyOfSelectedNotes, interval);
////            processor->sequenceObject.setChangedFlag(true);
////            processor->catchUp();
////            processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getTimeInTicks());
//        }
//        if (cmd[0]=="ht") //Set chord note time humanize
//        {
//            const String amount = cmd.size()>1?cmd[1]:"";
//            const double humT = amount.getFloatValue();
//            if (0 <= humT)
//            {
//                processor->sequenceObject.setChordTimeHumanize(humT, true);
//                processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getSequenceReadHead());
//            }
//        }
//        if (cmd[0]=="hv") //Set chord note velocity humanize
//        {
//            const String amount = cmd.size()>1?cmd[1]:"";
//            const double humV = amount.getFloatValue();
//            if (0 <= humV && humV <= 1.0)
//                processor->sequenceObject.setChordVelocityHumanize(humV, false);
//            processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getSequenceReadHead());
//        }
//        else if (cmd[0]=="vr") //Velocity ratio.  Sets exprVelToScoreVelRatio
//        {
//            if (cmd.size()==2)
//            {
//                double ratio = cmd[1].getFloatValue();
//                if (ratio<=0.0)
//                    ratio = 0.0;
//                else if (ratio>1.0)
//                    ratio = 1.0;
//                processor->sequenceObject.setExprVelToOriginalValRatio(ratio, true);
//            }
//        }
//        else if (cmd[0]=="d") //Dump data on some number of steps to console
//        {
//            int first = 0;
//            int last = cmd[2].getIntValue();
//            int nn = -1;
//            if (cmd.size()>1)
//                first = cmd[1].getIntValue();
//            if (cmd.size()>2)
//                last = cmd[2].getIntValue();
//            if (cmd.size()>3)
//                nn = cmd[3].getIntValue();
//            processor->sequenceObject.dumpData(first, last, nn);
//        }
//    }
//    unfocusAllComponents();
//}

//==============================================================================
void ViewerFrame::paint (Graphics& g)
{
    g.setColour(Colours::purple);
    g.fillRect(0,0,getWidth(),noteViewer.getToolbarHeight()); //Command bar
    g.drawImageAt(noteViewer.getKeysImage(), 0, noteViewer.getToolbarHeight()); //Keyboard
}

void ViewerFrame::resized()
{
    if (mainToolbar.isVertical())
        ;//mainToolbar.setBounds (getLocalBounds().removeFromLeft (noteViewer.getToolbarHeight()));
    else
        mainToolbar.setBounds (getLocalBounds().removeFromTop  (noteViewer.getToolbarHeight()));
    
    if (altToolbarVisible)
    {
        if (altToolbar.isVertical())
            ;//altToolbar.setBounds (getLocalBounds().removeFromLeft (noteViewer.getToolbarHeight()));
        else
        {
            Rectangle<int> shifted = getLocalBounds().removeFromTop  (noteViewer.getToolbarHeight());
            shifted.translate(0,noteViewer.getToolbarHeight());
            altToolbar.setBounds (shifted);
        }
    }
    
    int tbHeightMultiplier = altToolbarVisible?2:1;
    noteViewer.setBounds(noteViewer.getKeysWidth(), noteViewer.getToolbarHeight()*tbHeightMultiplier,
                     getWidth()-noteViewer.getKeysWidth(), getHeight()-noteViewer.getToolbarHeight()*tbHeightMultiplier);
    hoverStepInfo.setBounds(95, 0, 340, noteViewer.getToolbarHeight()-1);
    scoreTempoInfo.setBounds(95+340+16, 1, 40, noteViewer.getToolbarHeight()-1);
}
