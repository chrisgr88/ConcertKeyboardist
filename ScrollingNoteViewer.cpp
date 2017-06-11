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
#include "ScrollingNoteViewer.h"
#include <array>

//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

GLint ScrollingNoteViewer::Uniforms::viewMatrixHandle;
GLint ScrollingNoteViewer::Uniforms::projectionMatrixHandle;

#define PAINT_TIMER 0
#define NOTE_PLAY_TIMER 2

//==============================================================================
ScrollingNoteViewer::ScrollingNoteViewer (ReExpressorAudioProcessor* p) :
processor(p),
wKbd(24.f),
maxNote(84),
minNote(59),
compressNotes(false),
octaveNumForMiddleC (3),
commandBar(15),
topMargin(17),
leftMargin(24),
noteBarWidthRatio(1.f) //As fraction of note track width
{
    openGLContext.setMultisamplingEnabled(true);
    OpenGLPixelFormat format;
    format.multisamplingLevel = 4;
    openGLContext.setPixelFormat(format);
    
    setPaintingIsUnclipped(true);
    x = 0;
    y = 0;
    timeInTicks = 0.0;
    leadTimeInMeasures = 2.0;
    position = nullptr;
    sourceColour = nullptr;
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);
    openGLContext.setContinuousRepainting (true);
    horizontalScale = 1.0f;
    verticalScale = 1.0f;
    initialMeasuresAcrossWindow = 5;
    newLastPlayedSeqStep = -1;
    lastPlayedSeqStep = -1; //-1 if no notes played
    startTimer (PAINT_TIMER, 1000/30);
    //    startTimer (CLOCK_TIMER, 1000/30);
    startTimer (NOTE_PLAY_TIMER, 1000/30);
    grabbedInitialWindowSize = false;
    zoomDragStarting = false;
    zoomOrScrollDragging = false;
    horizontalShift = 0.f;
}

ScrollingNoteViewer::~ScrollingNoteViewer()
{
    openGLContext.setRenderer(NULL);
    openGLContext.setContinuousRepainting (false);
    openGLContext.detach();
}

void ScrollingNoteViewer::mouseDown(const MouseEvent &e)
{
    //We defer pickup of up the mouse position until drag actually starts because the mouseDown position is slightly
    //different from the first position returned in mouseDrag.
    if (e.position.getY()<commandBar*verticalScale) {
        zoomDragStarting = true;
    }
}
void ScrollingNoteViewer::mouseUp (const MouseEvent& event)
{
    zoomDragStarting = false;
    zoomOrScrollDragging = false;
}
void ScrollingNoteViewer::mouseDoubleClick (const MouseEvent& event)
{
    horizontalScale = 1.f;
    horizontalShift = 0.f;
    repaint();
}
void ScrollingNoteViewer::mouseDrag (const MouseEvent& event)
{
    if (zoomDragStarting)
    {
        zoomOrScrollDragging = true;
        preDragScale = horizontalScale;
        mouseBeforeDrag = Point<float>(event.position.getX(), event.position.getY());
        prevMouseDragX = event.position.getX();
        zoomDragStarting = false;
    }
    if (zoomOrScrollDragging)
    {
        float scaleChange = mouseBeforeDrag.getY()-event.position.getY();
//        std::cout << "scaleChange " << scaleChange << "\n";
        float absScaleChange = fabsf(scaleChange)/300.f;
        const float minScale = 0.01f;
        float rawScale;
        if (mouseBeforeDrag.getY()>event.position.getY())
            rawScale = preDragScale+absScaleChange;
        else
            rawScale = preDragScale-absScaleChange;
//        std::cout << "rawScale " << event.position.getY() << " " << rawScale << "\n";
        horizontalScale = fmaxf(minScale,rawScale);
//        std::cout << "scale " <<minScale<<" "<<preDragScale<<" "<<absScaleChange << " " << horizontalScale << "\n";
        prevMouseDragX = event.position.getX();
        repaint();
    }
}
void ScrollingNoteViewer::mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel)
{
    
}
void ScrollingNoteViewer::mouseMagnify (const MouseEvent& event, float scaleFactor)
{
    
}

int ScrollingNoteViewer::addNote(float x, float y,  float w, float h, float r, float g, float b)
{
    const float barFract = 4.f;
    const float maxHeaderWidth = 6.0f;
    float headerWidth;
    if (w < 2.f*maxHeaderWidth)
    {
        headerWidth = w/2.f;
    }
    else
    {
        headerWidth = maxHeaderWidth;
    }
    int id = addRectangle(x, y, headerWidth, h, r, g, b); //Header
    addRectangle(x, y+h/2., w, h/barFract, r, g, b); //Bar
    return id;
}

int ScrollingNoteViewer::addRectangle(float x, float yy, float w, float hh, Colour col)
{
    return addRectangle(x, yy, w, hh, col.getFloatRed(), col.getFloatGreen(), col.getFloatBlue());
}

int ScrollingNoteViewer::addRectangle(float x, float yy, float w, float hh, float r, float g, float b)
{
    const int firstVertex = vertices.size(); //There are four vertices per rectangle
    float y = initialHeight-yy;
    float h = -hh;
    //Define the four vertices of a rectangle
    Vertex v0 =
    {
        { x, y},
        { r, g, b}
    };
    vertices.add(v0);
    Vertex v1 =
    {
        { x, y+h},
        { r, g, b}
    };
    vertices.add(v1);
    Vertex v2 =
    {
        { x+w, y+h},
        { r, g, b}
    };
    vertices.add (v2);
    Vertex v3 =
    {
        { x+w, y},
        { r, g, b}
    };
    vertices.add (v3);
    //Define the order in which the vertices of the rectangle are used
    indices.add(0+firstVertex);
    indices.add(1+firstVertex);
    indices.add(2+firstVertex);
    indices.add(0+firstVertex);
    indices.add(2+firstVertex);
    indices.add(3+firstVertex);
    return firstVertex/4;
}

Matrix3D<float> ScrollingNoteViewer::getProjectionMatrix(float horizScale, float vertScale) const
{
    return Matrix3D<float>::fromFrustum (0.f, getWidth()/horizScale, 0.f, getHeight()/vertScale, 4.0f, 30.0f);
}

Matrix3D<float> ScrollingNoteViewer::getViewMatrix(float x) const
{
    Matrix3D<float> viewMatrix (Vector3D<float> (x, 0.0f, -4.0f));
    Matrix3D<float> rotationMatrix = viewMatrix.rotated (Vector3D<float> (0.0f, 0.0f, 0.0f));
    return rotationMatrix * viewMatrix;
}

void ScrollingNoteViewer::setRectangleColour (int rect, float r, float g, float b)
{
    float colour[3] = {r,g,b};
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect) + 8,
                    sizeof(colour),
                    colour);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+1) + 8,
                    sizeof(colour),
                    colour);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+2) + 8,
                    sizeof(colour),
                    colour);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+3) + 8,
                    sizeof(colour),
                    colour);
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void ScrollingNoteViewer::setRectanglePos(int rect, float x, float yy, float w, float hh)
{
    float y = getHeight()*verticalScale-yy;
    float h = -hh;
    
    float v0[2] = { x, y};
    float v1[2] = { x, y+h};
    float v2[2] = { x+w, y+h};
    float v3[2] = { x+w, y};
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect),
                    sizeof(v0),
                    v0);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+1),
                    sizeof(v1),
                    v1);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+2),
                    sizeof(v2),
                    v2);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+3),
                    sizeof(v3),
                    v3);
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
}

//initialise
void ScrollingNoteViewer::newOpenGLContextCreated()
{

    
    ////Vertex buffer (called the "GL_ARRAY_BUFFER")
    // ************************** TRIANGLES DEFINITION
    
//    makeNoteBars();
    createShaders();
    
//    openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
//    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
//    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
//                                           static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof (Vertex)),
//                                           vertices.getRawDataPointer(), GL_DYNAMIC_DRAW);
//    
//    numIndices = 6*(vertices.size()/4);
//    //generate buffer object name(s) (names are ints) (indexBuffer is an GLuint)
//    openGLContext.extensions.glGenBuffers (1, &indexBuffer); //Gets id of indexBuffer
//    
//    //bind a named buffer object (to a buffer type such as GL_ELEMENT_ARRAY_BUFFER)
//    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
//    openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER,
//                                           static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof (juce::uint32)),
//                                           indices.getRawDataPointer(), GL_STATIC_DRAW);
    ////Projection & View Matrices
    if (uniforms->projectionMatrix != nullptr)
        uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix(horizontalScale, verticalScale).mat, 1, false);
    
    if (uniforms->viewMatrix != nullptr)
        uniforms->viewMatrix->setMatrix4 (getViewMatrix(0.f).mat, 1, false);
}

//render
void ScrollingNoteViewer::renderOpenGL()
{
    ++frameCounter;
    jassert (OpenGLHelpers::isContextActive());
    
    desktopScale = (float) openGLContext.getRenderingScale();
    OpenGLHelpers::clear (Colour::greyLevel (0.1f));
    
    if  (processor->getSequenceChanged())
    {
        makeKeyboard ();
        sendChangeMessage();
        makeNoteBars ();
        processor->setSequenceChanged(false);
    }
    
    glEnable (GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
    shader->use();
    
    const double time = processor->getTimeInTicks();
    float timeShiftInPixels = -time*pixelsPerTick;
    glUniformMatrix4fv(Uniforms::viewMatrixHandle, 1, false, getViewMatrix(timeShiftInPixels+horizontalShift/horizontalScale).mat);
    glUniformMatrix4fv(Uniforms::projectionMatrixHandle, 1, false, getProjectionMatrix(horizontalScale, verticalScale).mat);
    
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    
    //Specify offset to each attribute in a vector node
    // 0 is offset into the Vertex of the "position"
    if (position != nullptr)
    {
        openGLContext.extensions.glVertexAttribPointer (position->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
        openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
    }
    
    //Below: "(GLvoid*) (sizeof (float) * 6))" is offset into the Vertex of the "sourceColour"
    if (sourceColour != nullptr)
    {
        openGLContext.extensions.glVertexAttribPointer (sourceColour->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 2));
        openGLContext.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
    }
    
    //**** Render primitives from array data.  GL_TRIANGLES is the type of primitive
    glDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    
    //glDisableVertexAttribArray disables a generic vertex attribute array
    if (position != nullptr)
        openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
    if (sourceColour != nullptr)
        openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
    
    // Reset the element buffers so child Components draw correctly
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    
    //Reset all to unplayed
    if (newLastPlayedSeqStep<lastPlayedSeqStep) {
        const int nRect = 2 * processor->getSequence()->size();
        for (int i=0; i<nRect; i++)
            setRectangleColour(firstNoteBarRect+i, 1.f, 0.f, 0.f);
        lastPlayedSeqStep = -1;
    }
    
    if (newLastPlayedSeqStep>lastPlayedSeqStep) {
        
        //Already-played notes
        if (lastPlayedSeqStep!=-1) { //If -1 then no previous value
            for (int i=firstNewNote; i<=lastNewNote;i++)
            {
                setRectangleColour(firstNoteBarRect+i*2,   .6f, 0.f, 0.0f);
                setRectangleColour(firstNoteBarRect+i*2+1, .6f, 0.f, 0.0f);
//                std::cout << "mark played note "<<i<<"\n";
            }
        }

        //Mark latest-played note(s)
        firstNewNote = lastPlayedSeqStep+1;
        lastNewNote = newLastPlayedSeqStep;
        for (int i=firstNewNote; i<=lastNewNote;i++)
        {
//            std::cout << "mark newest note "<<i<<"\n";
            setRectangleColour(firstNoteBarRect+i*2,   0.0f, 1.0f, 0.0f);
            setRectangleColour(firstNoteBarRect+i*2+1, 0.6f, 0.0f, 0.0f);
        }
        
        //Mark next to be played note(s)
//        std::vector<NoteWithOffTime> *sequence = processor->getSequence();
//        int nextSeqStep = lastNewNote+1;
//        if (nextSeqStep < sequence->size())
//        {
//            const int nextTimeStamp = sequence->at(nextSeqStep).getOnTime();
//            while (nextSeqStep < sequence->size() && sequence->at(nextSeqStep).getTimeStamp() == nextTimeStamp)
//            {
//                setRectangleColour(firstNoteBarRect+nextSeqStep, 0.f, 1.f, 0.f);
//                nextSeqStep+=1;
//            }
//        }
        
        lastPlayedSeqStep = newLastPlayedSeqStep;
    }
}

//shutdown openGL
void ScrollingNoteViewer::openGLContextClosing()
{
    shader = nullptr;
    uniforms = nullptr;
    openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
    openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
}

void ScrollingNoteViewer::createShaders()
{
    vertexShader =
    "attribute vec4 position;\n"
    "attribute vec3 sourceColour;\n"
    "\n"
    "uniform mat4 projectionMatrix;\n"
    "uniform mat4 viewMatrix;\n"
    "\n"
    "varying vec3 destinationColour;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    destinationColour = sourceColour;\n"
    "    gl_Position = projectionMatrix * viewMatrix * position;\n"
    "}\n";
    
    fragmentShader =
#if JUCE_OPENGL_ES
    "varying lowp vec3 destinationColour;\n"
#else
    "varying vec3 destinationColour;\n"
#endif
    "\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(destinationColour,1.0); \n"
    "}\n";
    
    ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
    String statusText;
    
    if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
        && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
        && newShader->link())
    {
        shader = newShader;
        uniforms = nullptr;
        shader->use();
        
        if (openGLContext.extensions.glGetAttribLocation (shader->getProgramID(), "position") < 0)
            position      = nullptr;
        else
            position      = new OpenGLShaderProgram::Attribute (*shader,    "position");
        
        if (openGLContext.extensions.glGetAttribLocation (shader->getProgramID(), "sourceColour") < 0)
            sourceColour      = nullptr;
        else
            sourceColour  = new OpenGLShaderProgram::Attribute (*shader,    "sourceColour");
        
        uniforms   = new Uniforms (openGLContext, *shader);
//        std::cout << "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2) + " Shaders created" << "\n";
    }
    else
    {
        std::cout <<  newShader->getLastError()<<"\n";
    }
}

//==============================================================================
void ScrollingNoteViewer::makeKeyboard() {
    ticksPerQuarter = processor->getPPQ();
    timeSigChanges = processor->getTimeSigInfo();
    timeSigChanges[0].getTimeSignatureInfo(numerator, denominator);
    pixelsPerTick = initialWidth/(ticksPerQuarter * numerator * initialMeasuresAcrossWindow);
    notesUsed = processor->getNotesUsed(minNote,maxNote);
    nKeys = maxNote-minNote+1;
    keysImage = Image(Image::RGB, leftMargin+wKbd, getHeight(), true);
    Graphics keysGr (keysImage);
    
    const Colour textColour (findColour (textLabelColourId));
    trackVerticalSize = ((float)getHeight()-getTopMargin())/nKeys;
    
    //left margin
    keysGr.setColour(Colours::grey);
    keysGr.fillRect(0,0,roundToInt(leftMargin),roundToInt(getHeight()));
    //topMargin
    keysGr.setColour(Colours::black.brighter().brighter());
    keysGr.fillRect(0, 0, roundToInt(wKbd+leftMargin), getTopMargin());
    
//    keysGr.setColour (Colours::black);
    for (int note = maxNote; note >= minNote; note--) //Draw keys
    {
        const float noteY = (maxNote-note) * trackVerticalSize;
        noteYs[note]= noteY;
        const String text (getNoteText (note));
        const float fontHeight = jmin (12.0f, trackVerticalSize * 0.9f);
        if (blackNote[note%12])
        {
            keysGr.setColour (Colour(Colours::black));
            keysGr.fillRect((float)leftMargin, noteY+getTopMargin(), wKbd, trackVerticalSize);
        }
        else
        {
            keysGr.setColour (Colour(Colours::white));
            keysGr.fillRect((float)leftMargin, noteY+getTopMargin(), wKbd, trackVerticalSize);
            keysGr.setColour (Colour(Colours::black));
            keysGr.drawLine((float)leftMargin, noteY+trackVerticalSize+noteY+getTopMargin(),
                                    leftMargin+wKbd, noteY+trackVerticalSize+getTopMargin(),1.5f);
        }
        if (trackVerticalSize>6)
        {
            keysGr.setColour(Colours::white);
            keysGr.setFont (Font (fontHeight).withHorizontalScale (0.95f));
            keysGr.drawText (text,2, roundToInt(noteY+2.f+noteY+getTopMargin()), (int)wKbd-4,(int)trackVerticalSize-4,
                             Justification::centredLeft, false);
        }
    }
    keysGr.setColour(Colours::grey);
    keysGr.fillRect(roundToInt(wKbd+leftMargin-2), 0, 2, roundToInt(getHeight()));
}

void ScrollingNoteViewer::makeNoteBars()
{
    const float rescaleHeight = initialHeight/getHeight();
    const int leadTimeInTicks = leadTimeInMeasures*numerator*ticksPerQuarter;
    
    trackVerticalSize = ((float)initialHeight-topMargin)/nKeys;
    vertices.clear();
    indices.clear();
    std::vector<NoteWithOffTime> *sequence = processor->getSequence();
    notesUsed = processor->getNotesUsed(minNote,maxNote);
    nKeys = maxNote-minNote+1;
    const float h = trackVerticalSize*noteBarWidthRatio; //Note bar height
    const float trackGutter = (trackVerticalSize-h)/2.f;
    const int seqSize = sequence->size();
    if (seqSize==0)
        return;
    const int seqDurationInTicks = sequence->at(seqSize-1).getOffTime();
    const float noteBarVerticalSize = trackVerticalSize*noteBarWidthRatio;
    
    const int sequenceWidthPixels = (seqDurationInTicks+leadTimeInTicks)*pixelsPerTick;
    
    //Top margin
    addRectangle(0.f, 0.f, sequenceWidthPixels, topMargin, Colour(Colours::grey));
    addRectangle(0.f, 0.f, sequenceWidthPixels, 2.0f, 0.8f, 0.8f, 0.8f);
    
    //Black/white track highlighting
    for (int note = minNote;note<=maxNote;note++)
    {
        if (blackNote[note%12])
            addRectangle(0.f, noteYs[note]*rescaleHeight+topMargin, sequenceWidthPixels, trackVerticalSize, Colours::black);
        else
            addRectangle(0.f, noteYs[note]*rescaleHeight+topMargin, sequenceWidthPixels, trackVerticalSize, 0.3f, 0.3, 0.3);
    }

    //Beat & measure lines
    int tickPosition = leadTimeInTicks;
    for (int timeSigIndex=0;timeSigChanges.size()>timeSigIndex;timeSigIndex++)
    {
        timeSigChanges[timeSigIndex].getTimeSignatureInfo(numerator, denominator);
        const float ticksPerBeat = ticksPerQuarter * 4.f/denominator;
        
        int startTick;
        if (timeSigIndex==0)
            startTick = 0;
        else
            startTick = timeSigChanges[timeSigIndex].getTimeStamp();
        int endTick;
        if (timeSigChanges.size()>timeSigIndex+1)
            endTick = timeSigChanges[timeSigIndex+1].getTimeStamp();
        else
            endTick = seqDurationInTicks;
        
        const int sectionDurationInBeats = (endTick - startTick) / ticksPerBeat;
        for (int beat=0;beat<sectionDurationInBeats;beat++)
        {
            const float lineX = tickPosition*pixelsPerTick;
            if (beat%numerator == 0) //Measure
            {
                addRectangle(lineX-1.0f,0.f,1.0f,initialHeight, 0.8f, 0.8f, 0.8f);
            }
            else //Beat
            {
                addRectangle(lineX-1.0f,0.f,1.0f,initialHeight, .5f, .5f, .5f);
            }
            tickPosition += ticksPerBeat;
        }
    }
    
    //First and last line
    addRectangle(leadTimeInTicks*pixelsPerTick-1.0f,0.f,2.0f,initialHeight, 0.8f, 0.8f, 0.8f);
    addRectangle((leadTimeInTicks+seqDurationInTicks)*pixelsPerTick-1.0f,0.f,2.0f,initialHeight, 0.8f, 0.8f, 0.8f);
    seqEndInPixels = seqDurationInTicks*pixelsPerTick;
    //Note bars
    double nextOnTime = sequence->at(0).getOnTime();
    double longestOffTime = sequence->at(0).getOffTime();
    std::vector<NoteWithOffTime> summaryNoteOns;
    for (int index = 0;index<seqSize;index++)
    {
        NoteWithOffTime msg = sequence->at(index);
        const double onTime = msg.getOnTime();
        const double offTime = msg.getOffTime();
//        if (onTime!=nextOnTime)
//        {
//            MidiMessage noteOn = MidiMessage::noteOn(1,1,1.f);
//            noteOn.setTimeStamp(nextOnTime);
//            NoteWithOffTime msg = NoteWithOffTime(-1,noteOn,longestOffTime);
//            summaryNoteOns.push_back(msg);
//            nextOnTime = onTime;
//            longestOffTime = offTime;
//        }
//        else
//        {
//            if (offTime > longestOffTime)
//                longestOffTime = offTime;
//        }
        
        const int noteNumber = msg.getNoteNumber();
        float x = (onTime + leadTimeInTicks)*pixelsPerTick;
        const float y = noteYs[noteNumber]*rescaleHeight + trackGutter;
        float w = (offTime - onTime)*pixelsPerTick;
        int rectNum;
//        float trk = msg.getTrack();
        rectNum = addNote(x, y+topMargin, w, noteBarVerticalSize, 1.f, 0.f, 0.f);
        
        if (index == 0)
            firstNoteBarRect = rectNum;
    }
//    MidiMessage noteOn = MidiMessage::noteOn(1,1,1.f);
//    noteOn.setTimeStamp(nextOnTime);
//    NoteWithOffTime mmsg = NoteWithOffTime(-1,noteOn,longestOffTime);
//    summaryNoteOns.push_back(mmsg);
//    
//    for (int i=0;i<summaryNoteOns.size();i++)
//    {
//        NoteWithOffTime msg = sequence->at(i);
//        const double onTime = msg.getOnTime();
//        const double offTime = msg.getOffTime();
//        float x = (onTime + leadTimeInTicks)*pixelsPerTick;
//        float w = (offTime - onTime)*pixelsPerTick;
//        int rectNum;
//        rectNum = addNote(x, 6.f, w, 10.f, 0.f, 1.f, 0.f);
//    }
    
    openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                           static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof (Vertex)),
                                           vertices.getRawDataPointer(), GL_DYNAMIC_DRAW);
    
    numIndices = 6*(vertices.size()/4);
    //generate buffer object name(s) (names are ints) (indexBuffer is an GLuint)
    openGLContext.extensions.glGenBuffers (1, &indexBuffer); //Gets id of indexBuffer
    
    //bind a named buffer object (to a buffer type such as GL_ELEMENT_ARRAY_BUFFER)
    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                                           static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof (juce::uint32)),
                                           indices.getRawDataPointer(), GL_STATIC_DRAW);
}

void ScrollingNoteViewer::paint (Graphics& g)
{
    //Current time indicator line
    g.setColour (Colour(50,50,255));
    const float currentTimeIndicatorPos = horizontalShift+horizontalScale*(ticksPerQuarter*leadTimeInMeasures*numerator*pixelsPerTick);
    g.fillRect(Rectangle<float>(currentTimeIndicatorPos-1.f,0.f,
                                2.0, getHeight()));
}

void ScrollingNoteViewer::timerCallback(int id)
{
    static int prev;
    if (id == PAINT_TIMER)
    {
        newLastPlayedSeqStep = processor->getLatestPlayedSequenceStep();
        if (prev!=newLastPlayedSeqStep) {
//            std::cout << "newLastPlayedSeqStep "<<newLastPlayedSeqStep<<"\n";
            prev = newLastPlayedSeqStep;
        }
    }
    else if (id == NOTE_PLAY_TIMER) {
        x=x-1;
        if (x==-5000)
            x=0;
    }
}

void ScrollingNoteViewer::resized()
{
    if (!grabbedInitialWindowSize)
    {
        initialWidth = getWidth();
        initialHeight = getHeight();
        grabbedInitialWindowSize = true;
    }
    verticalScale = (float)getHeight()/initialHeight;
    makeKeyboard ();
    sendChangeMessage();
}

String ScrollingNoteViewer::getNoteText (const int midiNoteNumber)
{
    return MidiMessage::getMidiNoteName (midiNoteNumber, true, true, octaveNumForMiddleC);
}

//==============================================================================
#if 0
/*  -- Projucer information section --
 
 This is where the Projucer stores the metadata that describe this GUI layout, so
 make changes in here at your peril!
 
 BEGIN_JUCER_METADATA
 
 <JUCER_COMPONENT documentType="Component" className="ScrollingNoteViewer" componentName=""
 parentClasses="public Component" constructorParams="" variableInitialisers=""
 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
 fixedSize="0" initialWidth="600" initialHeight="400">
 <BACKGROUND backgroundColour="ffffffff"/>
 </JUCER_COMPONENT>
 
 END_JUCER_METADATA
 */
#endif
//[EndFile] You can add extra defines here...
//[/EndFile]

