
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
//==============================================================================
ScrollingNoteViewer::ScrollingNoteViewer (MIDIProcessor *p) :
processor(p),
wKbd(24.f),
maxNote(84),
minNote(59),
compressNotes(false),
octaveNumForMiddleC (3),
toolbarHeight(30),
topMargin(15),
leftMargin(2),
noteBarWidthRatio(1.f) //As fraction of note track width
{
    processor->initialWindowHeight = 88;
    glBufferUpdateCountdown = 0; //Countdown of number of renders to pass before another buffer update is allowed.
    sequenceChanged = false;
    prevFileLoaded = File();
    processor->addChangeListener(this); //Sent at the end of rewind()
    processor->sequenceObject.addChangeListener(this); //Send at the end of saveSequence()
    openGLContext.setMultisamplingEnabled(true);
    OpenGLPixelFormat format;
    format.multisamplingLevel = 4;
    openGLContext.setPixelFormat(format);
    hoverStep = -1;
    
    setPaintingIsUnclipped(true);
    x = 0;
    y = 0;
    position = nullptr;
    sourceColour = nullptr;
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);
    openGLContext.setContinuousRepainting (true);
    horizontalScale = 1.0f;
    verticalScale = 1.0f;
    initialMeasuresAcrossWindow = 5;
    leadTimeProportionOfWidth = 0.4;
    grabbedInitialWindowSize = false;
    zoomDragStarting = false;
    zoomOrScrollDragging = false;
    selecting = false;
    resetHorizontalShift();
    colourActiveNoteHead = Colour(0xffff00ff);//Cyan
    colourPrimaryNoteBar = Colour(0xffffccff).darker().darker();
    colourInactiveNoteHead = Colour (0xffbbbbbb);
    colourNoteOn = Colour(0xffFFFF55);
    nSteps = -1;
}

ScrollingNoteViewer::~ScrollingNoteViewer()
{
    openGLContext.setContinuousRepainting (false);
    openGLContext.detach();
    openGLContext.setRenderer(NULL);
    processor->removeChangeListener(this);
    processor->sequenceObject.removeChangeListener(this);
}

void ScrollingNoteViewer::mouseDown(const MouseEvent &e)
{
    //We defer pickup of up the mouse position until drag actually starts because the mouseDown position is slightly
    //different from the first position returned in mouseDrag.
    newlySelectedNotes.clear();
//    std::cout << "Selected steps ";
//    for (int j=0;j<displayedSelection.size();j++)
//        std::cout << " " << displayedSelection[j];
//    std::cout  <<" hover step "<< hoverStep <<" "<<displayedSelection.contains(hoverStep)<<"\n";
    if (!ModifierKeys::getCurrentModifiers().isCommandDown() && hoveringOver != HOVER_NONE)
    {
        if (hoveringOver == HOVER_NOTEBAR && !selectedNotes.contains(hoverStep))
        {
            displayedSelection.clear();
            clearSelectedNotes();
        }
    }
    selectionRect = Rectangle<int>();
    repaint();
    if (processor->isPlaying )
    {
//        setEditable(true);
        processor->play(false,"current");
    }
    if (e.position.getY()<topMargin*verticalScale) { //Test mouse position
        zoomDragStarting = true;
    }
    else if (hoveringOver != HOVER_NOTEBAR && hoveringOver != HOVER_ZEROTIMELINE)
    {
        selectionAnchor = Point<int>(e.getPosition().getX(),e.getPosition().getY());
        startTimer(TIMER_MOUSE_HOLD, 1);
    }
//    std::cout << "mouse down " <<e.getPosition().getX()<<" "
//    << selectionRect.getTopLeft().getX()<<" "<<selectionRect.getTopLeft().getY()<< "\n";
}
void ScrollingNoteViewer::mouseUp (const MouseEvent& event)
{
    if (selecting)
    {
        selecting = false;
        if (!event.source.hasMouseMovedSignificantlySincePressed())
        {
            clearSelectedNotes();
//            newlySelectedNotes.clear();
//            displayedSelection.clear();
        }
        else
        {
            setSelectedNotes(displayedSelection);
        }
        repaint();
        return;
    }
    stopTimer(TIMER_MOUSE_HOLD);
    const double x = event.position.getX();
    const double y = event.position.getY();
    const double vert = (y - getTopMargin())/trackVerticalSize;
    const double xInTicks = ((x - (horizontalShift+sequenceStartPixel))/pixelsPerTick)/horizontalScale + processor->getTimeInTicks();
    const float sequenceScaledX = xInTicks*pixelsPerTick;
    const float scaledY = y/verticalScale;
    preDragXinTicks = xInTicks;
    if (vert>0.0 && xInTicks>0.0) //Test if we are on a note bar
    {
        std::vector<NoteWithOffTime> *sequence = processor->sequenceObject.getSequence();
        int i;
        int step = -1;
        for (i=0;i<sequence->size();i++)
        {
            if (sequence->at(i).head.contains(sequenceScaledX, scaledY))
            {
                step = i;
                break;
            }
        }
        hoverStep = step;
        if (step!=-1)
        {
            hoverStep = step;
            hoveringOver = HOVER_NOTEBAR;
        }
    }
    
    zoomDragStarting = false;
    zoomOrScrollDragging = false;
    if (hoveringOver==HOVER_ZEROTIMEHANDLE) //Clicked on ZTL handle - add/remove bookmark
    {
        if (!processor->atZTL())
            processor->catchUp();
        else
        {
            processor->catchUp();
            processor->addRemoveBookmark(BOOKMARK_TOGGLE);
        }
    }
    else if (hoveringOver == HOVER_NOTEBAR)
    {
        if (processor->getNotesEditable() && hoverStep>=0) //We do the actual work on the message thread by calling a timer that turns itself off after one tick.
            startTimer(TIMER_TOGGLE_TARGET_NOTE, 1);
    }
    else if (hoveringOver==HOVER_ZEROTIMELINE || hoveringOver==HOVER_ZEROTIMEHANDLE) //Clicked on ZTL - make this the current time
    {
        double xInTicks = processor->getTimeInTicks()+((event.position.getX() -
                                                        (horizontalShift+sequenceStartPixel))/pixelsPerTick)/horizontalScale;
        processor->setXInTicks(horizontalShift*pixelsPerTick/horizontalScale);
        processor->rewind(xInTicks);
    }
    else
        hoveringOver = HOVER_NONE;
    repaint();
}
void ScrollingNoteViewer::mouseDoubleClick (const MouseEvent& e)
{
    if (e.position.getY()<topMargin*verticalScale) { //Test mouse position
        horizontalScale = 1.f;
        setHorizontalShift(0.f);
        repaint();
    }
}

void ScrollingNoteViewer::mouseDrag (const MouseEvent& event)
{
    if (!event.source.hasMouseMovedSignificantlySincePressed())
        return;
    stopTimer(TIMER_MOUSE_HOLD);
    curDragPosition = event.getPosition();
    startTimer(TIMER_MOUSE_DRAG, 1);
}
void ScrollingNoteViewer::mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel)
{
    processor->leadLag = 0;
    if (processor->isPlaying )
        processor->play(false,"current");
    float newShift = horizontalShift-300*wheel.deltaX;
    const double seqStartRelToLeftEdgeInPixels =
        (sequenceStartPixel - processor->getTimeInTicks()*pixelsPerTick*horizontalScale +
         newShift)*horizontalScale;
    double seqDurationInPixels = processor->sequenceObject.seqDurationInTicks*pixelsPerTick;
    double scaledSeqDurationInPixels = seqDurationInPixels*horizontalScale*horizontalScale;
    const double seqEndRelToLeftEdgeInPixels = seqStartRelToLeftEdgeInPixels + scaledSeqDurationInPixels;
    if (seqStartRelToLeftEdgeInPixels > sequenceStartPixel*horizontalScale)
        newShift = processor->getTimeInTicks()*pixelsPerTick*horizontalScale;
    else if (seqEndRelToLeftEdgeInPixels < sequenceStartPixel*horizontalScale)
        newShift =  processor->getTimeInTicks()*pixelsPerTick*horizontalScale - scaledSeqDurationInPixels/horizontalScale;
    setHorizontalShift(newShift);
    repaint();
}
void ScrollingNoteViewer::mouseMagnify (const MouseEvent& event, float scaleFactor)
{
    
}
void ScrollingNoteViewer::mouseMove (const MouseEvent& event)
{
    const double x = event.position.getX();
    const double y = event.position.getY();
    const double vert = (y - getTopMargin())/trackVerticalSize;
    const double xInTicks = ((x - (horizontalShift+sequenceStartPixel))/pixelsPerTick)/horizontalScale + processor->getTimeInTicks();
    const float sequenceScaledX = xInTicks*pixelsPerTick;
    const float scaledY = y/verticalScale;
    preDragXinTicks = xInTicks;
    if (vert>0.0 && xInTicks>0.0) //Test if we are on a note bar
    {
        std::vector<NoteWithOffTime> *sequence = processor->sequenceObject.getSequence();
        const int nn = maxNote - vert + 1;
        int i;
        int step = -1;
        for (i=0;i<sequence->size();i++)
        {
            if (sequence->at(i).head.contains(sequenceScaledX, scaledY))
                {
                    step = i;
                    break;
                }
        }
        hoverStep = step;
        if (step==-1)
        {
            hoveringOver = HOVER_NOTETRACK;
            String note = MidiMessage::getMidiNoteName (nn, true, true, 3);
            hoverInfo = note;
        }
        else if (step)
        {
//            std::cout << "mouseMove found step " << step <<"\n";
            hoveringOver = HOVER_NOTEBAR;
            String note = MidiMessage::getMidiNoteName (nn, true, true, 3)
                + " nn:" + String::String(nn) + " ch:" + String::String(sequence->at(i).getChannel())
                + " vel:" + String((int)sequence->at(i).getVelocity());
            hoverInfo = note+" step:"+String::String(hoverStep) + " tick:" + String(sequence->at(i).getTimeStamp())
                + " dur:" + String((sequence->at(i).offTime-sequence->at(i).getTimeStamp()));
        }
//        std::cout << "mouseMove HOVER = " << hoveringOver << "\n";
        sendChangeMessage();  //This is NOT being sent to MidiProcessor
    }
    else if (!selecting && fabs(sequenceStartPixel-x)<=4)
    {
        //        double xInTicks = processor->getTimeInTicks()+((event.position.getX() -
        //                                                        (horizontalShift+sequenceStartPixel))/pixelsPerTick)/horizontalScale;
        //        processor->setXInTicks(xInTicks);
        
        if (0 < y && y && event.position.getY() < getTopMargin())
            hoveringOver = HOVER_ZEROTIMEHANDLE;
        else
        {
//            std::cout << "HOVER_ZEROTIMELINE" << "\n";
            hoveringOver = HOVER_ZEROTIMELINE;
        }
    }
    else
        hoveringOver = HOVER_NONE;
//    if (hoveringOver!=HOVER_NONE)
//        std::cout << "Hover " << hoveringOver << "\n";
}

int ScrollingNoteViewer::addNote(bool playable, float x, float y,  float w, float h, float headWidth, float headHeight,
                                 Colour colHead, Colour colBar)
{
    const float headY = y-(headHeight-h)/2.0;
    int id = addRectangle(x, y, w, h, colBar); //Bar
    if (playable)
        addRectangle(x, headY, headWidth, headHeight, colHead); //Header
    else
        addRectangle(x, headY, headWidth, headHeight, colHead);
    return id;
}

int ScrollingNoteViewer::addRectangle(float x, float yy, float w, float hh, Colour col)
{
    const int firstVertex = vertices.size(); //There are four vertices per rectangle
    float y = initialHeight-yy;
    float h = -hh;
    float r = col.getFloatRed();
    float g = col.getFloatGreen();
    float b = col.getFloatBlue();
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

int ScrollingNoteViewer::addQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Colour col)
{
    const int firstVertex = vertices.size(); //There are four vertices per quad
    float r = col.getFloatRed();
    float g = col.getFloatGreen();
    float b = col.getFloatBlue();
    //Define the four vertices of a rectangle
    Vertex v0 =
    {
        { x1, y1},
        { r, g, b}
    };
    vertices.add(v0);
    Vertex v1 =
    {
        { x2, y2},
        { r, g, b}
    };
    vertices.add(v1);
    Vertex v2 =
    {
        { x3, y3},
        { r, g, b}
    };
    vertices.add (v2);
    Vertex v3 =
    {
        { x4, y4},
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

int ScrollingNoteViewer::addEqTriangle (float x, float y, float w, float h, bool invert, Colour col)
{
    return addQuad(
                x+w/2, invert?(y-h):y,
                x-w/2, invert?(y-h):y,
                x+0.1f, invert?y:(y-h),
                x-0.1f, invert?y:(y-h),
                Colour(col)
                );
}

int ScrollingNoteViewer::addLine(float x1, float y1, float x2, float y2, float w, Colour col)
{
    double angle;
    if (y2>=y1)
    {
    if ((y2-y1)/(x2-x1) <= 10.0)
        angle = atan((y2-y1)/(x2-x1));
    else
        angle = atan(10.0);
    }
    else
    {
        if ((y1-y2)/(x2-x1) <= 10.0)
            angle = atan((y2-y1)/(x2-x1));
        else
            angle = -atan(10.0);
    }
    
    double dy = w/cos(angle);
    
    return addQuad(
                   x1, y1-dy/2,
                   x1, y1+dy/2,
                   x2, y2-dy/2,
                   x2, y2+dy/2,
                   Colour(col)
                   );
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

void ScrollingNoteViewer::setRectangleColour (int rect, Colour col)
{
    float r = col.getFloatRed();
    float g = col.getFloatGreen();
    float b = col.getFloatBlue();
    float colour[3] = {r, g, b};
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
    float y = initialHeight-yy;
//    float y = getHeight()*verticalScale-yy;
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
    createShaders();
    
    ////Projection & View Matrices
    if (uniforms->projectionMatrix != nullptr)
        uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix(horizontalScale, verticalScale).mat, 1, false);
    
    if (uniforms->viewMatrix != nullptr)
        uniforms->viewMatrix->setMatrix4 (getViewMatrix(0.f).mat, 1, false);
}

void ScrollingNoteViewer::resetHorizontalShift() {
//    std::cout << "horizontalShift " <<horizontalShift<<"\n";
    setHorizontalShift(0.f);
//    foo = true;
}

//<#render#>
void ScrollingNoteViewer::renderOpenGL()
{
//    if (renderingStartCounter>0)
//    {
//        renderingStartCounter--;
//        return;
//    }
//    std::cout << "render" << "\n";
    const ScopedLock myScopedLock (glRenderLock);
    if (!processor->appIsActive)
        return;
    ++frameCounter;
    jassert (OpenGLHelpers::isContextActive());
    
    desktopScale = (float) openGLContext.getRenderingScale();
    OpenGLHelpers::clear (Colour::greyLevel (0.1f));
    
    if (glBufferUpdateCountdown > 0)
        glBufferUpdateCountdown--;
    if  (sequenceChanged && glBufferUpdateCountdown == 0)
    {
        glBufferUpdateCountdown = 2; //Number of renders that must pass before we are allowed in here again
        resized();
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
        sequenceChanged = false;
    }
    if (processor->resetViewer)
    {
        processor->resetViewer = false;
        setHorizontalShift(0);
        repaint();
    }
    if (numIndices==0)
        return;
    
    glEnable (GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
    shader->use();
    static double prevTime;
    double timeShiftInPixels = -processor->getTimeInTicks()*pixelsPerTick;
//    if (prevTime != time)
//    {
//        std::cout
//        << " time " << time
//        << " horizontalShift " << horizontalShift
//        << " timeShiftInPixels " << timeShiftInPixels
//        << " toViewmatrix " << timeShiftInPixels+(sequenceStartPixel+horizontalShift)/horizontalScale
//        <<"\n";
//    }
    prevTime = processor->getTimeInTicks();
    glUniformMatrix4fv(Uniforms::viewMatrixHandle, 1, false, getViewMatrix(timeShiftInPixels+(sequenceStartPixel+horizontalShift)/horizontalScale).mat);
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
    
    std::vector<NoteWithOffTime> *sequence = processor->sequenceObject.getSequence();

    //Get steps (that turned off or on) out of queue
    Array<int> stepsThatChanged;
    int num = processor->noteOnOffFifo.getNumReady();
    if (num>0)
    {
        int start1, size1, start2, size2;
        processor->noteOnOffFifo.prepareToRead (num, start1, size1, start2, size2);
        
        for (int i = 0; i < size1; ++i)
            stepsThatChanged.add(processor->noteOnOffFifoBuffer[start1 + i]);
        for (int i = 0; i < size2; ++i)
            stepsThatChanged.add(processor->noteOnOffFifoBuffer[start2 + i]);
        processor->noteOnOffFifo.finishedRead (size1 + size2);
        for (int j=0;j<stepsThatChanged.size();j++) //Turn off or on notes
        {
//            String note = MidiMessage::getMidiNoteName (stepsThatChanged[j], true, true, 3);
//            std::cout <<"step " << stepsThatChanged[j] << " " << note << "\n";
            if (stepsThatChanged[j]<0) //If was an off
            {
                const int step = -(stepsThatChanged[j]+1);
//                std::cout << "DeHighlight step " << step << "\n";
                if (processor->sequenceObject.isPrimaryTrack(sequence->at(step).track))
                {
                    setRectangleColour(sequence->at(step).rectHead, colourInactiveNoteHead);//Head
//                    setRectangleColour(sequence->at(step).rectBar,  colourPrimaryNoteBar);//Bar
                }
            }
            else if (stepsThatChanged[j]>0) //It was an on
            {
//                std::cout << "Highlight step " << (stepsThatChanged[j]-1) << "\n";
                setRectangleColour(sequence->at(stepsThatChanged[j]-1).rectHead,   colourNoteOn); //Head
            }
        }
    }
    if (!processor->playing() || processor->waitingForFirstNote)
    {
        int step = processor->lastPlayedSeqStep+1;
        if (step<processor->sequenceObject.theSequence.size())
        {
            const float timeStamp = sequence->at(step).getTimeStamp();
            const float width = 4.0f;// (sequence->at(step+1).getTimeStamp()-timeStamp)*pixelsPerTick;
    //      const float height = 13.0f * sequence->at(step).getFloatVelocity();
    //      setRectanglePos(nextNoteRect, timeStamp*pixelsPerTick, 2.0f+(13.0-height), width, height);
            setRectanglePos(nextNoteRect, timeStamp*pixelsPerTick, 0.0f, width, 15.f);
        }
    }
    else
        setRectanglePos(nextNoteRect, 0.0f, 2.f, 0.0f, 13.f); //Make invisible with zero width
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
//<#makeKeyboard#>
void ScrollingNoteViewer::makeKeyboard()
{
    ticksPerQuarter = processor->sequenceObject.getPPQ();
    timeSigChanges = processor->sequenceObject.getTimeSigInfo();
    timeSigChanges[0].getTimeSignatureInfo(numerator, denominator);
    pixelsPerTick = initialWidth/(ticksPerQuarter * numerator * initialMeasuresAcrossWindow);
    leadTimeInTicks = ((getWidth()-wKbd+leftMargin)/pixelsPerTick)*leadTimeProportionOfWidth;
    processor->setLeadTimeInTicks(leadTimeInTicks);
    sequenceStartPixel = leadTimeInTicks*pixelsPerTick;
    processor->sequenceObject.getNotesUsed(minNote,maxNote);
    nKeys = maxNote-minNote+1;
    int height = getHeight();
    if (height==0)
        height = 300;
    keysImage = Image(Image::RGB, leftMargin+wKbd, height, true);
    Graphics keysGr (keysImage);
    
    const Colour textColour (findColour (textLabelColourId));
    trackVerticalSize = ((float)getHeight()-getTopMargin())/nKeys;
    if (trackVerticalSize<1)
        trackVerticalSize = 1;
    
    //left margin
    keysGr.setColour(Colours::black);
    keysGr.fillRect(0,0,roundToInt(leftMargin),roundToInt(getHeight()));
    keysGr.setColour(Colours::grey);
    //topMargin
    keysGr.fillRect(0, 0, roundToInt(wKbd+leftMargin), getTopMargin());
    
    for (int note = maxNote; note >= minNote; note--) //Draw keys
    {
        const float noteY = (maxNote-note) * trackVerticalSize;
        noteYs[note]= noteY;
        const String text (getNoteText (note));
        const float fontHeight = jmin (12.0f, trackVerticalSize * 0.9f);
        if (processor->sequenceObject.isBlackNote(note))
        {
            keysGr.setColour (Colour(Colours::black));
            keysGr.fillRect((float)leftMargin, noteY+getTopMargin(), wKbd, trackVerticalSize);
        }
        else
        {
            keysGr.setColour (Colour(Colours::white));
            keysGr.fillRect((float)leftMargin, noteY+getTopMargin(), wKbd, trackVerticalSize);
            keysGr.setColour (Colour(Colours::black));
            keysGr.drawLine((float)leftMargin, trackVerticalSize+noteY+getTopMargin(),
                                    leftMargin+wKbd, trackVerticalSize+noteY+getTopMargin(),1.5f);
        }
        if (trackVerticalSize>6)
        {
            keysGr.setFont (Font (fontHeight).withHorizontalScale (0.95f));
            if (processor->sequenceObject.isBlackNote(note))
                keysGr.setColour(Colours::white);
            else
                keysGr.setColour(Colours::black);
            keysGr.drawText (text,3, roundToInt(noteY+2.f+getTopMargin()), (int)wKbd-4,(int)trackVerticalSize-4,
                             Justification::centredLeft, false);
        }
    }
    keysGr.setColour(Colours::grey);
    keysGr.fillRect(roundToInt(wKbd+leftMargin-2), 0, 2, roundToInt(getHeight()));
}

//makeNoteBars (highlighted as of time of sequenceReadHead)
void ScrollingNoteViewer::makeNoteBars()
{
//    std::cout << "MNB editable " << processor->getNotesEditable() << "\n";
    if (processor->initialWindowHeight<topMargin)
        return;
    float initialHeight = processor->initialWindowHeight;
    if (getHeight()<0.0000001f)
        return;
    const float rescaleHeight = ((float)initialHeight)/getHeight();
    trackVerticalSize = ((float)initialHeight-topMargin)/nKeys;
    vertices.clear();
    indices.clear();
    std::vector<NoteWithOffTime> *sequence = processor->sequenceObject.getSequence();
    processor->sequenceObject.getNotesUsed(minNote,maxNote);
    nKeys = maxNote-minNote+1;
    const float h = trackVerticalSize*noteBarWidthRatio; //Note bar height
    const float trackGutter = (trackVerticalSize-h)/2.f;
    const int seqSize = static_cast<int>(sequence->size());
    if (seqSize==0)
        return;
    int seqDurationInTicks =  processor->sequenceObject.getSeqDurationInTicks();
    const int sequenceWidthPixels = seqDurationInTicks+sequenceStartPixel;
    
    const float noteBarVerticalSize = trackVerticalSize*noteBarWidthRatio;
    
    //Top margin
    addRectangle(-sequenceWidthPixels, 0.f, sequenceWidthPixels*30, topMargin, Colour(Colours::grey));
//    addRectangle(-sequenceWidthPixels, 0.f, sequenceWidthPixels*30, 2.0f, Colour(0xFFC0C0C0));
    
    //Black & white note track highlighting
    for (int note = minNote;note<=maxNote;note++)
    {
        if (processor->sequenceObject.isBlackNote(note))
            addRectangle(-sequenceWidthPixels, noteYs[note]*rescaleHeight+topMargin, sequenceWidthPixels*30, trackVerticalSize, Colours::black);
        else
            addRectangle(-sequenceWidthPixels, noteYs[note]*rescaleHeight+topMargin, sequenceWidthPixels*30, trackVerticalSize, Colour(0xFF404040));
    }

    //Beat & measure lines
    for (int beat=0; beat<processor->sequenceObject.beatTimes.size(); beat++)
        addRectangle(processor->sequenceObject.beatTimes[beat]*pixelsPerTick-1.0,topMargin,0.8,initialHeight-topMargin, Colour(0xFFB0B0B0).darker());
    
    for (int measure=0; measure<processor->sequenceObject.measureTimes.size(); measure++)
        addRectangle(processor->sequenceObject.measureTimes[measure]*pixelsPerTick-1.0,topMargin,1.0,initialHeight-topMargin, Colour(0xFFE0E0E0).darker());
    
    //Last line
    addRectangle(seqDurationInTicks*pixelsPerTick-1.0f,0.f,2.0f,initialHeight, Colour(0xFFC0C0C0));
    
    //Velocity graph
    const double graphHeight = 300.0-getTopMargin()-toolbarHeight; //300 is the original window height set in MainComponent.cpp
    double prevY = graphHeight * sequence->at(0).highestVelocityInChain/127.0;
    double prevX = -1;
    for (int index = 0;index<static_cast<int>(sequence->size());index++)
    {
        const NoteWithOffTime msg = sequence->at(index);
        const double startPixel = msg.getTimeStamp()*pixelsPerTick;
        double x = startPixel;
        if (msg.triggeredBy==-1)
        {
            int velocityOfTargetNote = msg.getVelocity();
            
            const double scaledVelocity = graphHeight * velocityOfTargetNote/127.0;
            const double thisY = scaledVelocity;
            if (prevX != -1)
            {
//                if (30<index&&index<50)
//                    std::cout << index<<" prevY graphHeight "<<prevY <<" "<<graphHeight<<" "<<topMargin<<"\n";
                addLine (prevX, prevY, x, thisY, 1.0f, Colour(0xFFF0F0FF));
                prevY = thisY;
            }
            prevX = x;
        }
    }
    
    const double readHead = processor->getSequenceReadHead();
//    std::cout
//    << " In makeNoteBars  sequenceReadHead = " << readHead
//    << " editable = " << editable
//    << "\n";
    //Note bars but not target notes
    for (int index = 0;index<static_cast<int>(sequence->size());index++)
    {
        const NoteWithOffTime msg = sequence->at(index);
        if (msg.chainTrigger==index)
            continue; //If it's a target note
        const double startPixel = msg.getTimeStamp()*pixelsPerTick;
        double endPixel = msg.offTime*pixelsPerTick;
        const int noteNumber = msg.getNoteNumber();
        const float y = noteYs[noteNumber]*rescaleHeight + trackGutter + topMargin;
        float x = startPixel;
        float w = endPixel - startPixel;
        const float headToBarHeightRatio = 1.0 + 0.4 * (std::max(nKeys-10.0,0.0))/88.0;
        const float headHeight =  h * headToBarHeightRatio;
        float headWidth = 6.0f;
        const float vel = msg.getFloatVelocity(); //Cyan
        Colour vBasedPrimaryNoteBar = Colour::fromFloatRGBA(0.3f + 0.7f*vel,
                                                            0.2f + 0.6f*vel,
                                                            0.3f + 0.7f*vel,
                                                            1.0f);
        sequence->at(index).head = Rectangle<float>(x, y,headWidth,headHeight);
        if (sequence->at(index).chordTopStep==-1 && (processor->getNotesEditable()||sequence->at(index).getTimeStamp()>=readHead))
            sequence->at(index).rectBar = addNote(false,x, y, w, noteBarVerticalSize, headWidth, headHeight,
                                                  colourInactiveNoteHead, vBasedPrimaryNoteBar);
        else
            sequence->at(index).rectBar = addNote(true,x, y, w, noteBarVerticalSize, headWidth, headHeight,
                                                      vBasedPrimaryNoteBar, vBasedPrimaryNoteBar);
        sequence->at(index).rectHead = sequence->at(index).rectBar + 1;
    }
    //Target Notes
//    std::cout<< "MNB editable, readHead "<< processor->getNotesEditable() <<" "<< readHead << "\n";
    for (int index = 0;index<static_cast<int>(sequence->size());index++)
    {
        const NoteWithOffTime msg = sequence->at(index);
        if (msg.chainTrigger!=index) //If it's not an active (target) note
            continue;
        const double startPixel = msg.getTimeStamp()*pixelsPerTick;
        double endPixel = msg.offTime*pixelsPerTick;
        const int noteNumber = msg.getNoteNumber();
        const double thisEndTime = msg.offTime;
        const float y = noteYs[noteNumber]*rescaleHeight + trackGutter + topMargin;
        int indexOfNextSameNote = -1;
        long size = sequence->size();
        for (int nxtNoteIndex=index+1;nxtNoteIndex<size-2;nxtNoteIndex++)
        {
            if (sequence->at(nxtNoteIndex).getTimeStamp() > thisEndTime)
                break;
            if (sequence->at(nxtNoteIndex).getNoteNumber()==noteNumber && sequence->at(nxtNoteIndex).chainTrigger==nxtNoteIndex)
            {
                indexOfNextSameNote = nxtNoteIndex;
                break;
            }
        }
        const float x = startPixel;
        const float w = endPixel - startPixel;
        double startPixelOfNextSameNote;
        if (indexOfNextSameNote == -1)
            startPixelOfNextSameNote = DBL_MAX;
        else
            startPixelOfNextSameNote = sequence->at(indexOfNextSameNote).getTimeStamp()*pixelsPerTick;
        const float headToBarHeightRatio = 1.0 + 0.4 * (std::max(nKeys-10.0,0.0))/88.0;
        const float headHeight =  h * headToBarHeightRatio;
        float headWidth = 6.0f;
        const double minSpacing = 1.0;
        if (headWidth > startPixelOfNextSameNote-x)
            headWidth = std::max(minSpacing,startPixelOfNextSameNote-x-minSpacing);
        const float vel = msg.getFloatVelocity();
        const Colour vBasedNoteBar = Colour::fromFloatRGBA(0.3f + 0.7f*vel, 0.2f + 0.6f*vel, 0.3f + 0.7f*vel, 1.0f);
        sequence->at(index).head = Rectangle<float>(x, y,headWidth,headHeight);
        if (processor->getNotesEditable() || sequence->at(index).getTimeStamp()>=readHead)
            sequence->at(index).rectBar = addNote(true,x, y, w, noteBarVerticalSize, headWidth, headHeight,
                                                    colourActiveNoteHead, vBasedNoteBar);
        else
            sequence->at(index).rectBar = addNote(true,x, y, w, noteBarVerticalSize, headWidth, headHeight,
                                                  colourInactiveNoteHead, vBasedNoteBar);
        sequence->at(index).rectHead = sequence->at(index).rectBar + 1;
    }
    
    //Sustain bars
    if (processor->sequenceObject.sustainPedalChanges.size()>0)
    {
        Array<Rectangle<double>> sustainBars;
        double sustainStartTick = 0;
        //First make an array of Rectangles each with just a bar's start tick and width in ticks
        bool pedalDown = false;
        for (int i=0;i<processor->sequenceObject.sustainPedalChanges.size();i++)
        {
            if (!pedalDown && processor->sequenceObject.sustainPedalChanges.at(i).getControllerValue()>=64)
            {
                sustainStartTick = processor->sequenceObject.sustainPedalChanges.at(i).getTimeStamp();
                pedalDown = true;
            }
            else if (pedalDown && sustainStartTick!=-1 && processor->sequenceObject.sustainPedalChanges.at(i).getControllerValue()<64)
            {
                sustainBars.add(Rectangle<double>(sustainStartTick,0,processor->sequenceObject.sustainPedalChanges.at(i).getTimeStamp()-sustainStartTick,0));
                pedalDown = false;
            }
        }
        //Then scan the sequence looking for the highest note in the range of each bar and draw the bar just higher than hignest note
        bool inSustainBar = false;
        int highestNote = -1;
        int sustainBarNum = 0;
        for (int step=0; step<static_cast<int>(sequence->size()); step++)
        {
            const NoteWithOffTime msg = sequence->at(step);
            const double barLeft = sustainBars[sustainBarNum].getX();
            const double barRight = sustainBars[sustainBarNum].getRight();
            const double msgTimeStamp = msg.getTimeStamp();
            if (!inSustainBar) //Msg is after the start of this bar
            {
                if (msgTimeStamp > barRight)
                {
                    const double barLeft = sustainBars[sustainBarNum].getX();
                    const double barRight = sustainBars[sustainBarNum].getRight();
                    const float y = noteYs[highestNote]*rescaleHeight + topMargin - 1.3*trackVerticalSize;
                    addRectangle(barLeft*pixelsPerTick,y,(barRight-barLeft)*pixelsPerTick,2.0, Colour(Colours::orange).brighter());
                    sustainBarNum++;
                }
                else if (msgTimeStamp>=barLeft)
                {
                    inSustainBar = true;
                }
            }
            else if (inSustainBar && msgTimeStamp >= barRight)
            {
                inSustainBar = false;
                const float y = noteYs[highestNote]*rescaleHeight + topMargin - 2.3*trackVerticalSize;
                addRectangle(barLeft*pixelsPerTick,y,(barRight-barLeft)*pixelsPerTick,2.0, Colour(Colours::orange).brighter());
                sustainBarNum++;
                if (msgTimeStamp > barRight)
                    highestNote = msg.getNoteNumber();
                else
                    highestNote = -1;
            }
            if (inSustainBar)
            {
                if (msg.getNoteNumber()>highestNote)
                    highestNote = msg.getNoteNumber();
            }
        }
    } //End sustain bars
    
    //soft bars
    if (processor->sequenceObject.softPedalChanges.size()>0)
    {
        Array<Rectangle<double>> softBars;
        double softStartTick = 0;
        //First make an array of Rectangles each with just a bar's start tick and width in ticks
        bool pedalDown; // = processor->sequenceObject.softPedalChanges.at(0).getControllerValue()>=64;
        for (int i=0;i<processor->sequenceObject.softPedalChanges.size();i++)
        {
            if (!pedalDown && processor->sequenceObject.softPedalChanges.at(i).getControllerValue()>=64)
            {
                softStartTick = processor->sequenceObject.softPedalChanges.at(i).getTimeStamp();
                pedalDown = true;
            }
            else if (pedalDown && softStartTick!=-1 && processor->sequenceObject.softPedalChanges.at(i).getControllerValue()<64)
            {
                softBars.add(Rectangle<double>(softStartTick,0,processor->sequenceObject.softPedalChanges.at(i).getTimeStamp()-softStartTick,0));
                pedalDown = false;
            }
        }
        //Then scan the sequence looking for the highest note in the range of each bar and draw the bar just higher than hignest note
        int countSofts = 0;
        bool insoftBar = false;
        int highestNote = -1;
        int softBarNum = 0;
        for (int step=0; step<static_cast<int>(sequence->size()); step++)
        {
            const NoteWithOffTime msg = sequence->at(step);
            const double barLeft = softBars[softBarNum].getX();
            const double barRight = softBars[softBarNum].getRight();
            const double msgTimeStamp = msg.getTimeStamp();
            if (!insoftBar) //Msg is after the start of this bar
            {
                if (msgTimeStamp > barRight)
                {
                    const double barLeft = softBars[softBarNum].getX();
                    const double barRight = softBars[softBarNum].getRight();
                    const float y = noteYs[highestNote]*rescaleHeight + topMargin - 2.3*trackVerticalSize;
                    countSofts++;
                    addRectangle(barLeft*pixelsPerTick,y,(barRight-barLeft)*pixelsPerTick,2.0, Colour(Colours::lightblue).brighter());
                    softBarNum++;
                }
                else if (msgTimeStamp>=barLeft)
                {
                    insoftBar = true;
                }
            }
            else if (insoftBar && msgTimeStamp >= barRight)
            {
                insoftBar = false;
//                const float y = noteYs[highestNote]*rescaleHeight + topMargin - 2.3*trackVerticalSize;
                const float y = 3.0;
                countSofts++;
                addRectangle(barLeft*pixelsPerTick,y,(barRight-barLeft)*pixelsPerTick,2.0, Colour(Colours::lightblue).brighter());
                
                softBarNum++;
                if (msgTimeStamp > barRight)
                    highestNote = msg.getNoteNumber();
                else
                    highestNote = -1;
            }
            if (insoftBar)
            {
                if (msg.getNoteNumber()>highestNote)
                    highestNote = msg.getNoteNumber();
            }
        }
    } //End soft bars
    
    //Bookmarks
    //        std::cout << processor->sequenceObject.bookmarkTimes.size() << "\n";
    for (int i=0;i<processor->sequenceObject.bookmarkTimes.size();i++)
    {
        const double x = processor->sequenceObject.bookmarkTimes[i]*pixelsPerTick;
        addRectangle(x-1.95, 0.0f,     4, (topMargin), juce::Colour(Colours::red));
    }
    
    //Position of next note to play
    const double x = processor->sequenceObject.theSequence[processor->lastPlayedSeqStep+1].getTimeStamp()*pixelsPerTick;
    nextNoteRect = addRectangle(x-1.95, 0,     4, (topMargin),Colours::green);
}

void ScrollingNoteViewer::updatePlayedNotes()
{

}

//<#paint#>
void ScrollingNoteViewer::paint (Graphics& g)
{
    //Start of most recently played note
    if (processor->isPlaying && !processor->waitingForFirstNote)
    {
        const double hLinePos = 2.8 * horizontalScale + sequenceStartPixel + processor->leadLag * pixelsPerTick * horizontalScale;
        g.setColour (colourNoteOn);
        g.fillRect(Rectangle<float>(hLinePos,topMargin*verticalScale, 1.1, getHeight()-topMargin*verticalScale));
    }
    if (processor->isPlaying)
    {
        //ZTL
        if (processor->waitingForFirstNote)
            g.setColour (Colour(Colours::orange));
        else
            g.setColour (Colour(Colours::green).brighter());
    }
    else
        g.setColour (Colour(30,30,255).brighter()); //Blue
    g.fillRect(Rectangle<float>(sequenceStartPixel-1.f,topMargin*verticalScale, 2.0, getHeight()-topMargin*verticalScale));
    //Handle at top of line
    g.setColour (Colour((uint8)190,(uint8)220,(uint8)0xff,(uint8)127));
    g.fillRect(Rectangle<float>(sequenceStartPixel-3.f,0.0, 6.0, (topMargin)*verticalScale));
    
    const int meas = processor->getMeasure(horizontalShift);
    Font f = Font (11.0*verticalScale);
    f.setStyleFlags(Font::FontStyleFlags::bold);
    g.setFont(f);
    g.setColour (Colours::white);
    g.drawText(String(meas), sequenceStartPixel+6, 5.5*verticalScale, 50, 9*verticalScale, juce::Justification::centredLeft);
    
    if (!processor->isPlaying)
    {
        g.setColour (Colours::whitesmoke);
//        if (processor->undoMgr->inUndo)
//        {
//            selectedNotes.clear();
//            displayedSelection.clear();
//        }
        if (processor->undoMgr->inRedo)
        {
            std::cout << "paint inUndo   " << "\n";
            displayedSelection = processor->sequenceObject.undoneOrRedoneSteps;
            setSelectedNotes(processor->sequenceObject.undoneOrRedoneSteps);
            processor->undoMgr->inRedo = false;
//            processor->inUndoRedo = false;
        }
        for (int i=0;i<displayedSelection.size();i++)
        {
            const Rectangle<float> scaledHead = processor->sequenceObject.theSequence[displayedSelection[i]].head;
            const Rectangle<float> head = Rectangle<float>(
                 scaledHead.getX()*horizontalScale+sequenceStartPixel+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                 scaledHead.getY()*verticalScale,
                 scaledHead.getWidth()*horizontalScale,
                 scaledHead.getHeight()*verticalScale).expanded(1.5, 1.5);
            g.drawRect(head, 1.2);
        }
        
        g.setColour (Colours::yellow);
        if (selecting)
            g.drawRect(selectionRect,2);
    }
}

void ScrollingNoteViewer::changeListenerCallback (ChangeBroadcaster*
                                                  broadcaster)
{
    if ((MIDIProcessor*)broadcaster == processor) //Triggered at the end of rewind() in MIDIProcessor
    {
//        std::cout << " ViewerCallback:  " <<  processor->changeMessageType <<"\n";
        if (processor->changeMessageType == CHANGE_MESSAGE_REWIND)
        {
            if (processor->sequenceObject.fileToLoad != prevFileLoaded || processor->getTimeInTicks()==0)
            {
                makeKeyboard ();
                makeNoteBars ();
                clearSelectedNotes();
                sequenceChanged = true;
            }
            else
            {
                makeKeyboard ();
                makeNoteBars ();
                sequenceChanged = true;
            }
            setHorizontalShift(0);
            prevFileLoaded = processor->sequenceObject.fileToLoad;
        }
        else if (processor->changeMessageType == CHANGE_MESSAGE_TWEEN)
        {
            if (isTimerRunning(TIMER_TWEEN))
                stopTimer(TIMER_TWEEN);
            const double twTo = processor->tweenTo;
            const double prevTimeInTicks = processor->getTimeInTicks();
            
            timeInTicksTweens.clear();
            horizontalShiftTweens.clear();
            transitionTime = processor->transitionTime;
            if (transitionTime<=10)
                nSteps = 1;
            else if (transitionTime<=300)
                nSteps = 4;
            else
                nSteps = 6;
            for (int i=0;i<nSteps;i++)
            {
                timeInTicksTweens.add(prevTimeInTicks+(twTo-prevTimeInTicks)*((double)(i+1)/(double)nSteps));
                horizontalShiftTweens.add(0);
                //                std::cout << i << " timeInTicksStep horizontalShift step " <<  timeInTicksTweens[i]<< " " <<timeInTicksTweens[i] <<"\n";
            }
            animationStep = 0;
            startTimer (TIMER_TWEEN, transitionTime/nSteps); // timeInMS / nSteps
        }
        else if (processor->changeMessageType == CHANGE_MESSAGE_STOP_PLAYING)
        {
            processor->setNotesEditable(true);
        }
        else if (processor->changeMessageType == CHANGE_MESSAGE_START_PLAYING)
        {
//            static int count2;
//            count2++;
//            std::cout << "CHANGE_MESSAGE_START_PLAYING " << count2<< " "<<horizontalShift<< "\n";
//            nSteps = -1;
            processor->setNotesEditable(false);
            setHorizontalShift(0.0);
        }
        else if (processor->changeMessageType == CHANGE_MESSAGE_MEASURE_CHANGED)
        {
//            std::cout << "CHANGE_MESSAGE_MEASURE_CHANGED " << "\n";
//            repaint();
        }
        else if (processor->changeMessageType == CHANGE_MESSAGE_BEAT_CHANGED)
        {
//            std::cout << "CHANGE_MESSAGE_BEAT_CHANGED " <<  "\n";
//            repaint();
        }
        else if (processor->changeMessageType == CHANGE_MESSAGE_NOTE_PLAYED)
        {
            //            std::cout << "CHANGE_MESSAGE_BEAT_CHANGED " <<  "\n";
//            repaint();
        }
        else if (processor->changeMessageType == CHANGE_MESSAGE_UNDO)
        {
            processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getTimeInTicks());
            makeKeyboard ();
            makeNoteBars ();
            repaint();
        }
        if (grabbedInitialWindowSize)
            processor->changeMessageType = CHANGE_MESSAGE_NONE;
    }
    else if ((Sequence*)broadcaster == &(processor->sequenceObject))
    {
        double hScale = processor->sequenceObject.sequenceProps.getDoubleValue("horizontalScale");
        horizontalScale = fmaxf(0.01,hScale);
    }
    repaint();
}

void ScrollingNoteViewer::timerCallback (int timerID)
{
    if (timerID == TIMER_TWEEN)
    {
        processor->setTimeInTicks(timeInTicksTweens[animationStep]);
        setHorizontalShift(horizontalShiftTweens[animationStep]);
    //    std::cout << "setHorizontalShift, setTimeInTicks " << horizontalShift << " " << processor->getTimeInTicks() << "\n";
        repaint();
        processor->catchUp();
        animationStep++;
        if (animationStep==nSteps)
        {
            nSteps = -1;
            stopTimer(TIMER_TWEEN);
        }
    }
    else if (timerID == TIMER_TOGGLE_TARGET_NOTE)
    {
        stopTimer(TIMER_TOGGLE_TARGET_NOTE);
        processor->undoMgr->beginNewTransaction();
        MIDIProcessor::ActionSetNoteActivity* action;
        bool setNotesActive = !processor->getNoteActivity(hoverStep);
        if (selectedNotes.size()==0)
        {
            Array<int> oneStep;
            oneStep.add(hoverStep);
            setSelectedNotes(oneStep);
            action = new MIDIProcessor::ActionSetNoteActivity(*processor, setNotesActive, oneStep);
        }
        else
            action = new MIDIProcessor::ActionSetNoteActivity(*processor, setNotesActive, selectedNotes);
        processor->undoMgr->perform(action);
    }
    else if (timerID == TIMER_MOUSE_HOLD)
    {
        stopTimer(TIMER_MOUSE_HOLD);
//        std::cout << "Here - mouse hold " << selectionAnchor.getY() << "\n";
        selecting = true;
    }
    else if (timerID == TIMER_MOUSE_DRAG)
    {
        stopTimer(TIMER_MOUSE_DRAG);
        static double prevY;
        double xx = Desktop::getInstance().getMousePosition().getX();
        double yy = Desktop::getInstance().getMousePosition().getY();
        double y = curDragPosition.getY();
        double hh =
        Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getHeight();
        if (yy<=0)
        {
            Desktop::getInstance().setMousePosition(Point<int> (xx,5));
            mouseBeforeDrag.setY(mouseBeforeDrag.getY()+5);
        }
        else if (prevY<=y && yy > Desktop::getInstance().getDisplays().getMainDisplay().userArea.getHeight())
        {
            Desktop::getInstance().setMousePosition(Point<int> (xx,hh-5));
            mouseBeforeDrag.setY(mouseBeforeDrag.getY()-5);
        }
        prevY = y;
        
        if (zoomDragStarting)
        {
            zoomOrScrollDragging = true;
            preDragScale = horizontalScale;
            mouseBeforeDrag = Point<float>(curDragPosition.getX(),
                                           curDragPosition.getY());
            preDragHorizShift = horizontalShift;
            zoomDragStarting = false;
        }
        if (zoomOrScrollDragging)
        {
            float scaleChange = mouseBeforeDrag.getY()-curDragPosition.getY();
            //        std::cout
            //        << " y " << y
            //        << " yy " << yy
            //        << " mbd " << mouseBeforeDrag.getY()
            //        << " scaleChange " << scaleChange
            //        << "\n";
            float absScaleChange = fabsf(scaleChange)/300.f;
            const float minScale = 0.01f;
            float rawScale;
            double absScaleChangeWithCorrectSign;
            if (mouseBeforeDrag.getY()>curDragPosition.getY())
            {
                rawScale = preDragScale+absScaleChange;
                absScaleChangeWithCorrectSign = absScaleChange;
            }
            else
            {
                rawScale = preDragScale-absScaleChange;
                absScaleChangeWithCorrectSign = -absScaleChange;
            }
            //        std::cout << "rawScale " << event.position.getY() <<" " << rawScale << "\n"
            if (rawScale>minScale)
            {
                horizontalScale = fmaxf(minScale,rawScale);
                const double seqLengthInPixels = processor->sequenceObject.seqDurationInTicks*pixelsPerTick;
                //            double proportionOfSequenceLeftOfMouse = preDragXinTicks/processor->sequenceObject.seqDurationInTicks;
                
                double proportionOfSequenceLeftOfMouse = (preDragXinTicks-processor->getTimeInTicks())/processor->sequenceObject.seqDurationInTicks;
                double shiftLeft = preDragHorizShift-seqLengthInPixels *
                proportionOfSequenceLeftOfMouse*(absScaleChangeWithCorrectSign);
                
                float shift = (curDragPosition.getX() - mouseBeforeDrag.getX());
                
                setHorizontalShift(shiftLeft+shift);
                
                processor->sequenceObject.sequenceProps.setValue("horizontalScale",
                                                                 var(horizontalScale));
                //        std::cout << "scale " <<minScale<<" "<<preDragScale<<" "<<absScaleChange << " " << horizontalScale << "\n";
                //            prevMouseDragX = event.position.getX();
                repaint();
            }
        }
        else
        {
            if (selecting)
            {
                if (!ModifierKeys::getCurrentModifiers().isCommandDown())
                    clearSelectedNotes();
                if (selectionAnchor.getX() > curDragPosition.getX())
                {
                    if (selectionAnchor.getY() > curDragPosition.getY())
                    {
                        selectionRect = Rectangle<int>::leftTopRightBottom(curDragPosition.getX(), curDragPosition.getY(),
                                                                           selectionAnchor.getX(), selectionAnchor.getY());
                    }
                    else
                    {
                        selectionRect = Rectangle<int>::leftTopRightBottom(curDragPosition.getX(), selectionAnchor.getY(),
                                                                           selectionAnchor.getX(), curDragPosition.getY());
                    }
                }
                else
                {
                    if (selectionAnchor.getY() > curDragPosition.getY())
                    {
                        selectionRect = Rectangle<int>::leftTopRightBottom(selectionAnchor.getX(), curDragPosition.getY(),
                                                                           curDragPosition.getX(), selectionAnchor.getY());
                    }
                    else
                    {
                        selectionRect = Rectangle<int>::leftTopRightBottom(selectionAnchor.getX(), selectionAnchor.getY(),
                                                                           curDragPosition.getX(),curDragPosition.getY());
                    }
                }
                const int xInTicksLeft = ((selectionRect.getX() - (horizontalShift+sequenceStartPixel))/pixelsPerTick)/
                horizontalScale + processor->getTimeInTicks();
                const int xInTicksRight = ((selectionRect.getRight() - (horizontalShift+sequenceStartPixel))/pixelsPerTick)/
                horizontalScale + processor->getTimeInTicks();
                Rectangle<float> selRect = Rectangle<float>::leftTopRightBottom((float)selectionRect.getX(),
                                                                                (float)selectionRect.getY(),
                                                                                (float)selectionRect.getRight(),
                                                                                (float)selectionRect.getBottom()
                                                                                );
                newlySelectedNotes.clear();
                for (int step=0;step<processor->sequenceObject.theSequence.size();step++)
                {
                    if (processor->sequenceObject.theSequence[step].chordTopStep==-1 &&
                        processor->sequenceObject.theSequence[step].getTimeStamp() >= xInTicksLeft
                        && processor->sequenceObject.theSequence[step].getTimeStamp() <= xInTicksRight)
                    {
                        const Rectangle<float> scaledHead = processor->sequenceObject.theSequence[step].head;
                        const Rectangle<float> head = Rectangle<float>(
                                                                 scaledHead.getX()*horizontalScale+sequenceStartPixel+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                                                                 scaledHead.getY()*verticalScale,
                                                                 scaledHead.getWidth()*horizontalScale,
                                                                 scaledHead.getHeight()*verticalScale);
                        
                        if (head.intersects(selRect))
                        {
//                            newlySelectedNotes.add(head);
                             newlySelectedNotes.add(step);
                        }
                        
                    }
                }
                displayedSelection.clear();
                for (int i=0;i<newlySelectedNotes.size();i++)
                {
                    if (!selectedNotes.contains(newlySelectedNotes[i]))
                        displayedSelection.add(newlySelectedNotes[i]);
                }
                for (int i=0;i<selectedNotes.size();i++)
                {
                    if (!newlySelectedNotes.contains(selectedNotes[i]))
                        displayedSelection.add(selectedNotes[i]);
                }
//                std::cout
//                <<  " selected "<<selectedNotes.size()
//                <<  " newlySelected "<<newlySelectedNotes.size()
//                <<  " displayedSelection "<<displayedSelection.size()
//                <<  "\n";
                repaint();
            }
        }
    }
}

void ScrollingNoteViewer::resized()
{
    if (!grabbedInitialWindowSize)
    {
        initialWidth = getWidth();
//        initialHeight = getHeight ();
        processor->initialWindowHeight = getHeight ();
        grabbedInitialWindowSize = true;
    }
    initialHeight = processor->initialWindowHeight;
//    std::cout << "resized - trackVerticalSize, verticalScale: before " << trackVerticalSize <<" "<<verticalScale;
    verticalScale = (float)getHeight()/initialHeight;
    makeKeyboard ();
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

