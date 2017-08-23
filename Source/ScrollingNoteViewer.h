/*
 ==============================================================================
 
 This is an automatically generated GUI class created by the Projucer!
 
 Be careful when adding custom code to these files, as only the code within
 the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
 and re-saved.
 
 Created with Projucer version: 4.2.3
 
 ------------------------------------------------------------------------------
 
 The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
 Copyright (c) 2015 - ROLI Ltd.
 
 ==============================================================================
 */

#ifndef __JUCE_HEADER_B11C4CC4490D982E__
#define __JUCE_HEADER_B11C4CC4490D982E__

//[Headers]     -- You can add your own extra header files here --
#include "../JuceLibraryCode/JuceHeader.h"
//#include "PluginProcessor.h"
#include <array>
#include <set>
#include <algorithm>
#include <queue>
#include "NoteWithOffTime.h"
#include "MIDIProcessor.h"
//[/Headers]

struct Vertex  // class storing the information about a single vertex
{
    float position[2];
    float colour[3];
};

//==============================================================================
/**
 //[Comments]
 An auto-generated component, created by the Projucer.
 
 Describe your class and how it works here!
 //[/Comments]
 */
class ScrollingNoteViewer  :
    public Component,
    private MultiTimer,
    private OpenGLContext,
    private OpenGLRenderer,
    public ChangeBroadcaster,
    public ChangeListener
{
public:
    //==============================================================================
    ScrollingNoteViewer (MIDIProcessor *p);
    ~ScrollingNoteViewer();
    
    void changeListenerCallback (ChangeBroadcaster* broadcaster) override;    
    virtual void mouseDown (const MouseEvent& event) override;
    virtual void mouseUp (const MouseEvent& event) override;
    virtual void mouseDoubleClick (const MouseEvent& event) override;
    virtual void mouseMove (const MouseEvent& event) override;
    virtual void mouseDrag (const MouseEvent& event) override;
    virtual void mouseWheelMove (const MouseEvent& event,
                                 const MouseWheelDetails& wheel) override;
    virtual void mouseMagnify (const MouseEvent& event, float scaleFactor) override;
    
    OpenGLContext openGLContext;
    virtual void newOpenGLContextCreated() override;
    virtual void renderOpenGL() override;
    virtual void openGLContextClosing() override;
    CriticalSection glRenderLock;
    std::atomic_bool rebuidingGLBuffer;
    CriticalSection mkNoteBars;
    
    Matrix3D<float> getProjectionMatrix(float horizScale, float vertScale) const;
    Matrix3D<float> getViewMatrix(float x) const;
    void setRectangleColour (int rect, Colour col);
    int addRectangle(float x, float y, float w, float h, Colour col);
    int addQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Colour col);
    int addEqTriangle (float x, float yy, float w, float hh, bool invert, Colour col);
    int addLine(float x1, float y1, float x2, float y2, float w, Colour col);
//    int addRectangle(float x, float y, float w, float h, float r, float g, float b);
//    int addNote(bool playable, float x, float yy, float w, float hh, float r, float g, float b);
    
    struct NoteBarDescription
    {
        int seqIndex;
        float x;
        float y;
        float w;
        float h;
        float headWidth;
        float headHeight;
        Colour colHead;
        Colour colBar;
    };
    int addNote(/*bool playable, */float x, float y,  float w, float h, float headWidth, float headHeight,
                Colour colHead, Colour colBar);
    void setRectanglePos(int rect, float x, float y, float w, float h);
    void createShaders();
    
    void makeKeyboard();
    Image getKeysImage() { return keysImage; }
    int getKeysWidth() {return leftMargin+wKbd;}
    int getToolbarHeight() { return toolbarHeight;}
    int getTopMargin() { return verticalScale*topMargin;}
    String getHoverInfo() {return hoverInfo;}
    void makeNoteBars();
    void updatePlayedNotes(); //Just updates the note bar heads in the exising vertex and index buffers
    int glBufferUpdateCountdown;
    bool sequenceChanged; //Set true to regen note bar GL index and vertex buffers
    File prevFileLoaded;
    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
#define TIMER_TWEEN 0
#define TIMER_TOGGLE_TARGET_NOTE 1
#define TIMER_MOUSE_HOLD 2
#define TIMER_MOUSE_DRAG 3
#define TIMER_REPAINT_SELECTION 4
#define TIMER_MOUSE_UP 5
    Point<int> curDragPosition;
    
    void timerCallback (int timerID) override;
    
    String getNoteText (const int midiNoteNumber);
    
    /** A set of colour IDs to use to change the colour of various aspects of the keyboard.
     
     These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
     methods.
     
     @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
     */
    enum ColourIds
    {
        whiteNoteColourId               = 0x1005000,
        blackNoteColourId               = 0x1005001,
        keySeparatorLineColourId        = 0x1005002,
        mouseOverKeyOverlayColourId     = 0x1005003,
        keyDownOverlayColourId          = 0x1005004,
        textLabelColourId               = 0x1005005,
        upDownButtonBackgroundColourId  = 0x1005006,
        upDownButtonArrowColourId       = 0x1005007,
        shadowColourId                  = 0x1005008
    };
    
    //==============================================================================
    // This class just manages the uniform values that the demo shaders use.
    struct Uniforms
    {
        Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
        {
            projectionMatrix = createUniform (openGLContext, shaderProgram, "projectionMatrix", projectionMatrixHandle);
            viewMatrix       = createUniform (openGLContext, shaderProgram, "viewMatrix", viewMatrixHandle);
        }
        
        ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix;
        ScopedPointer<OpenGLShaderProgram::Uniform> viewMatrix;
        static GLint viewMatrixHandle;
        static GLint projectionMatrixHandle;
        
    private:
        static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
                                                            OpenGLShaderProgram& shaderProgram,
                                                            const char* uniformName,
                                                            GLint &handle)
        {
            handle = openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName);
//            std::cout << "vmh when defined "<< viewMatrixHandle << "\n";
            if (handle < 0)
                return nullptr;
            
            return new OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
        }
        
        static GLint getViewMatrixHandle()
        {
            return viewMatrixHandle;
        }
        static GLint getProjectionMatrixHandle()
        {
            return projectionMatrixHandle;
        }
    };
    
    double horizontalShift;  //Shift in pixels due to drag left or right on the resize bar
    double horizontalShiftAtStop;
    inline void setHorizontalShift(double shift)
    {
        horizontalShift = shift;
//        const double xInTicks = processor->getTimeInTicks()-horizontalShift*pixelsPerTick;
        if (horizontalShift==0)
            processor->setXInTicks(0);
        else
            processor->setXInTicks((horizontalShift/pixelsPerTick)/horizontalScale);
    }
// std::cout << "HorizontalShift, pixelsPerTick "<<horizontalShift <<" "<<xInTicks <<"\n";
    void resetHorizontalShift();
    
    int nSteps;
    double transitionTime;
    int animationStep;
    Array<float> horizontalShiftTweens;
    Array<double> timeInTicksTweens;
    void setSelectedNotes(Array<int> sel)
    {
        selectedNotes = sel;
        processor->setCopyOfSelectedNotes(selectedNotes);
    }
    Array<int> getSelectedNotes()
    {
        return selectedNotes;
    }
    void clearSelectedNotes()
    {
        selectedNotes.clear();
        displayedSelection.clear();
        newlySelectedNotes.clear();
        processor->setCopyOfSelectedNotes(selectedNotes);
        for (int i=0;i<processor->sequenceObject.chords.size();i++)
            processor->sequenceObject.chords[i].selected = false;
        repaint();
    }
    void selectAll()
    {
        selectedNotes.clear();
        displayedSelection.clear();
        newlySelectedNotes.clear();
        for (int step=0;step<processor->sequenceObject.theSequence.size();step++)
        {
//            if (processor->sequenceObject.isActiveTrack(processor->sequenceObject.theSequence[step]->track))
            selectedNotes.add(step);
            displayedSelection.add(step);
        }
        processor->setCopyOfSelectedNotes(selectedNotes);
        repaint();
    }
    
    bool showingChords;
    
private:
    MIDIProcessor *processor;
    
    
    
//    int stepToReset; //Keeps track of next step to reset during render.
//    int lastStepToReset;
    int leadTimeInTicks;
    double leadTimeProportionOfWidth;
    double sequenceStartPixel;
    int seqEndInPixels;
    float wKbd;
    int maxNote;
    int minNote;
    bool compressNotes;
    int octaveNumForMiddleC;
    float toolbarHeight;
    float topMargin;
    float leftMargin;
    int initialMeasuresAcrossWindow;
    int initialWidth;
    int initialHeight;
    bool zoomDragStarting;
    bool zoomOrScrollDragging;
    bool selecting;
    bool draggingVelocity; //Dragging on one note head
    bool drawingVelocity; //Dragging on background to set multiple notes velocities
    bool draggingTime;
    bool draggingOffTime;
//    bool showVelocityIndicator; //If true, the velocity tick mark indicator of the hoverStep is to be painted
    float velStartDrag;
    float timeStartDrag;
    float offTimeStartDrag;
    double timeAfterDrag;
    float velocityAfterDrag;
    double offTimeAfterDrag;
    Point<int> selectionAnchor;
    Rectangle<int> selectionRect;
    Array<int> selectedNotes;
    Array<int> newlySelectedNotes;
    Array<int> displayedSelection;
    
    bool editingNote; //Dragging on note head
    Point<int> noteEditAnchor;
    
    float preDragScale;
    float preDragHorizShift;
    float preDragXinTicks;
    float mouseXinTicks;
    Point<float> mouseBeforeDrag;
    int hoverStep; //Step over which mouse pointer hovers.  -1 if none.
    int hoverChord; //Chord over which mouse pointer hovers.  -1 if none.
    String hoverInfo;
#define HOVER_NONE 0
#define HOVER_NOTEBAR 1
#define HOVER_NOTETRACK 2
#define HOVER_ZEROTIMEHANDLE 3
#define HOVER_ZEROTIMELINE 4
#define HOVER_TOPBAR 5
#define HOVER_CHORD 6
    int hoveringOver;
    Point<float> hoverPosition;
    
    int numerator, denominator; //Of time signature
    double ticksPerQuarter;
    
    bool refresh = true;
    int nKeys;
    SortedSet<int> notesUsed;
    float trackVerticalSize;
    float noteBarWidthRatio; //As fraction of track width
    std::array<float, 128> noteYs;
    Image keysImage;
    Array<MidiMessage> timeSigChanges;
    double pixelsPerTick;
    bool grabbedInitialWindowSize;

    //For OpenGL
//    int renderingStartCounter = 3;
    int frameCounter = 0;
    GLuint vertexBuffer, indexBuffer;
    int numIndices = 0;
    float delta = 0.002;
    float x;
    float y;
    float pos = 0;
    float desktopScale;
    float horizontalScale;
    float verticalScale;
    float prevMouseDragX;
    int positionLine;

    int firstNewNote, lastNewNote;
    
    int firstNoteBarRect;
    int nextNoteRect;  //Index of green rectangle that shows time of next note due
    
    Array<Vertex> vertices;
    Array<int> indices;
    const char* vertexShader;
    const char* fragmentShader;
    
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<OpenGLShaderProgram::Attribute> position, sourceColour;
    ScopedPointer<Uniforms> uniforms;
    
    String newVertexShader, newFragmentShader;
    
    Colour colourActiveNoteHead;
    Colour colourPrimaryNoteBar;
    Colour colourNoteOn;
    Colour colourInactiveNoteHead;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollingNoteViewer)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_B11C4CC4490D982E__
