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
    mainFactory.pViewer = &noteViewer;
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
            pChainAmountBox->setWidth(35);
            pChainAmountBox->textBox.setText("2.0");
            chainAmount = 120.0;
        }
        else if (id == MainToolbarItemFactory::ToolbarItemIds::_humanizeTimeBox)
        {
            
            pHumanizeStartTime =
            (MainToolbarItemFactory::ChainAmountBox *) mainToolbar.getItemComponent(i);
            pHumanizeStartTime->setWidth(65);
            pHumanizeStartTime->textBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey));
            pHumanizeStartTime->textBox.setText("50");
            humanizeTimeAmount = "50";
        }
        else if (id == MainToolbarItemFactory::ToolbarItemIds::_humanizeVelocityBox)
        {
            
            pHumanizeVelocity =
            (MainToolbarItemFactory::ChainAmountBox *) mainToolbar.getItemComponent(i);
            pHumanizeVelocity->setWidth(65);
            pHumanizeVelocity->textBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey));
            pHumanizeVelocity->textBox.setText(".6,.8");
            humanizeVelocityAmount = ".6,.8";
        }
        else
            mainToolbar.getItemComponent(i)->addListener(this);
    }
    for (int i=0; i<altToolbar.getNumItems(); i++)
    {
        int id = altToolbar.getItemId(i);
        if (id == AltToolbarItemFactory::ToolbarItemIds::tempoMultiplier)
        {
            pTempoMultiplier = (AltToolbarItemFactory::TempoMultiplier *)altToolbar.getItemComponent(i);
            pTempoMultiplier->numberBox.setFont (Font (19.00f, Font::bold));
            pTempoMultiplier->numberBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey).brighter());
        }
        else if (id == AltToolbarItemFactory::ToolbarItemIds::scoreTempo)
        {
            pScoreTempo = (AltToolbarItemFactory::ScoreTempo *) altToolbar.getItemComponent(i);
        }
        else if (id == AltToolbarItemFactory::ToolbarItemIds::scaledTempo)
        {
            pScaledTempo = (AltToolbarItemFactory::ScaledTempo *) altToolbar.getItemComponent(i);
        }
        altToolbar.getItemComponent(i)->addListener(this);
    }
    addAndMakeVisible (hoverStepInfo);
    hoverStepInfo.setFont (Font (15.00f, Font::bold));
    hoverStepInfo.setJustificationType (Justification::left);
    hoverStepInfo.setColour (Label::textColourId, Colours::darkgrey);
    
    scoreTempoLabel.setText("Score Tempo",NotificationType::dontSendNotification);
    scoreTempoLabel.setFont (Font (14.00f, Font::bold));
    scoreTempoLabel.setJustificationType (Justification::right);
    scoreTempoLabel.setColour (Label::textColourId, Colours::darkgrey);
    addAndMakeVisible (scoreTempoLabel);
    
    scaledTempoLabel.setText("Scaled Tempo",NotificationType::dontSendNotification);
    scaledTempoLabel.setFont (Font (14.00f, Font::bold));
    scaledTempoLabel.setJustificationType (Justification::right);
    scaledTempoLabel.setColour (Label::textColourId, Colours::darkgrey);
    addAndMakeVisible (scaledTempoLabel);
    
    scaleFactorLabel.setText("Scale Factor",NotificationType::dontSendNotification);
    scaleFactorLabel.setFont (Font (14.00f, Font::bold));
    scaleFactorLabel.setJustificationType (Justification::right);
    scaleFactorLabel.setColour (Label::textColourId, Colours::darkgrey);
    addAndMakeVisible (scaleFactorLabel);
    
    playableKeys = "qwertyuiopasdfghjklzxcvbnm;',./[]";

    for (int i=0;i<playableKeys
         .length();i++)
keysThatAreDown.add(false);
    
//    std::cout << "ViewerComponent isBroughtToFrontOnMouseClick " << isBroughtToFrontOnMouseClick()  <<"\n";
    startTimer(200);
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
//    if(processor->sequenceObject.propertiesChanged)
//    {
        String plugin;
        if (processor->sequenceObject.thePlugin)
        {
            plugin = " playing "+processor->sequenceObject.thePlugin->getName();
            int progNum = processor->sequenceObject.thePlugin->getCurrentProgram();
            if (progNum>=0)
                plugin = plugin + " (" +processor->sequenceObject.thePlugin->getProgramName(progNum)+")";
        }
        String txt = processor->sequenceObject.getScoreFileName();
        getTopLevelComponent()->setName ("Concert Keyboardist - " + processor->sequenceObject.getScoreFileName() + plugin);
//        repaint();
        processor->sequenceObject.propertiesChanged = false;
//    }
    
    if (pChainAmountBox->returnPressed)
    {
        std::cout << "Return pressed - chainAmount" <<pChainAmountBox->textBox.getText().getDoubleValue()<<"\n";
        chainAmount = pChainAmountBox->textBox.getText().getDoubleValue()*60.0;
        sendActionMessage("chain:"+String(chainAmount));
        grabKeyboardFocus();
        pChainAmountBox->returnPressed = false;
    }
    
    if (pHumanizeStartTime->returnPressed)
    {
        //        std::cout << "HumanizeStartTime " <<pHumanizeStartTime->textBox.getText().getDoubleValue()<<"\n";
        //        humanizeTimeAmount = pHumanizeStartTime->textBox.getText();
        sendActionMessage("humanizeTime:"+pHumanizeStartTime->textBox.getText());
        grabKeyboardFocus();
        pHumanizeStartTime->returnPressed = false;
    }
    if (pHumanizeVelocity->returnPressed)
    {
        std::cout << "Return pressed - HumanizeVelocity " <<pHumanizeVelocity->textBox.getText().getDoubleValue()<<"\n";
        String temp = pHumanizeVelocity->textBox.getText();
        humanizeVelocityAmount.clear();
        for (int i=0;i<temp.length();i++)
            if (temp.substring(i,i+1).containsAnyOf(".0123456789,"))
                humanizeVelocityAmount.append(temp.substring(i,i+1), 1);
                
        sendActionMessage("humanizeVelocity:"+String(humanizeVelocityAmount));
        grabKeyboardFocus();
        pHumanizeVelocity->returnPressed = false;
    }

    if (pTempoMultiplier->changed)
    {
        std::cout << "pTempoMultiplier->changed " << pTempoMultiplier->numberBox.getText().getDoubleValue()/100.0 << "\n";
        processor->setTempoMultiplier(pTempoMultiplier->numberBox.getText().getDoubleValue()/100.0,
                                                     processor->getZTLTime(noteViewer.horizontalShift), true);
        pTempoMultiplier->changed = false;
        grabKeyboardFocus();
    }
    else
    {
//        pTempoMultiplier->setValue(std::round(processor->sequenceObject.getTempoMultipier(processor->getTimeInTicks())*100.0));
    }
    
    if (pScoreTempo->returnPressed)
        ;
    else
    {
//        double scoreTempo = processor->sequenceObject.getTempo(processor->getTimeInTicks(),
//                                                               processor->sequenceObject.tempoChanges);
//        if (!pScoreTempo->textBox.hasKeyboardFocus(true))
//            pScoreTempo->textBox.setText(String(std::round(scoreTempo)));
    }
    if (pScaledTempo->returnPressed)
        ;
    else
    {
//        double scaledTempo = processor->sequenceObject.getTempo(processor->getTimeInTicks(),
//                                                                processor->sequenceObject.scaledTempoChanges);
//        if (!pScaledTempo->textBox.hasKeyboardFocus(true))
//            pScaledTempo->textBox.setText(String(std::round(scaledTempo)));
    }
}

void ViewerFrame::changeListenerCallback (ChangeBroadcaster* cb)
{
    if (cb == processor)
    {
        String txt = processor->sequenceObject.getScoreFileName();
//        std::cout << "ViewerFrame::CLB tempoMultipier " << std::round(processor->sequenceObject.
//                                                                          getTempoMultipier(processor->getZTLTime(noteViewer.horizontalShift))*100.0) << "\n";
        
        const double scaledTempo = processor->sequenceObject.getTempo(processor->getZTLTime(noteViewer.horizontalShift),
                                                                processor->sequenceObject.scaledTempoChanges);
        if (!pScaledTempo->textBox.hasKeyboardFocus(true))
            pScaledTempo->textBox.setText(String(std::round(scaledTempo)));
        const double scoreTempo = processor->sequenceObject.getTempo(processor->getZTLTime(noteViewer.horizontalShift),
                                                               processor->sequenceObject.tempoChanges);
        if (!pScoreTempo->textBox.hasKeyboardFocus(true))
            pScoreTempo->textBox.setText(String(std::round(scoreTempo)));
        
        pTempoMultiplier->setValue(std::round(processor->sequenceObject.
                                              getTempoMultipier(processor->getZTLTime(noteViewer.horizontalShift))*100.0));
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
    bool inMainToolbar=false;
    int i=0;
    for (i=0;i<mainToolbar.getNumItems();i++)
    {
        if (button == mainToolbar.getItemComponent(i))
        {
            inMainToolbar=true;
            break;
        }
    }
    if (!inMainToolbar)
        for (i=0;i<altToolbar.getNumItems();i++)
        {
            if (button == altToolbar.getItemComponent(i))
                break;
        }
    if (inMainToolbar)
    {
        int id = mainToolbar.getItemId(i);
        if (MainToolbarItemFactory::ToolbarItemIds::doc_open == id)
        {
            sendActionMessage("fileOpen");
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::doc_save == id)
            sendActionMessage("fileSave");
        else if(MainToolbarItemFactory::ToolbarItemIds::doc_saveAs == id)
            sendActionMessage("fileSaveAs");
        else if(MainToolbarItemFactory::ToolbarItemIds::edit_undo == id)
            sendActionMessage("editUndo");
        else if(MainToolbarItemFactory::ToolbarItemIds::edit_redo == id)
            sendActionMessage("editRedo");
        else if(MainToolbarItemFactory::ToolbarItemIds::_toggleActivity == id)
            sendActionMessage("toggleActivity");
//        else if(MainToolbarItemFactory::ToolbarItemIds::_marqueeSelectionAdd == id)
//            sendActionMessage("marqueeSelectionAdd");
//        else if(MainToolbarItemFactory::ToolbarItemIds::_marqueeSelectionRemove == id)
//            sendActionMessage("marqueeSelectionRemove");
//        else if(MainToolbarItemFactory::ToolbarItemIds::_markSelectedNotes == id)
//            sendActionMessage("markSelectedNote");
//        else if(MainToolbarItemFactory::ToolbarItemIds::_clearSelectedNotes == id)
//            sendActionMessage("clearSelectedNotes");
//        else if(MainToolbarItemFactory::ToolbarItemIds::_clearAllSelection == id)
//            sendActionMessage("clearAllSelection");
        else if(MainToolbarItemFactory::ToolbarItemIds::_chain == id)
        {
            chainAmount = pChainAmountBox->textBox.getText().getDoubleValue()*60.0;
            sendActionMessage("chain:"+String(chainAmount));
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_humanizeTime == id)
        {
            sendActionMessage("humanizeTime:"+pHumanizeStartTime->textBox.getText());
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_humanizeVelocity == id)
            sendActionMessage("humanizeVelocity:"+pHumanizeVelocity->textBox.getText());
        else if(MainToolbarItemFactory::ToolbarItemIds::_chordEditToggle == id)
            sendActionMessage("_showChords");
        else if(MainToolbarItemFactory::ToolbarItemIds::_showVelocities == id)
        {
//            if (noteViewer.editingVelocities.getValue())
//            {
//                pMarqueeSelectionAdd->setToggleState(false, juce::NotificationType::dontSendNotification);
//                noteViewer.marqueeAddingNotes = false;
//                pMarqueeSelectionRemove->setToggleState(false, juce::NotificationType::dontSendNotification);
//                noteViewer.marqueeRemovingNotes = false;
//                pClearSelectedNotes->setToggleState(false, juce::NotificationType::dontSendNotification);
//                noteViewer.clearingSelectedNotes = false;
//                pMarkSelectedNotes->setToggleState(false, juce::NotificationType::dontSendNotification);
//                noteViewer.markingSelectedNotes = false;
//            }
            noteViewer.repaint();
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
        else if (MainToolbarItemFactory::ToolbarItemIds::create_chord == id)
        {
            std::cout << "create_chord\n";
            sendActionMessage("create_chord");
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::delete_chord == id)
        {
            std::cout << "delete_chord\n";
            sendActionMessage("delete_chord");
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_humanizeTime == id)
        {
            sendActionMessage("humanizeTime:"+String(pHumanizeStartTime->textBox.getText()));
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::_help == id)
        {
            sendActionMessage("help");
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::loadPlugin == id)
        {
            sendActionMessage("loadPluginMenu");
        }
        else if(MainToolbarItemFactory::ToolbarItemIds::editPlugin == id)
            sendActionMessage("editPlugin");
    }
    else //in alt toolbar
    {
        int id = altToolbar.getItemId(i);
        if(AltToolbarItemFactory::ToolbarItemIds::_play == id)
            sendActionMessage("play");
        else if(AltToolbarItemFactory::ToolbarItemIds::_stop == id)
            sendActionMessage("pause");
        else if(AltToolbarItemFactory::ToolbarItemIds::_rewind == id)
            sendActionMessage("rewind");
        else if(AltToolbarItemFactory::ToolbarItemIds::_listen == id)
            sendActionMessage("listenToSelection");
        else if(AltToolbarItemFactory::ToolbarItemIds::saveTempoChange == id)
        {
            std::cout << "Save tempo change\n";//sendActionMessage("listenToSelection");
            processor->catchUp(false);
            processor->addRemoveBookmark(BOOKMARK_ADD,true,pTempoMultiplier->numberBox.getText().getDoubleValue()/100.0);
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
    
    altToolbarVisible = true;
    if (altToolbarVisible)
    {
        if (altToolbar.isVertical())
            ;//altToolbar.setBounds (getLocalBounds().removeFromLeft (noteViewer.getToolbarHeight()));
        else
        {
            Rectangle<int> shifted = getLocalBounds();//.removeFromBottom(noteViewer.getToolbarHeight());
            shifted.setHeight(noteViewer.getToolbarHeight());
//            shifted.setTop(getLocalBounds().getHeight()-noteViewer.getToolbarHeight());
            shifted.translate(0,  getLocalBounds().getHeight()-noteViewer.getToolbarHeight());
            altToolbar.setBounds (shifted);
        }
    }
    
//    int tbHeightMultiplier = altToolbarVisible?2:1;
    noteViewer.setBounds(noteViewer.getKeysWidth(), noteViewer.getToolbarHeight(),
                     getWidth()-noteViewer.getKeysWidth(), getHeight()-noteViewer.getToolbarHeight()*2);
    
    scoreTempoLabel.setBounds(22, getHeight()-30, 80, noteViewer.getToolbarHeight()-1);
    scaleFactorLabel.setBounds(138, getHeight()-30, 80, noteViewer.getToolbarHeight()-1);
    scaledTempoLabel.setBounds(276, getHeight()-30, 80, noteViewer.getToolbarHeight()-1);
    hoverStepInfo.setBounds(585, getHeight()-30, 600, noteViewer.getToolbarHeight()-1);
}
