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
    toolbar.setColour(Toolbar::ColourIds::backgroundColourId, Colours::lightgrey);
    factory.addChangeListener(this);
    
    for (int i=0; i<toolbar.getNumItems(); i++)
    {
//        std::cout <<i<< "Listener " << toolbar.getItemComponent(i) <<"\n";
        int id = toolbar.getItemId(i);
        if (id == DemoToolbarItemFactory::DemoToolbarItemIds::chainAmountBox)
            pChainAmountBox = (DemoToolbarItemFactory::ChainAmountBox *) toolbar.getItemComponent(i);
        else if (id == DemoToolbarItemFactory::DemoToolbarItemIds::tempoMultiplier)
            pTempoMultiplier = (DemoToolbarItemFactory::TempoMultiplier *) toolbar.getItemComponent(i);
        else if (id == DemoToolbarItemFactory::DemoToolbarItemIds::realTimeTempo)
            pRealTimeTempo = (DemoToolbarItemFactory::RealTimeTempo *) toolbar.getItemComponent(i);
        else if (id == DemoToolbarItemFactory::DemoToolbarItemIds::scoreTempo)
            pScoreTempo = (DemoToolbarItemFactory::ScoreTempo *) toolbar.getItemComponent(i);
        else if (id == DemoToolbarItemFactory::DemoToolbarItemIds::humTimeBox)
            pHumanizeStartTime = (DemoToolbarItemFactory::ChainAmountBox *) toolbar.getItemComponent(i);
        else if (id == DemoToolbarItemFactory::DemoToolbarItemIds::humVelocityBox)
            pHumanizeVelocity = (DemoToolbarItemFactory::ChainAmountBox *) toolbar.getItemComponent(i);
        else
            toolbar.getItemComponent(i)->addListener(this);
    }
    
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
        std::cout << "chainAmount" <<pChainAmountBox->textBox.getText().getDoubleValue()<<"\n";
        chainAmount = pChainAmountBox->textBox.getText().getDoubleValue();
        sendActionMessage("chain:"+String(chainAmount));
        grabKeyboardFocus();
        pChainAmountBox->returnPressed = false;
    }
    
    if (pHumanizeVelocity->returnPressed)
    {
        std::cout << "HumanizeVelocity " <<pHumanizeVelocity->textBox.getText().getDoubleValue()<<"\n";
        humanizeVelocityAmount = pHumanizeVelocity->textBox.getText().getDoubleValue();
        sendActionMessage("humanizeVelocity:"+String(humanizeVelocityAmount));
        grabKeyboardFocus();
        pHumanizeVelocity->returnPressed = false;
    }
    
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
//        if (!processor->playing())
//            pRealTimeTempo->setValue(std::round(processor->getRealTimeTempo()));
        pRealTimeTempo->setValue(std::round(scoreTempo*processor->sequenceObject.getTempoMultiplier()));
    }
//    double rtt = std::round(processor->getRealTimeTempo());
//    std::cout << "rtt " <<rtt<<"\n";
//    pRealTimeTempo->setValue(rtt);
//    pTempoMultiplier->setValue(processor->sequenceObject.getTempoMultiplier());
//    std::cout << "TempoMultiplier " <<processor->sequenceObject.getTempoMultiplier()<<"\n";
//    if (!processor->playing())
//        pRealTimeTempo->setValue(std::round(processor->getRealTimeTempo()));
    pScoreTempo->textBox.setText(String(std::round(scoreTempo)));
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
    
//    _play           = 9,
//    _stop           = 10,
//    _playPause      = 11,
//    _rewind         = 12,
//    _listen         = 13,
    
//    CommandIDs::fileOpen - doc_open
//    CommandIDs::fileSave - doc_save
//    CommandIDs::fileSaveAs - doc_saveAs
//    CommandIDs::editUndo - edit_undo
//    CommandIDs::editRedo -edit_redo
    
//    CommandIDs::playPause _play, _stop
//    CommandIDs::playFromCurrentPlayhead,
//    CommandIDs::listenToSelection - _listen
//    CommandIDs::increaseTempo,
//    CommandIDs::decreaseTempo,

    
//    CommandIDs::toggleSelectedNotesActive
//    CommandIDs::setSelectedNotesActive
//    CommandIDs::setSelectedNotesInactive
//    CommandIDs::chainSelectedNotes - _chain
//    CommandIDs::velHumanizeSelection,
//    CommandIDs::timeHumanizeSelection,
//    CommandIDs::rewind - _rewind
    
//    CommandIDs::toggleBookmark,
//    CommandIDs::previousBookmark,
//    CommandIDs::nextBookmark
    
    if (DemoToolbarItemFactory::DemoToolbarItemIds::doc_open == id)
        sendActionMessage("fileOpen");
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::doc_save == id)
        sendActionMessage("fileSave");
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::doc_saveAs == id)
        sendActionMessage("fileSaveAs");
    
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::edit_undo == id)
        sendActionMessage("editUndo");
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::edit_redo == id)
        sendActionMessage("editRedo");
    
    
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_play == id)
        sendActionMessage("play");
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_stop == id)
        sendActionMessage("pause");
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_rewind == id)
        sendActionMessage("rewind");
    
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_listen == id)
        sendActionMessage("listenToSelection");
    
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_makeActive == id)
        sendActionMessage("setSelectedNotesActive");
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_makeInactive == id)
        sendActionMessage("setSelectedNotesInactive");
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_chain == id)
        sendActionMessage("chain:"+String(chainAmount));
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_humanizeTime == id)
        sendActionMessage("humanizeTime:"+String(chainAmount));
    else if(DemoToolbarItemFactory::DemoToolbarItemIds::_humanizeVel == id)
        sendActionMessage("humanizeVel:"+String(chainAmount));
    
    
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
    g.setColour(Colours::purple);
    g.fillRect(0,0,getWidth(),noteViewer.getToolbarHeight()); //Command bar
    g.drawImageAt(noteViewer.getKeysImage(), 0, noteViewer.getToolbarHeight()); //Keyboard
}

void ViewerFrame::resized()
{
    if (toolbar.isVertical())
        toolbar.setBounds (getLocalBounds().removeFromLeft (noteViewer.getToolbarHeight()));
    else
        toolbar.setBounds (getLocalBounds().removeFromTop  (noteViewer.getToolbarHeight()));
    
    noteViewer.setBounds(noteViewer.getKeysWidth(), noteViewer.getToolbarHeight(),
                     getWidth()-noteViewer.getKeysWidth(), getHeight()-noteViewer.getToolbarHeight());
    
    hoverStepInfo.setBounds(getWidth()-380, 0, 340, noteViewer.getToolbarHeight()-1);
}
