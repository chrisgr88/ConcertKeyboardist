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
    mainToolbar.setColour(Toolbar::ColourIds::backgroundColourId, Colour(48,48,48));
    mainToolbar.setVisible(getAppProperties().getUserSettings()->getValue ("editToolbarVisible") == "true" ? true : false);
    mainFactory.addChangeListener(this);

    addAndMakeVisible(altToolbar);
    altToolbar.addDefaultItems (altToolbarFactory);
    altToolbar.setColour(Toolbar::ColourIds::backgroundColourId, Colour(48,48,48));
    altToolbarFactory.addChangeListener(this);
    
    for (int i=0; i<mainToolbar.getNumItems(); i++)
    {
        int id = mainToolbar.getItemId(i);
        if (id == MainToolbarItemFactory::ToolbarItemIds::customComboBox)
        {
            pChainAmountComboBox = (MainToolbarItemFactory::NumberComboBox *) mainToolbar.getItemComponent(i);
        }
        else if (id == MainToolbarItemFactory::ToolbarItemIds::_humanizeTimeBox)
        {
            
            pHumanizeStartTime = (MainToolbarItemFactory::ChainAmountBox *) mainToolbar.getItemComponent(i);
            pHumanizeStartTime->setWidth(40);
            pHumanizeStartTime->textBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey));
            pHumanizeStartTime->textBox.setText("40"); //This value should be initialized in MainWindow
            humanizeTimeAmount = "40";
        }
        else if (id == MainToolbarItemFactory::ToolbarItemIds::_humanizeVelocityBox)
        {
            
            pHumanizeVelocity =
            (MainToolbarItemFactory::ChainAmountBox *) mainToolbar.getItemComponent(i);
            pHumanizeVelocity->setWidth(65);
            pHumanizeVelocity->textBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey));
            pHumanizeVelocity->textBox.setText(".6,.8"); //This value should be initialized in MainWindow
            humanizeVelocityAmount = ".6,.8";
        }
        else
            mainToolbar.getItemComponent(i)->addListener(this);
    }
    for (int i=0; i<altToolbar.getNumItems(); i++)
    {
        int id = altToolbar.getItemId(i);
        if (id == AltToolbarItemFactory::ToolbarItemIds::adjustedTempo)
        {
            pAdjustedTempo = (AltToolbarItemFactory::AdjustedTempo *)altToolbar.getItemComponent(i);
            pAdjustedTempo->numberBox.setFont (Font (18.00f, Font::plain));
            pAdjustedTempo->numberBox.setColour(TextEditor::ColourIds::textColourId, Colour(Colours::darkgrey).darker());
        }
//        else if (id == AltToolbarItemFactory::ToolbarItemIds::scoreTempo)
//        {
//            pScoreTempo = (AltToolbarItemFactory::ScoreTempo *) altToolbar.getItemComponent(i);
//        }
        else if (id == AltToolbarItemFactory::ToolbarItemIds::scaledTempo)
        {
            pScaledTempo = (AltToolbarItemFactory::ScaledTempo *) altToolbar.getItemComponent(i);
        }
        altToolbar.getItemComponent(i)->addListener(this);
    }
    addAndMakeVisible (hoverStepInfo);
    hoverStepInfo.setFont (Font (15.00f, Font::bold));
    hoverStepInfo.setJustificationType (Justification::left);
    hoverStepInfo.setColour (Label::textColourId, Colours::lightgrey);
    
//    scoreTempoLabel.setText("Suggested Tempo",NotificationType::dontSendNotification);
    scoreTempoLabel.setFont (Font (19.00f, Font::plain ));
    scoreTempoLabel.setJustificationType (Justification::left);
    scoreTempoLabel.setColour (Label::textColourId, Colour(206,206,206));
    addAndMakeVisible (scoreTempoLabel);
    
//    adjustedTempoLabel.setText("BPM",NotificationType::dontSendNotification);
//    adjustedTempoLabel.setFont (Font (18.00f, Font::plain));
//    adjustedTempoLabel.setJustificationType (Justification::right);
//    adjustedTempoLabel.setColour (Label::textColourId, Colours::darkgrey);
//    addAndMakeVisible (adjustedTempoLabel);
    
    playableKeys = "qwertyuiopasdfghjklzxcvbnm;',./[]";

    for (int i=0;i<playableKeys
         .length();i++)
keysThatAreDown.add(false);
    
//    std::cout << "ViewerComponent isBroughtToFrontOnMouseClick " << isBroughtToFrontOnMouseClick()  <<"\n";
    startTimer(250);
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
    Sequence::Bookmark b = processor->atBookmark();
    if (processor->isPlaying)
    {
        if (mainToolbar.isVisible())
            editToolbarTempHidden = true;
        mainToolbar.setVisible(false);
    } else if (editToolbarTempHidden)
    {
        mainToolbar.setVisible(true);
        editToolbarTempHidden = false;
    }
    if (b.time!=-1)// && b.time!=0)
    {
        if (b.tempoChange)
            replaceAltToolbarItem(AltToolbarItemFactory::addTempoChange, AltToolbarItemFactory::removeTempoChange);
        else
            replaceAltToolbarItem(AltToolbarItemFactory::addBookmark, AltToolbarItemFactory::removeBookmark);
    }
    else
    {
        replaceAltToolbarItem(AltToolbarItemFactory::removeTempoChange, AltToolbarItemFactory::addTempoChange);
        replaceAltToolbarItem(AltToolbarItemFactory::removeBookmark, AltToolbarItemFactory::addBookmark);
    }
    if (processor->atPedalChange(MIDIProcessor::sustPedal))
    {
        replaceMainToolbarItem(MainToolbarItemFactory::_addSustain, MainToolbarItemFactory::_deleteSustain);
    }
    else
    {
        replaceMainToolbarItem(MainToolbarItemFactory::_deleteSustain, MainToolbarItemFactory::_addSustain);
    }
    if (processor->atPedalChange(MIDIProcessor::softPedal))
    {
        replaceMainToolbarItem(MainToolbarItemFactory::_addSoft, MainToolbarItemFactory::_deleteSoft);
    }
    else
    {
        replaceMainToolbarItem(MainToolbarItemFactory::_deleteSoft, MainToolbarItemFactory::_addSoft);
    }
    static String prevName = "";
    String plugin;
    if (processor->sequenceObject.pThePlugin)
    {
        plugin = " playing "+processor->sequenceObject.pThePlugin->getName();
        int progNum = processor->sequenceObject.pThePlugin->getCurrentProgram();
        if (progNum>=0)
            plugin = plugin + " (" +processor->sequenceObject.pThePlugin->getProgramName(progNum)+")";
    }
    String txt = processor->sequenceObject.getScoreFileName();
    const String newName = "Concert Keyboardist - " + processor->sequenceObject.getScoreFileName() + plugin;
    if (prevName != newName)
    {
		if (!(getTopLevelComponent()->getName() == newName))
		{
			getTopLevelComponent()->setName(newName);
			repaint();
		}
    }
    processor->sequenceObject.propertiesChanged = false;
    if (pChainAmountComboBox->newValue())
    {
        chainAmount = pChainAmountComboBox->getValue()*60.0;
        sendActionMessage("chain:"+String(chainAmount));
        grabKeyboardFocus();
        pChainAmountComboBox->clearChangeFlag();
    }
    if (pHumanizeVelocity->returnPressed)
    {
        std::cout << "Return pressed - HumanizeVelocity " <<pHumanizeVelocity->textBox.getText().getDoubleValue()<<"\n";
        String temp = pHumanizeVelocity->textBox.getText();
        humanizeVelocityAmount.clear();
        for (int i=0;i<temp.length();i++)
            if (temp.substring(i,i+1).containsAnyOf  (".0123456789,"))
                humanizeVelocityAmount.append(temp.substring(i,i+1), 1);
    
        sendActionMessage("humanizeVelocity:"+String(humanizeVelocityAmount));
        grabKeyboardFocus();
        pHumanizeVelocity->returnPressed = false;
    }
    if (pHumanizeStartTime->returnPressed)
    {
        std::cout << "Return pressed - HumanizeStartTime " <<pHumanizeStartTime->textBox.getText().getDoubleValue()<<"\n";
//        String temp = pHumanizeStartTime->textBox.getText();
//        humanizeTimeAmount.clear();
        sendActionMessage("humanizeTime:"+pHumanizeStartTime->textBox.getText());
//        for (int i=0;i<temp.length();i++)
//            if (temp.substring(i,i+1).containsAnyOf  (".0123456789,"))
//                humanizeVelocityAmount.append(temp.substring(i,i+1), 1);
//
//        sendActionMessage("humanizeVelocity:"+String(humanizeVelocityAmount));
        grabKeyboardFocus();
        pHumanizeStartTime->returnPressed = false;
    }
    if (pAdjustedTempo->changed)
    {
        if (processor->sequenceObject.tempoChanges.size()>0)
        {
                std::cout << "pTempoMultiplier->changed " << pAdjustedTempo->numberBox.getValue() << "\n";
                const int ztlTime = processor->getZTLTime(noteViewer.horizontalShift);
                const double tempoMult =    pAdjustedTempo->numberBox.getValue()/
                                            processor->sequenceObject.getTempo(ztlTime,processor->sequenceObject.tempoChanges);
                processor->setTempoMultiplier(tempoMult, ztlTime, true);
                pAdjustedTempo->changed = false;
                grabKeyboardFocus();
        }
    }
    else
    {
    }
}

void ViewerFrame::changeListenerCallback (ChangeBroadcaster* cb)
{
    if (cb == processor)
    {
        String txt = processor->sequenceObject.getScoreFileName();
        const double scaledTempo = processor->sequenceObject.getTempo(processor->getZTLTime(noteViewer.horizontalShift),
                                                                processor->sequenceObject.scaledTempoChanges);
        const double scoreTempo = processor->sequenceObject.getTempo(processor->getZTLTime(noteViewer.horizontalShift),
                                                               processor->sequenceObject.tempoChanges);
        scoreTempoLabel.setText(String(std::round(100.0*scaledTempo/scoreTempo))+"%",NotificationType::dontSendNotification);
//        std::cout << "scaledTempo, scoreTempo " << scaledTempo<<" "<<scoreTempo << "\n";
        pAdjustedTempo->setValue(scaledTempo);
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
//        if (MainToolbarItemFactory::ToolbarItemIds::doc_open == id)
//            sendActionMessage("fileOpen");
//        else if(MainToolbarItemFactory::ToolbarItemIds::doc_save == id)
//            sendActionMessage("fileSave");
//        else if(MainToolbarItemFactory::ToolbarItemIds::doc_saveAs == id)
//            sendActionMessage("fileSaveAs");
        if(MainToolbarItemFactory::ToolbarItemIds::edit_undo == id)
            sendActionMessage("editUndo");
        else if(MainToolbarItemFactory::ToolbarItemIds::edit_redo == id)
            sendActionMessage("editRedo");
        else if(MainToolbarItemFactory::ToolbarItemIds::_toggleActivity == id)
            sendActionMessage("toggleActivity");
        else if(MainToolbarItemFactory::ToolbarItemIds::_chain == id)
        {
            chainAmount = pChainAmountComboBox->getValue()*60.0;
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
//        else if(MainToolbarItemFactory::ToolbarItemIds::loadPlugin == id)
//        {
//            sendActionMessage("loadPluginMenu");
//        }
//        else if(MainToolbarItemFactory::ToolbarItemIds::editPlugin == id)
//            sendActionMessage("editPlugin");
    }
    else //in alt toolbar
    {
        int id = altToolbar.getItemId(i);
        if (AltToolbarItemFactory::ToolbarItemIds::doc_open == id)
            sendActionMessage("fileOpen");
        else if(AltToolbarItemFactory::ToolbarItemIds::doc_save == id)
            sendActionMessage("fileSave");
        else if(AltToolbarItemFactory::ToolbarItemIds::doc_saveAs == id)
            sendActionMessage("fileSaveAs");
        else if(AltToolbarItemFactory::ToolbarItemIds::loadPlugin == id)
            sendActionMessage("loadPluginMenu");
        
        else if(AltToolbarItemFactory::ToolbarItemIds::audioSettings == id)
            sendActionMessage("audioSettings");
        else if(AltToolbarItemFactory::ToolbarItemIds::scoreInfo == id)
            sendActionMessage("scoreInfo");
        
        else if(AltToolbarItemFactory::ToolbarItemIds::editPlugin == id)
            sendActionMessage("editPlugin");

        else if(AltToolbarItemFactory::ToolbarItemIds::_play == id)
            sendActionMessage("play");
        else if(AltToolbarItemFactory::ToolbarItemIds::_stop == id)
            sendActionMessage("pause");
        else if(AltToolbarItemFactory::ToolbarItemIds::_rewind == id)
            sendActionMessage("rewind");
        else if(AltToolbarItemFactory::ToolbarItemIds::_listen == id)
            sendActionMessage("listenToSelection");
        else if(AltToolbarItemFactory::ToolbarItemIds::addTempoChange == id)
        {
            std::cout << "Save tempo change\n";//sendActionMessage("listenToSelection");
            if (processor->sequenceObject.theSequence.size()>0)
            {
                processor->catchUp(false);
//                const double scoreTempo = processor->sequenceObject.getTempo(processor->getZTLTime(noteViewer.horizontalShift),
//                                                                             processor->sequenceObject.tempoChanges);
                processor->addRemoveBookmark(BOOKMARK_ADD,true,1.0);//pAdjustedTempo->numberBox.getText().getDoubleValue()/scoreTempo);
            }
        }
        else if(AltToolbarItemFactory::ToolbarItemIds::removeTempoChange == id)
        {
            std::cout << "Remove tempo change\n";
            if (processor->sequenceObject.theSequence.size()>0)
                processor->addRemoveBookmark(BOOKMARK_REMOVE);
        }
        else if(AltToolbarItemFactory::ToolbarItemIds::addBookmark == id)
        {
            std::cout << "Add bookmark\n";
            if (processor->sequenceObject.theSequence.size()>0)
            {
                processor->catchUp(false);
                processor->addRemoveBookmark(BOOKMARK_ADD);
            }
        }
        else if(AltToolbarItemFactory::ToolbarItemIds::removeBookmark == id)
        {
            std::cout << "Remove bookmark\n";//sendActionMessage("listenToSelection");
            if (processor->sequenceObject.theSequence.size()>0)
                processor->addRemoveBookmark(BOOKMARK_REMOVE);
        }
        else if(AltToolbarItemFactory::ToolbarItemIds::showEditToolbar == id)
        {
            const String visible = getAppProperties().getUserSettings()->getValue ("editToolbarVisible", "false");
            if (visible == "false") {
                getAppProperties().getUserSettings()->setValue ("editToolbarVisible","true");
                mainToolbar.setVisible(true);
            }
            else
            {
                getAppProperties().getUserSettings()->setValue ("editToolbarVisible","false");
                mainToolbar.setVisible(false);
            }
            std::cout << "showEditToolbar\n";//sendActionMessage("listenToSelection");
        }
        else if(AltToolbarItemFactory::ToolbarItemIds::_help == id)
        {
            sendActionMessage("help");
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
//        processor->sequenceObject.setScoreFile(file);
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
    g.setColour(Colour(48,48,48));
    //Block above keys
    g.fillRect(noteViewer.getKeysWidth()-2, noteViewer.getHeight(),      2, 2*(separatorLineWidth+noteViewer.getToolbarHeight()));
    //Block below keys
    g.setColour(Colour(64,64,64));
    g.fillRect(0, noteViewer.getHeight(),         noteViewer.getKeysWidth()-2, 2*(separatorLineWidth+noteViewer.getToolbarHeight()));
    //Vertical separator
    g.setColour(Colour(127,127,127));
    g.fillRect(noteViewer.getKeysWidth()-2, noteViewer.getHeight(),        2, 2*(separatorLineWidth+noteViewer.getToolbarHeight()));
    //Block for hover info
    g.setColour(Colour(64,64,64));
    g.fillRect(noteViewer.getKeysWidth(),  noteViewer.getHeight(),        noteViewer.getWidth(), 2*(separatorLineWidth+noteViewer.getToolbarHeight()));
    g.setColour(Colour(127,127,127));
    g.fillRect(248+noteViewer.getKeysWidth(),  noteViewer.getHeight(),      2, 2*(separatorLineWidth+noteViewer.getToolbarHeight()));
    //Keyboard
    g.drawImageAt(noteViewer.getKeysImage(), 0, 0); //Keyboard
    //Lines between toolbars
    g.setColour(Colour(127,127,127));
//    g.setColour(Colours::purple);
    g.fillRect(noteViewer.getKeysWidth(), noteViewer.getHeight()+separatorLineWidth,     getWidth(), separatorLineWidth*2);
//    g.setColour(Colours::green);
    g.fillRect(noteViewer.getKeysWidth()+250, noteViewer.getHeight()+noteViewer.getToolbarHeight()+separatorLineWidth*3.5,
               getWidth(), separatorLineWidth);
//    g.setColour(Colours::red);
    g.fillRect(0, 0,       getWidth(), 1.0);
}

void ViewerFrame::resized()
{
    float tbH = noteViewer.getToolbarHeight();

    noteViewer.setBounds(noteViewer.getKeysWidth(), 1.0, getWidth()-noteViewer.getKeysWidth(), getHeight()-tbH*2-separatorLineWidth*3);
    noteViewerBottom = noteViewer.getBottom();

    //Toolbars
    mainToolbar.setBounds(noteViewer.getKeysWidth()+250, noteViewerBottom+separatorLineWidth+1.0,     getWidth(), tbH);
    altToolbar.setBounds (noteViewer.getKeysWidth()+250, noteViewerBottom+tbH+separatorLineWidth*3.0, getWidth(), tbH);

//    altToolbarVisible = true;
//    if (altToolbarVisible)
//    {
//        Rectangle<int> shifted = getLocalBounds();//.removeFromBottom(noteViewer.getToolbarHeight());
//        shifted.setHeight(noteViewer.getToolbarHeight());
//        shifted.translate(0,  getLocalBounds().getHeight()-noteViewer.getToolbarHeight());
//        mainToolbar.setBounds (shifted);
//    }

    const auto adjTempoLeft = pAdjustedTempo->getBounds().getRight() + 250 - 4;
    scoreTempoLabel.setBounds(
                              adjTempoLeft + noteViewer.getKeysWidth(),
                              noteViewer.getHeight()+noteViewer.getToolbarHeight()+separatorLineWidth+2,
                              46, noteViewer.getToolbarHeight()-1);
    hoverStepInfo.setBounds(
                            noteViewer.getKeysWidth(),
                            noteViewer.getHeight(),
                            250, (tbH+separatorLineWidth)*2);
}
