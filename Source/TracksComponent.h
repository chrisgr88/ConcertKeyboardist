/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 4.3.1

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "Sequence.h"
#include "MIDIProcessor.h"
#include "MainWindow.h"

static int nActive;
class TracksComponent    : public Component, public TableListBoxModel
{
public:
    TracksComponent( MIDIProcessor *proc)   : font (12.0f)
    {
        nActive = 0;
        processor = proc;
        sequence = &proc->sequenceObject;
        addAndMakeVisible (table);
        table.setModel (this);
        table.setColour (ListBox::outlineColourId, Colours::grey);
        table.setOutlineThickness (1);
        table.getHeader().addColumn ("Track", 1,   32,32,32,   TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn ("Text", 2,   350,100,450,   TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn ("Instrument", 3, 120,70,200,   TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn ("Notes", 4,  35,35,50,   TableHeaderComponent::defaultFlags);
        
        table.getHeader().addColumn ("First Measure", 5,  68,35,68,   TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn ("Last Measure", 6,  65,35,65,   TableHeaderComponent::defaultFlags);
        
        table.getHeader().addColumn ("Sustains", 7,  57,50,70,   TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn ("Softs", 8,   35,35,40,   TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn ("Channel", 9,   57,50,70,   TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn ("Active", 10,   60,60,65,   TableHeaderComponent::defaultFlags);
        table.setMultipleSelectionEnabled (false);
    }
    ~TracksComponent()
    {
        
    }
    
    int getNumRows() override
    {
        return sequence->trackDetails.size();
    }
    
    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        g.fillAll (Colour (0xffeeeeee));
    }
    
    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom components.
    void paintCell (Graphics& g, int rowNum, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override
    {
//        std::cout << rowNum<< " paintCell row: rowNum, columnId " << rowNum<<" "<< columnId <<"\n";

        g.setColour (Colours::black);
        g.setFont (font);
        
        if (columnId==1)
            g.drawText (String(rowNum), 2, 0, width - 4, height, Justification::centred, true);
        else if (columnId==2)
            g.drawText (sequence->trackDetails[rowNum].description, 2, 0, width - 4, height, Justification::centredLeft, true);
        else if (columnId==3)
            g.drawText (sequence->trackDetails[rowNum].instrument, 2, 0, width - 4, height, Justification::centredLeft, true);
        else if (columnId==4)
            g.drawText (String(sequence->trackDetails[rowNum].nNotes), 2, 0, width - 4, height, Justification::centred, true);
        
        else if (columnId==5)
        {
            const double s = sequence->trackDetails[rowNum].startMeasure;
            g.drawText (s<0?"":String(s), 2, 0, width - 4, height, Justification::centred, true);
        }
        else if (columnId==6)
        {
            const double e = sequence->trackDetails[rowNum].endMeasure;
            g.drawText (e<0?"":String(e), 2, 0, width - 4, height, Justification::centred, true);
        }
        
        else if (columnId==7)
            g.drawText (String(sequence->trackDetails[rowNum].nSustains), 2, 0, width - 4, height, Justification::centred, true);
        else if (columnId==8)
            g.drawText (String(sequence->trackDetails[rowNum].nSofts), 2, 0, width - 4, height, Justification::centred, true);
        else if (columnId==9)
        {
            String chan = sequence->trackDetails[rowNum].originalChannel>=0?String(sequence->trackDetails[rowNum].originalChannel):"";
            g.drawText (chan, 2, 0, width - 4, height, Justification::centred, true);
        }
        
        g.setColour (Colours::black.withAlpha (0.2f));
        g.fillRect (width - 1, 0, 1, height);
    }
    
    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (int rowNum, int columnId, bool /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override
    {
//        std::cout << rowNum<< "refreshComponentForCell Track row: nEvents " << sequence->trackDetails[rowNum].nNotes <<"\n";

        if (rowNum==0)
            nActive = 0;
        if (columnId != 10)
        {
            jassert (existingComponentToUpdate == nullptr);
            return nullptr;
        }
        
        PlayabilityColumnCustomComponent* playabilityBox = static_cast<PlayabilityColumnCustomComponent*> (existingComponentToUpdate);
        if (playabilityBox == nullptr)
        {
            playabilityBox = new PlayabilityColumnCustomComponent (*this,false);
        }
        else
        {
            delete playabilityBox;
            playabilityBox = new PlayabilityColumnCustomComponent (*this, false);
        }
    
        if (sequence->trackDetails[rowNum].nNotes>0)
        {
            playabilityBox->setRowAndColumn (rowNum, columnId, sequence->trackDetails[rowNum].playability);
            if (sequence->trackDetails[rowNum].playability==1)
                nActive++;
        }
        else
            playabilityBox->setEnabled(false);
        
        return playabilityBox;
    }
    
    int getColumnAutoSizeWidth (int columnId) override
    {
        if (columnId == 5)
            return 100;
        
        int widest = 32;
        
        // find the widest bit of text in this column..
        for (int i = getNumRows(); --i >= 0;)
        {
            Label *lab = (EditableTextCustomComponent*)table.getCellComponent(columnId, i);
            String text;
            if (lab)
                text = lab->getText();
            else
                text = "";
            widest = jmax (widest, font.getStringWidth (text));
        }
        
        return widest + 8;
    }
    
    void setPlayability (const int rowNum, const int newPlayability)
    {
        try {
            Sequence::TrackDetail trkDetail = sequence->trackDetails[rowNum];
            if (trkDetail.playability != newPlayability)
            {
                trkDetail.playability = newPlayability;
                sequence->trackDetails.set(rowNum, trkDetail);
                processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getTimeInTicks());
            }
            nActive = 0;
            for (int t=1;t<3;t++)
            {
                if ( sequence->trackDetails[t].playability==1)
                    nActive++;
            }
         } catch (const std::out_of_range& ex) {
             std::cout << " error in setPlayability " << "\n";
         }
    }
    
    //==============================================================================
    void resized() override
    {
        table.setBoundsInset (BorderSize<int> (8));
    }
    
private:
    TableListBox table;
    Font font;
    int numRows;
    
    //==============================================================================
    // This is a custom Label component, which we use for the table's editable text columns.
    class EditableTextCustomComponent  : public Label
    {
    public:
        EditableTextCustomComponent (TracksComponent& td)  : owner (td)
        {
            // double click to edit the label text; single click handled below
            setEditable (false, true, false);
            setColour (textColourId, Colours::black);
        }
        
        void mouseDown (const MouseEvent& event) override
        {
            // single click on the label should simply select the row
//            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);
//            Label::mouseDown (event);
        }
        
        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
        }
        
    private:
        TracksComponent& owner;
        ComboBox comboBox;
        int row, columnId;
    };
    
    //==============================================================================
    class PlayabilityColumnCustomComponent    :
    public Component,
    public Button::Listener
    {
    public:
        PlayabilityColumnCustomComponent (TracksComponent& td, int playability)  : owner (td), activeButton("")
        {
            activeButton.setButtonText ("");
            activeButton.changeWidthToFitText();
            activeButton.addListener (this);
            activeButton.setWantsKeyboardFocus (false);
            addAndMakeVisible (activeButton);
        }
        
        void resized() override
        {
            BorderSize<int> bs =  BorderSize<int> (0,14,0,0);
            activeButton.setBoundsInset (bs);
        }
        
        void setRowAndColumn (int newRow, int newColumn, int setting)
        {
            row = newRow;
            columnId = newColumn;
            if (setting==1)
                activeButton.setToggleState(true, dontSendNotification);
            else
                activeButton.setToggleState(false, dontSendNotification);
        }

        void buttonClicked (Button*) override
        {
            if (nActive>0 || activeButton.getToggleState()==true)
                owner.setPlayability (row, activeButton.getToggleState());
            else
                activeButton.setToggleState(true, NotificationType::dontSendNotification);
        };
        
    private:
        TracksComponent& owner;
        ToggleButton activeButton;
        int row, columnId;
    };
    
    MIDIProcessor *processor;
    Sequence *sequence;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TracksComponent)
};


