/*
  ==============================================================================
    This file was auto-generated!
    It contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
//#include "Main Screen.h"
#include "ScrollingNoteViewer.h"

//==============================================================================
/**
*/
class ReExpressorAudioProcessorEditor  :
    public AudioProcessorEditor,
    public ChangeListener,
    public ChangeBroadcaster,
    private Button::Listener,
    private Slider::Listener,
    public FileBrowserListener
{
public:
    ReExpressorAudioProcessorEditor (ReExpressorAudioProcessor*);
    ~ReExpressorAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    double timeInTicks;
    double prevTimeInTicks = -1.0;
    
    void changeListenerCallback (ChangeBroadcaster*) override;
    
    void buttonClicked (Button* button) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    /** Callback when the user selects a different file in the browser. */
    virtual void selectionChanged() override;
    /** Callback when the user clicks on a file in the browser. */
    virtual void fileClicked (const File& file, const MouseEvent& e) override;
    /** Callback when the user double-clicks on a file in the browser. */
    virtual void fileDoubleClicked (const File& file) override;
    /** Callback when the browser's root folder changes. */
    virtual void browserRootChanged (const File& newRoot) override;

private:
    ReExpressorAudioProcessor *processor;
    ScrollingNoteViewer viewer;
    FileBrowserComponent *fileBrowser;
    WildcardFileFilter *fileFilter;
    const int commandBarHeight =30;

    TextButton fileOpenButton;
    TextButton rewindButton;
    Slider tempoSlider;
    ScopedPointer<ResizableCornerComponent> resizer;
    ComponentBoundsConstrainer resizeLimits;
    


//    static void menuItemChosenCallback (int result, MultithreadingDemo* demoComponent)
//    {
//        if (result != 0 && demoComponent != nullptr)
//            demoComponent->setUsingPool (result == 2);
//    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReExpressorAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
