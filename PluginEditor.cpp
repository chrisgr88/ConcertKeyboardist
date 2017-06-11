/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
ReExpressorAudioProcessorEditor::ReExpressorAudioProcessorEditor (ReExpressorAudioProcessor *p) :
    AudioProcessorEditor (p),
    processor (p),
    viewer(p)
{
    viewer.addChangeListener(this);
    addAndMakeVisible(viewer);
    viewer.setOpaque(true);
    
    addAndMakeVisible (resizer = new ResizableCornerComponent (this, &resizeLimits));
    resizeLimits.setSizeLimits (150, 300, 1600, 1000);
    setSize (1000,300);
    
    addAndMakeVisible (tempoSlider);
    tempoSlider.setRange (20, 400, 1);
    tempoSlider.setSliderStyle (Slider::LinearBar);
//    tempoSlider.setTextBoxStyle (Slider::NoTextBox, true, 80, 14);
    tempoSlider.setTextBoxIsEditable(false);
    tempoSlider.addListener (this);
    tempoSlider.setValue(processor->getTempo(), NotificationType::dontSendNotification);
    
    fileOpenButton.setButtonText ("Commands");
    fileOpenButton.setTriggeredOnMouseDown (true);
    fileOpenButton.addListener (this);
    addAndMakeVisible (fileOpenButton);
    
    rewindButton.setButtonText ("Rewind");
    rewindButton.setTriggeredOnMouseDown (false);
    rewindButton.addListener (this);
    addAndMakeVisible (rewindButton);
}

ReExpressorAudioProcessorEditor::~ReExpressorAudioProcessorEditor()
{
    fileOpenButton.removeListener (this);
    rewindButton.removeListener (this);
    tempoSlider.removeListener(this);
}

void ReExpressorAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster*)
{
    std::cout << "Change Reported in PluginEditor\n";
    repaint();
}

void ReExpressorAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == &fileOpenButton)
    {
        //            processor->loadSequence("/Users/chrisgr/Downloads/grieg_walzer.mid");
        File fileFolder = File::getSpecialLocation(File::userHomeDirectory);
        int flags = FileBrowserComponent::openMode |
        FileBrowserComponent::canSelectFiles | FileBrowserComponent::filenameBoxIsReadOnly;
        fileFilter = new WildcardFileFilter("*.mid", "*", "Midi files");
        // create the browser component
        fileBrowser = new FileBrowserComponent(flags,fileFolder, fileFilter, NULL);
        
        // add browser compoenent to form and add us as a listener
        addAndMakeVisible(fileBrowser);
        fileBrowser->setTopLeftPosition(0, 15);
        fileBrowser->setSize(300, getHeight()-15);
        fileBrowser->addListener(this);
        
        Rectangle<int> r = viewer.getBounds();
        r.setLeft(300);
        r.setWidth(getWidth()-300);
        viewer.setBounds(r);
    }
    else if (button == &rewindButton)
    {
        processor->rewind();
    }
}

void ReExpressorAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == &tempoSlider)
        processor->setTempo(tempoSlider.getValue());
}

/** Callback when the user selects a different file in the browser. */
void ReExpressorAudioProcessorEditor::selectionChanged() {};

/** Callback when the user clicks on a file in the browser. */
void ReExpressorAudioProcessorEditor::fileClicked (const File& file, const MouseEvent& e)
{
};

/** Callback when the user double-clicks on a file in the browser. */
void ReExpressorAudioProcessorEditor::fileDoubleClicked (const File& file) {
    if (!file.isDirectory())
    {
        String fn = file.getFullPathName();
        fn = file.getFullPathName();
        processor->loadSequence(fn);
        processor->setSequenceChanged(true);
        fileBrowser->setVisible(false);
        Rectangle<int> r = viewer.getBounds();
        r.setLeft(viewer.getKeysWidth());
        r.setWidth(getWidth()-viewer.getKeysWidth());
        viewer.setBounds(r);
        repaint();
        tempoSlider.setValue(processor->getTempo());
    }
};

/** Callback when the browser's root folder changes. */
void ReExpressorAudioProcessorEditor::browserRootChanged (const File& newRoot) {};

//==============================================================================
void ReExpressorAudioProcessorEditor::paint (Graphics& g)
{
    g.setColour(Colours::grey);
    g.fillRect(0,0,getWidth(),viewer.getCommandBar()); //Command bar
    g.drawImageAt(viewer.getKeysImage(), 0, viewer.getCommandBar()); //Keyboard
}

void ReExpressorAudioProcessorEditor::resized()
{
    viewer.setBounds(viewer.getKeysWidth(), viewer.getCommandBar(),
                     getWidth()-viewer.getKeysWidth(), getHeight()-viewer.getCommandBar());
    
    resizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);
    fileOpenButton.setBounds (0,0,100,viewer.getCommandBar()-1);
    rewindButton.setBounds(102, 0, 80, viewer.getCommandBar()-1);
    tempoSlider.setBounds(200, 0, 200, viewer.getCommandBar()-1);
}
