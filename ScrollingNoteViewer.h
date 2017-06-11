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
#include "PluginProcessor.h"
#include <array>
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
    ScrollingNoteViewer (ReExpressorAudioProcessor*);
    ~ScrollingNoteViewer();
    
    void changeListenerCallback (ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        std::cout << "Change Reported in NoteViewer\n";
    }
    
    virtual void mouseDown (const MouseEvent& event) override;
    virtual void mouseUp (const MouseEvent& event) override;
    virtual void mouseDoubleClick (const MouseEvent& event) override;
    virtual void mouseDrag (const MouseEvent& event) override;
    virtual void mouseWheelMove (const MouseEvent& event,
                                 const MouseWheelDetails& wheel) override;
    virtual void mouseMagnify (const MouseEvent& event, float scaleFactor) override;
    
    OpenGLContext openGLContext;
    virtual void newOpenGLContextCreated() override;
    virtual void renderOpenGL() override;
    virtual void openGLContextClosing() override;
    
    Matrix3D<float> getProjectionMatrix(float horizScale, float vertScale) const;
    Matrix3D<float> getViewMatrix(float x) const;
    void setRectangleColour (int rect, float r, float g, float b);
    int addRectangle(float x, float y, float w, float h, Colour col);
    int addRectangle(float x, float y, float w, float h, float r, float g, float b);
    int addNote(float x, float yy, float w, float hh, float r, float g, float b);
    void setRectanglePos(int rect, float x, float y, float w, float h);
    void createShaders();
    
    void makeKeyboard();
    Image getKeysImage() { return keysImage; }
    int getKeysWidth() {return leftMargin+wKbd;}
    int getCommandBar() { return commandBar;}
    int getTopMargin() { return verticalScale*topMargin;}
    void makeNoteBars();
    bool sequenceChanged; //Set true to rebuild note bar GL index and vertex buffers
    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    void timerCallback(int id) override;
    
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
        mouseOverKeyOverlayColourId     = 0x1005003,  /**< Will be overlaid on the normal note colour.*/
        keyDownOverlayColourId          = 0x1005004,  /**< Will be overlaid on the normal note colour.*/
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
    
private:
    ReExpressorAudioProcessor *processor;
    
    double timeInTicks;
    int leadTimeInMeasures;
//    int leadTimeInPixels;
    int seqEndInPixels;
    float wKbd;
    int maxNote;
    int minNote;
    bool compressNotes;
    int octaveNumForMiddleC;
    const bool blackNote[11] = {false,true,false,true,false,false,true,false,true,false,true};
    float commandBar;
    float topMargin;
    float leftMargin;
    int initialMeasuresAcrossWindow;
    float initialWidth;
    float initialHeight;
    
    bool zoomDragStarting;
    bool zoomOrScrollDragging;
    float preDragScale;
    Point<float> mouseBeforeDrag;
    
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
    float pixelsPerTick;
    bool grabbedInitialWindowSize;

    //For OpenGL
    int frameCounter = 0;
    GLuint vertexBuffer, indexBuffer;
    int numIndices;
    float delta = 0.002;
    float x;
    float y;
    float pos = 0;
    float desktopScale;
    float horizontalScale;
    float verticalScale;
    float horizontalShift;  //Shift in pixels due to drag left or right on the resize bar
    float prevMouseDragX;
    int positionLine;
    
    int lastPlayedSeqStep; //Up to this step note bars are highlighted as played
    int newLastPlayedSeqStep; //When not -1, indicates to update lastPlayedSeqStep highlighting in GL render callback
    int prevNewLastPlayedSeqStep;
    
    int firstNewNote, lastNewNote;
    
    int firstNoteBarRect;
    
    Array<Vertex> vertices;
    Array<int> indices;
    const char* vertexShader;
    const char* fragmentShader;
    
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<OpenGLShaderProgram::Attribute> position, sourceColour;
    ScopedPointer<Uniforms> uniforms;
    
    String newVertexShader, newFragmentShader;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollingNoteViewer)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_B11C4CC4490D982E__
