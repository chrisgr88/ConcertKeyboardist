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

struct ViewStateInfo
{
    //public:
    static int initialWidth;
    //    static int a;
    static int initialHeight;
    static int initialPPT; //Initial pixels per tick
    static int viewWidth;
    static int viewHeight;
    static float verticalScale;
    static float horizontalScale;
    static float trackVerticalSize;
    static Array<Vertex> vertices;
    static Array<int> indices;
};
//These are in the cpp file
//int ViewStateInfo::initialWidth = 0;
//int initialHeight = 0;
//int initialPPT = 0; //Initial pixels per tick
//int viewWidth = 0;
//int viewHeight = 0;

class NoteTimeComparator
{
public:
    NoteTimeComparator(MIDIProcessor *proc) :
    p(proc)
    {}
    
    int compareElements (int first, int second)
    {
        return (p->sequenceObject.theSequence.at(first)->getTimeStamp() < p->sequenceObject.theSequence.at(second)->getTimeStamp() ?
            -1
            : (p->sequenceObject.theSequence.at(second)->getTimeStamp() < p->sequenceObject.theSequence.at(first)->getTimeStamp()) ?
                1 : 0);
    }
    MIDIProcessor *p;
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
    friend NoteTimeComparator;
public:
    //==============================================================================
    ScrollingNoteViewer (MIDIProcessor *p);
    ~ScrollingNoteViewer();
    
    bool tickIsVisible(double tick) //True if given time is visible in the viewer
    {
        double ltp = leadTimeProportionOfWidth * ViewStateInfo::viewWidth;
        double positionOfTime = ltp - (processor->getZTLTime(99)-tick)*pixelsPerTick * horizontalScale;
//        positionOfTime += 484;
//        std::cout << "positionOfTime "<<positionOfTime
//        <<" leadTimeInTicks "<<leadTimeInTicks
//        <<" viewWidth "<<ViewStateInfo::viewWidth
//        <<" onScreen "<<(0<positionOfTime && positionOfTime<ViewStateInfo::viewWidth)<<"\n";
        
        return (0<positionOfTime && positionOfTime<ViewStateInfo::viewWidth);
    }
    
    int seqSize = 0;
    MouseCursor editVelocityCursor;
    MouseCursor getMouseCursorFromZipFile(const String& filename) {
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
                    //                        std::cout << "file " << icons.getEntry(i)->filename<<"\n";
                    iconNames.add (icons.getEntry(i)->filename);
                    iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));
                }
            }
        }
        ScopedPointer<Drawable> image = iconsFromZipFile [iconNames.indexOf (filename)]->createCopy();
        juce::Image img =  juce::Image(juce::Image::ARGB, 32, 32, true);
        Graphics g(img);
        image->draw(g, 1.0f);
        return MouseCursor(img,0,0);
    }
    MouseCursor selectionMarkerCursor;
    MouseCursor selectionUnMarkerCursor;
    MouseCursor marqueeAddingCursor;
    MouseCursor marqueeRemovingCursor;
    MouseCursor getCircleCursor(Colour col, float diameter)
    {
        juce::Image img =  juce::Image(juce::Image::ARGB, 32, 32, true);
        col = col.withAlpha(0.9f);
        Graphics g(img);
        g.setColour (col);
        g.fillEllipse ((1.0f-diameter)/2.0f, (1-diameter)/2.0f, 32.f*diameter, 32.f*diameter);
        return MouseCursor(img,0,0);
    }
    
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
    std::atomic_bool rendering;
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
#define TIMER_PERIODIC 6
#define TIMER_IGNORE_WHEEL 7
    
    bool ignoreWheel;
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
    
    //These are used to protect the selection during operations that can change ot order of note steps 
    std::vector<std::shared_ptr<NoteWithOffTime>> stashSelectedNotes()
    {
        std::vector<std::shared_ptr<NoteWithOffTime>> selNotes;
        for (int i=0; i<selectedNotes.size();i++)
            selNotes.push_back(processor->sequenceObject.theSequence.at(selectedNotes[i]));
        return selNotes;
    }
    void restoreSelectedNotes(std::vector<std::shared_ptr<NoteWithOffTime>> selNotes)
    {
        selectedNotes.clear();
        for (int i=0;i<selNotes.size();i++)
            selectedNotes.add(selNotes.at(i)->currentStep);
        displayedSelection = selectedNotes;
    }
    
    void setSelectedNotes(Array<int> sel)
    {
        selectedNotes = sel;
        processor->setCopyOfSelectedNotes(selectedNotes);
        for (int i=0;i<processor->sequenceObject.theSequence.size();i++)
            if (sel.contains(processor->sequenceObject.theSequence.at(i)->currentStep))
                processor->sequenceObject.theSequence.at(i)->isSelected = true;
            else
                processor->sequenceObject.theSequence.at(i)->isSelected = false;
            
        for (int i=0;i<processor->sequenceObject.chords.size();i++)
        {
            processor->sequenceObject.chords.at(i).chordSelected = true;
            for (int j=0;j<processor->sequenceObject.chords.at(i).notePointers.size();j++)
            {
                if(!selectedNotes.contains(processor->sequenceObject.chords.at(i).notePointers.at(j)->currentStep))
                {
                    processor->sequenceObject.chords.at(i).chordSelected = false;
                    break;
                }
            }
        }
    }
    inline Array<int> getSelectedNotes()
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
            processor->sequenceObject.chords.at(i).chordSelected = false;
        for (int i=0;i<processor->sequenceObject.theSequence.size();i++)
            processor->sequenceObject.theSequence.at(i)->isSelected = false;
        repaint();
    }
    void selectAll()
    {
        selectedNotes.clear();
        displayedSelection.clear();
        newlySelectedNotes.clear();
        for (int step=0;step<processor->sequenceObject.theSequence.size();step++)
        {
//            if (processor->sequenceObject.isActiveTrack(processor->sequenceObject.theSequence.at(step)->track))
            selectedNotes.add(step);
            processor->sequenceObject.theSequence.at(step)->isSelected = true;
            displayedSelection.add(step);
        }
        processor->setCopyOfSelectedNotes(selectedNotes);
        repaint();
    }
    
    
    Value showingVelocities; //Edit velocity mode, toggled by graphVelocities toolbar button
    bool prevShowingVelocities = false;
    Value drawingVelocities; //Draw velocity mode, toggled by drawVelocities toolbar button
    bool prevDrawingVelocities = false;
//    Array<float> preDragOrDrawVelocities;   //Group velocity drags into an undo transaction - to reset viewer state before perform
    Array<float> velocitiesAfterDragOrDraw; //Used to group velocity drags into an undo transaction - passed to createAction
    Value adjustingingVelocities; //Adjust velocity mode, toggled by adjustVelocities toolbar button
    bool prevAdjustingVelocities = false;
    bool altKeyPressed; //Holding this key enables editingVelocities
    bool marqueeAddingNotes; //True if in marquee add note mode
    bool marqueeRemovingNotes; //If in marquee remove note mode
    bool markingSelectedNotes; //True if in mode to mark or clear target notes
    bool clearingSelectedNotes; //If in markingMode: True if marking target notes, false if clearing
    
    bool showingChords;
    void setShowingChords(bool showing)
    {
        showingChords = showing;
        std::cout << "showingChords " << showingChords<<"\n";
        repaint();
    }
    
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
    int noteBeingDraggedOn;
//    bool showVelocityIndicator; //If true, the velocity tick mark indicator of the hoverStep is to be painted
    float velStartDrag;
    Array<int> notesBeingDraggedOn;
    Array<float> velsStartDrag;
    enum VelocityDragTypes {VelocityDragLeftEnd, VelocityDragRightEnd, VelocityDragMiddle};
    VelocityDragTypes velocityDragType;
    double selectionLeftTick; //Used in vel dragging
    double selectionWidthTicks; //Used in vel dragging
    int lowestVelDragStep;
    float timeStartDrag;
    float offTimeStartDrag;
    double timeAfterDrag;
    double deltaTimeDrag;
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
    String hoverInfo = String();
#define HOVER_NONE 0
#define HOVER_NOTEHEAD 1
#define HOVER_NOTEBAR 2
#define HOVER_NOTETRACK 3
#define HOVER_ZEROTIMEHANDLE 4
#define HOVER_ZEROTIMELINE 5
#define HOVER_TOPBAR 6
#define HOVER_CHORD 7
    int hoveringOver;
    Point<float> hoverPosition;
    
    int numerator, denominator; //Of time signature
    double ticksPerQuarter;
    
    bool refresh = true;
    int nKeys;
    SortedSet<int> notesUsed;
//    float trackVerticalSize;
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
//    float x;
//    float yy;
    float pos = 0;
    float desktopScale;
    float horizontalScale;
//    float verticalScale;
    float prevMouseDragX;
    int positionLine;

    int firstNewNote, lastNewNote;
    
    int firstNoteBarRect;
    int nextNoteRect;  //Index of green rectangle that shows time of next note due
    
//    Array<Vertex> vertices;
//    Array<int> indices;
    const char* vertexShader;
    const char* fragmentShader;
    
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<OpenGLShaderProgram::Attribute> position, sourceColour;
    ScopedPointer<Uniforms> uniforms;
    
//    String newVertexShader, newFragmentShader;
    
    Colour colourActiveNoteHead;
    Colour colourPrimaryNoteBar;
    Colour colourNoteOn;
    Colour colourInactiveNoteHead;
    
    StringArray iconNames;
    OwnedArray<Drawable> iconsFromZipFile;
    int createImageFromZipFileSVG (const String& filename)
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
                    //                        std::cout << "file " << icons.getEntry(i)->filename<<"\n";
                    iconNames.add (icons.getEntry(i)->filename);
                    iconsFromZipFile.add (Drawable::createFromImageDataStream (*svgFileStream));
                }
            }
        }
//        Drawable* image = iconsFromZipFile [iconNames.indexOf (filename)]->createCopy();
        return iconNames.indexOf (filename);
    }
    
    int getSizeOfPathType (Path::Iterator::PathElementType t)
    {
        switch (t)
        {
            case (Path::Iterator::startNewSubPath):
                return 3;
            case (Path::Iterator::lineTo):
                return 3;
            case (Path::Iterator::quadraticTo):
                return 5;
            case (Path::Iterator::cubicTo):
                return 7;
            case (Path::Iterator::closePath):
                return 1;
            default:
                return 0;
        }
    }
    
    int getNumElements (Path p)
    {
        Path::Iterator i (p);
        int numElements = 0;
        while (i.next() )
        {
            numElements += getSizeOfPathType (i.elementType);
        }
        
        return numElements;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollingNoteViewer)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_B11C4CC4490D982E__
