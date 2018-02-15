
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
#include <algorithm>

//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

int ViewStateInfo::initialWidth = 0;
int ViewStateInfo::initialHeight = 0;
int ViewStateInfo::initialPPT = 0; //Initial pixels per tick
int ViewStateInfo::viewWidth = 0;
int ViewStateInfo::viewHeight = 0;
float ViewStateInfo::verticalScale = 1;
float ViewStateInfo::horizontalScale = 1;
float ViewStateInfo::trackVerticalSize = 0;
Array<Vertex> ViewStateInfo::vertices;
Array<int> ViewStateInfo::indices;
bool ViewStateInfo::openGLStarted = false;
bool ViewStateInfo::finishedPaintAfterRewind = false;

GLint ScrollingNoteViewer::Uniforms::viewMatrixHandle;
GLint ScrollingNoteViewer::Uniforms::projectionMatrixHandle;
//==============================================================================
ScrollingNoteViewer::ScrollingNoteViewer(MIDIProcessor *p) :
        processor(p),
        wKbd(24.f),
        maxNote(84),
        minNote(59),
        compressNotes(false),
        octaveNumForMiddleC(3),
        toolbarHeight(30),
        topMargin(15),
        leftMargin(2),
        noteBarWidthRatio(1.f) //As fraction of note track width
{
    processor->initialWindowHeight = 88;
    glBufferUpdateCountdown = 0; //Countdown of number of renders to pass before another buffer update is allowed.
    rebuidingGLBuffer = false;
    rendering = false;
    sequenceChanged = false;
    ignoreWheel = false;
    prevFileLoaded = File();
    processor->addChangeListener(this); //Sent at the end of rewind  ()
    processor->sequenceObject.addChangeListener(this); //Send at the end of saveSequence()
    openGLContext.setMultisamplingEnabled(true);
    OpenGLPixelFormat format;
    format.multisamplingLevel = 4;
    openGLContext.setPixelFormat(format);
    animationStep = 0;
    hoverStep = -1;
    hoverChord = -1;
    draggingTime = false;
    draggingVelocity = false;
    drawingVelocity = false;
    draggingOffTime = false;
    xPositionOfBaseLine = -1;
//    showVelocityIndicator = false;

    setPaintingIsUnclipped(true);
//    x = 0;
//    y = 0;
    position = nullptr;
    sourceColour = nullptr;
    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(true);
    openGLContext.setSwapInterval(2);
    horizontalScale = 1.0f;
    ViewStateInfo::verticalScale = 1.0f;
    initialMeasuresAcrossWindow = 5;
    leadTimeProportionOfWidth = 0.4;
    grabbedInitialWindowSize = false;
    zoomDragStarting = false;
    zoomOrScrollDragging = false;
    selecting = false;
    editingNote = false;
    showingChords = false;
    resetHorizontalShift();
    colourActiveNoteHead = Colour(0xffff00ff);//Cyan
    colourPrimaryNoteBar = Colour(0xffffccff).darker().darker();
    colourInactiveNoteHead = Colour(0xffbbbbbb);
    colourNoteOn = Colour(0xffFFFF55);
    nSteps = -1;
    showingVelocities.setValue(false);
    marqueeAddingNotes = false;
    marqueeRemovingNotes = false;
    markingSelectedNotes = false; //Not in marking mode
    clearingSelectedNotes = false; //If we were in marking mode we would be makring them active
    altKeyPressed = ModifierKeys::getCurrentModifiers().isAltDown();
    editVelocityCursor = getMouseCursorFromZipFile("editVelocityCursor.svg");
    selectionUnMarkerCursor = getMouseCursorFromZipFile("SelectionUnMarkerCursor.svg");
    selectionMarkerCursor = getMouseCursorFromZipFile("SelectionMarkerCursor.svg");
    marqueeAddingCursor = getMouseCursorFromZipFile("MarqueeAddCursor.svg");
    marqueeRemovingCursor = getMouseCursorFromZipFile("MarqueeRemoveCursor.svg");
    startTimer(TIMER_PERIODIC, 200);
}

ScrollingNoteViewer::~ScrollingNoteViewer()
{
    openGLContext.setContinuousRepainting(false);
    openGLContext.detach();
    openGLContext.setRenderer(NULL);
    processor->removeChangeListener(this);
    processor->sequenceObject.removeChangeListener(this);
}

void ScrollingNoteViewer::mouseDown(const MouseEvent &e)
{
    try
    {
        selectionAnchor = Point < int > (e.getPosition().getX(), e.getPosition().getY());
        draggingVelocity = false;
        drawingVelocity = false;
        draggingTime = false;
        draggingOffTime = false;
        //We defer pickup of up the mouse position until drag actually starts because the mouseDown position is slightly
        //different from the first position returned in mouseDrag.
        newlySelectedNotes.clear();
        if (hoveringOver == HOVER_NOTETRACK && (!ModifierKeys::getCurrentModifiers().isCommandDown() &&
                                                !ModifierKeys::getCurrentModifiers().isShiftDown() &&
                                                !ModifierKeys::getCurrentModifiers().isAltDown() &&
                                                !drawingVelocities.getValue()))
            clearSelectedNotes();
        if (!ModifierKeys::getCurrentModifiers().isCommandDown() && hoveringOver != HOVER_NONE)
        {
            if ((hoveringOver == HOVER_NOTEHEAD || hoveringOver == HOVER_NOTEBAR) && !selectedNotes.contains(hoverStep))
            {
//            displayedSelection.clear();
                ;//clearSelectedNotes();
            }
        }
        selectionRect = Rectangle<int>();
        repaint();
        if (processor->isPlaying)
        {
//        setEditable(true);
            processor->play(false, "current");
        }
        if (e.position.getY() < topMargin * ViewStateInfo::verticalScale)
        { //Test mouse position
            zoomDragStarting = true;
        }
        else if (hoveringOver != HOVER_NOTEHEAD && hoveringOver != HOVER_NOTEBAR &&
                   hoveringOver != HOVER_ZEROTIMELINE)
        {
//        selectionAnchor = Point<int>(e.getPosition().getX(),e.getPosition().getY());
            startTimer(TIMER_MOUSE_HOLD, 1);
        }
        else
        {
            editingNote = true;
            noteEditAnchor = Desktop::getMousePosition();
        }
//    std::cout << "mouse down " <<e.getPosition().getX()<<" "
//    << selectionRect.getTopLeft().getX()<<" "<<selectionRect.getTopLeft().getY()<< "\n";
    }
    catch (const std::out_of_range &ex)
    {
        std::cout << " error noteviewer: mouseDown " << "\n";
    }
}

void ScrollingNoteViewer::mouseUp(const MouseEvent &event)
{
//  float trackVerticalSize = ((float)ViewStateInfo::viewHeight-ViewStateInfo::verticalScale*topMargin)/nKeys;
    try
    {
        stopTimer(TIMER_MOUSE_DRAG);
        const int xDist = abs(selectionAnchor.getX() - event.getPosition().getX());
        const int yDist = abs(selectionAnchor.getY() - event.getPosition().getY());
        distanceMouseMovedSinceMouseDown = xDist>yDist ? xDist :yDist;
//        std::cout << "distanceMouseMovedSinceMouseDown " << distanceMouseMovedSinceMouseDown << std::endl;
        std::vector<std::shared_ptr<NoteWithOffTime>> *pSequence = &(processor->sequenceObject.theSequence);
        const double vert = (event.getPosition().getY() - ViewStateInfo::verticalScale * topMargin) /
                            ViewStateInfo::trackVerticalSize;

        const double xInTicks =
                ((event.getPosition().getX() - (horizontalShift + xPositionOfBaseLine)) / pixelsPerTick) /
                horizontalScale + processor->getTimeInTicks();
        const float sequenceScaledX = mouseXinTicks * pixelsPerTick;
        const float scaledY = event.getPosition().getY() / ViewStateInfo::verticalScale;
        int i;
        int ch = -1;
        if (showingChords)
        {
            for (i = 0; i < processor->sequenceObject.chords.size(); i++)
            {
                if (processor->sequenceObject.chords.at(i).chordRect.expanded(2.0, 0.0).contains(sequenceScaledX,
                                                                                                 scaledY))
                {
                    ch = i;
                    break;
                }
            }
        }
        if (selecting)
        {
            selecting = false;
            marqueeRemovingNotes = false;
            marqueeAddingNotes = false;
            editingNote = false;

            if (!event.source.hasMouseMovedSignificantlySincePressed())
            {
                if (hoveringOver == HOVER_NOTEHEAD)
                {
                    if (displayedSelection.contains(hoverStep))
                    {
                        displayedSelection.remove(displayedSelection.indexOf(hoverStep));
                        selectedNotes.remove(selectedNotes.indexOf(hoverStep));
                    } else
                        displayedSelection.add(hoverStep);
                } else
                {
                    clearSelectedNotes();
                }
            }
            setSelectedNotes(displayedSelection);
            repaint();
            return;
        } else if (hoverChord != -1)
        {
            Array<int> chordNotes;
            for (int i = 0; i < processor->sequenceObject.chords.at(hoverChord).notePointers.size(); i++)
                chordNotes.add(processor->sequenceObject.chords.at(hoverChord).notePointers.at(i)->currentStep);
            displayedSelection = chordNotes;
            setSelectedNotes(chordNotes);
            repaint();
        }
        stopTimer(TIMER_MOUSE_HOLD);
        if (draggingTime)
        {
            if (deltaTimeDrag != -1)
            {
                Array<int> steps;
                if (selectedNotes.contains(noteBeingDraggedOn))
                    steps = selectedNotes;
                else
                    steps.add(noteBeingDraggedOn);
                std::vector<std::shared_ptr<NoteWithOffTime>> pointersToSelectedNotes = stashSelectedNotes();
                processor->undoMgr->beginNewTransaction();
                MIDIProcessor::ActionChangeNoteTimes *action;
                action = new MIDIProcessor::ActionChangeNoteTimes(*processor, deltaTimeDrag, pointersToSelectedNotes);
                processor->undoMgr->perform(action);
                processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits,
                                             processor->getZTLTime(horizontalShift));
                restoreSelectedNotes(pointersToSelectedNotes);
            }
            draggingTime = false;
            hoveringOver = HOVER_NONE;
        }
        else if (draggingVelocity)
        {
            //perform undoablecommand changing velocities to velocitiesAfterDragOrDraw
            Array<Sequence::NoteVelocities> newVelocities;
            for (int i = 0; i < notesBeingDraggedOn.size(); i++)
            {
                Sequence::NoteVelocities vel;
                vel.note = pSequence->at(notesBeingDraggedOn[i]);
                vel.velocity = pSequence->at(notesBeingDraggedOn[i])->getVelocity();
                newVelocities.add(vel);
                pSequence->at(notesBeingDraggedOn[i])->velocity = velsStartDrag[i];
            }
            processor->undoMgr->beginNewTransaction();
            MIDIProcessor::ActionChangeVelocities *action;
            action = new MIDIProcessor::ActionChangeVelocities(*processor, newVelocities);
            processor->undoMgr->perform(action);
            processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits,
                                         processor->getZTLTime(horizontalShift));
            draggingVelocity = false;
            hoveringOver = HOVER_NONE;
        }
        else if (draggingOffTime)
        {
            Array<int> steps;
            if (offTimeAfterDrag != -1)
            {
                if (selectedNotes.contains(noteBeingDraggedOn))
                    steps = selectedNotes;
                else
                    steps.add(noteBeingDraggedOn);
            }
            const double delta =
                    offTimeAfterDrag - processor->sequenceObject.theSequence.at(noteBeingDraggedOn)->offTime;
            std::vector<std::shared_ptr<NoteWithOffTime>> pointersToSelectedNotes = stashSelectedNotes();
            processor->undoMgr->beginNewTransaction();
            MIDIProcessor::ActionChangeNoteOffTimes *action;
            action = new MIDIProcessor::ActionChangeNoteOffTimes(*processor, delta, pointersToSelectedNotes);
            processor->undoMgr->perform(action);
            draggingOffTime = false;
            hoveringOver = HOVER_NONE;
        }
        else if (drawingVelocity)
        {
            //perform undoablecommand changing velocities to velocitiesAfterDragOrDraw
            Array<Sequence::NoteVelocities> newVelocities;
            for (int i = 0; i < selectedNotes.size(); i++)
            {
                Sequence::NoteVelocities vel;
                vel.note = pSequence->at(selectedNotes[i]);
                vel.velocity = pSequence->at(selectedNotes[i])->getVelocity();
                newVelocities.add(vel);
                pSequence->at(selectedNotes[i])->velocity = velsStartDrag[i];
            }
            processor->undoMgr->beginNewTransaction();
            MIDIProcessor::ActionChangeVelocities *action;
            action = new MIDIProcessor::ActionChangeVelocities(*processor, newVelocities);
            processor->undoMgr->perform(action);
            processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits,
                                         processor->getZTLTime(horizontalShift));
            drawingVelocity = false;
            hoveringOver = HOVER_NONE;
            startTimer(TIMER_MOUSE_UP, 1);
        }

        preDragXinTicks = xInTicks;
        if (vert > 0.0 && xInTicks > 0.0) //Test if we are on a note bar
        {
            bool inHead = false;
            bool inBar = false;
            for (int i = 0; i < pSequence->size(); i++)
            {
                if (pSequence->at(i)->head.contains(sequenceScaledX, scaledY))
                {
                    inHead = true;
                    hoverStep = i;
                    break;
                }
            }
            if (!inHead)
            {
                for (int i = 0; i < pSequence->size(); i++)
                {
                    if (pSequence->at(i)->bar.contains(sequenceScaledX, scaledY))
                    {
                        inBar = true;
                        hoverStep = i;
                        break;
                    }
                }
            }
            if (inHead || inBar)
            {
                if (inHead)
                    hoveringOver = HOVER_NOTEHEAD;
                else
                    hoveringOver = HOVER_NOTEBAR;
            }
        }

        zoomDragStarting = false;
        zoomOrScrollDragging = false;
        if (hoveringOver == HOVER_ZEROTIMEHANDLE) //Clicked on ZTL handle - add/remove bookmark
        {
            if (!processor->atZTL())
                processor->catchUp();
            else
            {
                processor->catchUp();
                processor->addRemoveBookmark(BOOKMARK_TOGGLE);
            }
        } else if ((hoverChord < 0) && hoveringOver == HOVER_NOTEHEAD)
        {
            if (abs(distanceMouseMovedSinceMouseDown)<=1 && !showingVelocities.getValue())
            {
                //We do the actual work on the message thread by calling a timer that turns itself off after one tick.
                if (!draggingVelocity && !draggingTime && !draggingOffTime && processor->getNotesEditable() &&
                    hoverStep >= 0)
                    startTimer(TIMER_TOGGLE_TARGET_NOTE, 1);
            } else
            {

            }
        } else
            hoveringOver = HOVER_NONE;
        processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits,
                                     processor->getZTLTime(horizontalShift));
        editingNote = false;
        drawingVelocity = false;
        repaint();
    } catch (const std::out_of_range &ex)
    {
        std::cout << " error noteviewer: mouseUp " << "\n";
    }
}

void ScrollingNoteViewer::mouseDoubleClick(const MouseEvent &e)
{
    if (e.position.getY() < topMargin * ViewStateInfo::verticalScale)
    { //Test mouse position
        horizontalScale = 1.f;
        setHorizontalShift(0.f);
        repaint();
    }
}

void ScrollingNoteViewer::mouseDrag(const MouseEvent &event)
{
    if (!event.source.hasMouseMovedSignificantlySincePressed())
        return;
    if (event.position.getY() < topMargin * ViewStateInfo::verticalScale)
        event.source.enableUnboundedMouseMovement(true, false);
    const double x = event.position.getX();
    mouseXinTicks = ((x - (horizontalShift + xPositionOfBaseLine)) / pixelsPerTick) / horizontalScale +
                    processor->getTimeInTicks();
    stopTimer(TIMER_MOUSE_HOLD);
    curDragPosition = event.getPosition();
    startTimer(TIMER_MOUSE_DRAG, 1);
}

void ScrollingNoteViewer::mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel)
{
    processor->leadLag = 0;
    processor->fullPowerMode = true;
    //TODO - Should move the stopping of the timer out of the mouseWheelMove thread
    if (processor->isPlaying && processor->getZTLTime(0)>0.1 && wheel.deltaX>0.0f)
    {
        processor->play(false,"current");
    }
    bool useDeltaX = abs(wheel.deltaX) > abs(wheel.deltaY);
    float newShift;
    if (useDeltaX)
        newShift = horizontalShift-300*wheel.deltaX;
    else
        newShift = horizontalShift + 300 * wheel.deltaY;
    const double seqStartRelToLeftEdgeInPixels =
            (xPositionOfBaseLine - processor->getTimeInTicks()*pixelsPerTick*horizontalScale +
             newShift)*horizontalScale;
    double seqDurationInPixels = processor->sequenceObject.seqDurationInTicks*pixelsPerTick;
    double scaledSeqDurationInPixels = seqDurationInPixels*horizontalScale*horizontalScale;
    const double seqEndRelToLeftEdgeInPixels = seqStartRelToLeftEdgeInPixels + scaledSeqDurationInPixels;
    if (seqStartRelToLeftEdgeInPixels > xPositionOfBaseLine*horizontalScale)
        newShift = processor->getTimeInTicks()*pixelsPerTick*horizontalScale;
    else if (seqEndRelToLeftEdgeInPixels < xPositionOfBaseLine*horizontalScale)
        newShift =  processor->getTimeInTicks()*pixelsPerTick*horizontalScale - scaledSeqDurationInPixels/horizontalScale;
//    std::cout << "newShift " << newShift << "\n";
    setHorizontalShift(newShift);
    processor->sendChangeMessage();
    repaint();
}
void ScrollingNoteViewer::mouseMagnify (const MouseEvent& event, float scaleFactor)
{
    
}

void ScrollingNoteViewer::mouseMove(const MouseEvent &event)
{
    try
    {
        std::vector<std::shared_ptr<NoteWithOffTime>> *pSequence = &(processor->sequenceObject.theSequence);
        const double x = event.position.getX();
        const double y = event.position.getY();
        const double vert = (y - ViewStateInfo::verticalScale * topMargin) / ViewStateInfo::trackVerticalSize;
        mouseXinTicks = ((x - (horizontalShift + xPositionOfBaseLine)) / pixelsPerTick) / horizontalScale +
                        processor->getTimeInTicks();
        const float sequenceScaledX = mouseXinTicks * pixelsPerTick;
        const float scaledY = y / ViewStateInfo::verticalScale;
        preDragXinTicks = mouseXinTicks;
        hoverChord = -1;
        if (showingChords)
        {
            if (vert > 0.0 && mouseXinTicks > 0.0) //Test if we are on a chord
            {
                int i;
                int ch = -1;
                if (showingChords)
                {
                    for (i = 0; i < processor->sequenceObject.chords.size(); i++)
                    {
                        if (processor->sequenceObject.chords.at(i).chordRect.expanded(2.0, 0.0).contains(
                                sequenceScaledX, scaledY))
                        {
                            ch = i;
                            break;
                        }
                    }
                    hoverChord = ch;
                }
                if (ch == -1)
                    hoverChord = -1;
                else
                {
                    //            std::cout << "mouseMove found step " << step <<"\n";
                    hoverChord = ch;
                }
//            std::cout << "in mouseMove hoverChord " << hoverChord << "\n";
                sendChangeMessage();  //Being sent to VieweFrame to display the info in the toolbar
            }
        }


        if (vert > 0.0 && mouseXinTicks > 0.0) //Test if we are on a note bar
        {
            const int nn = maxNote - vert + 1;
            bool inHead = false;
            bool inBar = false;
            for (int i = 0; i < pSequence->size(); i++)
            {
                if (pSequence->at(i)->head.contains(sequenceScaledX, scaledY))
                {
                    inHead = true;
                    hoverStep = i;
                    break;
                }
            }
            if (!inHead)
            {
                for (int i = 0; i < pSequence->size(); i++)
                {
                    if (pSequence->at(i)->bar.contains(sequenceScaledX, scaledY))
                    {
                        inBar = true;
                        hoverStep = i;
                        break;
                    }
                }
            }

//        std::cout << "set hoverStep " << step <<"\n";
            if (!(inHead || inBar))
            {
                hoveringOver = HOVER_NOTETRACK;
                String note = MidiMessage::getMidiNoteName(nn, true, true, 3);
                if (selectedNotes.size() > 1)
                {
                    const double time1 = pSequence->at(selectedNotes[0])->getTimeStamp();
                    const double time2 = pSequence->at(selectedNotes.getLast())->getTimeStamp();
                    note = note + " Selection width: " + String(std::abs(time1 - time2) / 10.0, 1);
                }
                if (!processor->playing())
                    hoverInfo = note;
                else
                    hoverInfo = "";
            } else
            {
                if (inHead)
                    hoveringOver = HOVER_NOTEHEAD;
                else
                    hoveringOver = HOVER_NOTEBAR;

                if (!processor->playing())
                {
                    hoverInfo = MidiMessage::getMidiNoteName(nn, true, true, 3)
                                + ": note number " + String::String(nn) +
                                + "\ntrack " + String::String(pSequence->at(hoverStep)->track)+ "; "
                                + "channel " + String::String(pSequence->at(hoverStep)->channel)
                                + "\nvelocity " + String(127.0 * pSequence->at(hoverStep)->velocity)+ "; "
                                + "duration " +
                                String((pSequence->at(hoverStep)->getOffTime() -
                                        pSequence->at(hoverStep)->getTimeStamp()) /
                                       10.0, 1) +
                                +"\ntick " + String(pSequence->at(hoverStep)->getTimeStamp() / 10.0, 1)+ "; "
                                + "step " + String::String(hoverStep);
                }
            }
//        std::cout << "mouseMove HOVER = " << hoveringOver << "\n";
            sendChangeMessage();  //Being sent to VieweFrame to display the info in the toolbar
        } else if (!selecting && fabs(xPositionOfBaseLine - x) <= 4)
        {
            if (0 < y && y && event.position.getY() < ViewStateInfo::verticalScale * topMargin)
                hoveringOver = HOVER_ZEROTIMEHANDLE;
            else
            {
//            std::cout << "HOVER_ZEROTIMELINE" << "\n";
                hoveringOver = HOVER_ZEROTIMELINE;
            }
        } else
            hoveringOver = HOVER_NONE;
//    if (hoveringOver!=HOVER_NONE)
//        std::cout << "Hover " << hoveringOver << "\n";
        repaint();
//    std::cout << "hoveringOver " << hoveringOver <<"\n";
    } catch (const std::out_of_range &ex)
    {
        std::cout << " error noteviewer: mouseMove " << "\n";
    }
}

int ScrollingNoteViewer::addNote(/*bool playable, */float x, float y,  float w, float h, float headWidth, float headHeight,
                                 Colour colHead, Colour colBar)
{
    const float headY = y-(headHeight-h)/2.0;
    int id = addRectangle(x, y, w, h, colBar); //Bar
    addRectangle(x, headY, headWidth, headHeight, colHead);
    return id;
}

int ScrollingNoteViewer::addRectangle(float x, float yy, float w, float hh, Colour col)
{
    const int firstVertex = ViewStateInfo::vertices.size(); //There are four vertices per rectangle
    float y = ViewStateInfo::initialHeight-yy;
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
    ViewStateInfo::vertices.add(v0);
    Vertex v1 =
    {
        { x, y+h},
        { r, g, b}
    };
    ViewStateInfo::vertices.add(v1);
    Vertex v2 =
    {
        { x+w, y+h},
        { r, g, b}
    };
    ViewStateInfo::vertices.add (v2);
    Vertex v3 =
    {
        { x+w, y},
        { r, g, b}
    };
    ViewStateInfo::vertices.add (v3);
    //Define the order in which the vertices of the rectangle are used
    ViewStateInfo::indices.add(0+firstVertex);
    ViewStateInfo::indices.add(1+firstVertex);
    ViewStateInfo::indices.add(2+firstVertex);
    ViewStateInfo::indices.add(0+firstVertex);
    ViewStateInfo::indices.add(2+firstVertex);
    ViewStateInfo::indices.add(3+firstVertex);
    return firstVertex/4;
}

int ScrollingNoteViewer::addQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Colour col)
{
    const int firstVertex = ViewStateInfo::vertices.size(); //There are four vertices per quad
    float r = col.getFloatRed();
    float g = col.getFloatGreen();
    float b = col.getFloatBlue();
    //Define the four vertices of a rectangle
    Vertex v0 =
    {
        { x1, y1},
        { r, g, b}
    };
    ViewStateInfo::vertices.add(v0);
    Vertex v1 =
    {
        { x2, y2},
        { r, g, b}
    };
    ViewStateInfo::vertices.add(v1);
    Vertex v2 =
    {
        { x3, y3},
        { r, g, b}
    };
    ViewStateInfo::vertices.add (v2);
    Vertex v3 =
    {
        { x4, y4},
        { r, g, b}
    };
    ViewStateInfo::vertices.add (v3);
    //Define the order in which the vertices of the rectangle are used
    ViewStateInfo::indices.add(0+firstVertex);
    ViewStateInfo::indices.add(1+firstVertex);
    ViewStateInfo::indices.add(2+firstVertex);
    ViewStateInfo::indices.add(0+firstVertex);
    ViewStateInfo::indices.add(2+firstVertex);
    ViewStateInfo::indices.add(3+firstVertex);
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
    return Matrix3D<float>::fromFrustum (0.f, getWidth()/horizScale, 0.f, ViewStateInfo::viewHeight/vertScale, 4.0f, 30.0f);
}

Matrix3D<float> ScrollingNoteViewer::getViewMatrix(float x) const
{
    Matrix3D<float> viewMatrix (Vector3D<float> (x, 0.0f, -4.0f));
    Matrix3D<float> rotationMatrix = viewMatrix.rotation (Vector3D<float> (0.0f, 0.0f, 0.0f));
    return rotationMatrix * viewMatrix;
}

void ScrollingNoteViewer::setRectangleColour (int rect, Colour col)
{
    float r = col.getFloatRed();
    float g = col.getFloatGreen();
    float b = col.getFloatBlue();
    float colour[3] = {r, g, b};
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
	openGLContext.extensions.glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect) + 8,
                    sizeof(colour),
                    colour);
	openGLContext.extensions.glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+1) + 8,
                    sizeof(colour),
                    colour);
	openGLContext.extensions.glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+2) + 8,
                    sizeof(colour),
                    colour);
	openGLContext.extensions.glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+3) + 8,
                    sizeof(colour),
                    colour);
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void ScrollingNoteViewer::setRectanglePos(int rect, float x, float yy, float w, float hh)
{
    float y = ViewStateInfo::initialHeight-yy;
    float h = -hh;
    
    float v0[2] = { x, y};
    float v1[2] = { x, y+h};
    float v2[2] = { x+w, y+h};
    float v3[2] = { x+w, y};
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
	openGLContext.extensions.glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect),
                    sizeof(v0),
                    v0);
	openGLContext.extensions.glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+1),
                    sizeof(v1),
                    v1);
	openGLContext.extensions.glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(Vertex)*(4*rect+2),
                    sizeof(v2),
                    v2);
	openGLContext.extensions.glBufferSubData(GL_ARRAY_BUFFER,
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
        uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix(horizontalScale, ViewStateInfo::verticalScale).mat, 1, false);
    
    if (uniforms->viewMatrix != nullptr)
        uniforms->viewMatrix->setMatrix4 (getViewMatrix(0.f).mat, 1, false);
}

void ScrollingNoteViewer::resetHorizontalShift() {
//    std::cout << "horizontalShift " <<horizontalShift<<"\n";
    setHorizontalShift(0.f);
}

void ScrollingNoteViewer::renderOpenGL()
{
//    String refreshTime = String(Time::getHighResolutionTicks() - renderStart);
//  renderStart = Time::getHighResolutionTicks();
  double timeShiftInPixels;
  try {
        std::vector<std::shared_ptr<NoteWithOffTime>> *pSequence = &(processor->sequenceObject.theSequence);
        if (rebuidingGLBuffer)
	            return;
        const ScopedLock myScopedLock (glRenderLock);
        if (!processor->fullPowerMode)
            return;
        rendering = true;
        ++frameCounter;
        jassert (OpenGLHelpers::isContextActive());
      
        desktopScale = (float) openGLContext.getRenderingScale();
        OpenGLHelpers::clear (Colour::greyLevel (0.1f));
        if (glBufferUpdateCountdown > 0)
            glBufferUpdateCountdown--;
        if  (sequenceChanged && glBufferUpdateCountdown == 0)// && ViewStateInfo::vertices.size()>0)
        {
            glBufferUpdateCountdown = 2; //Number of renders that must pass before we are allowed in here again
//            resized();
            openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
            openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
            openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                                   static_cast<GLsizeiptr> (static_cast<size_t> (ViewStateInfo::vertices.size()) * sizeof (Vertex)),
                                                   ViewStateInfo::vertices.getRawDataPointer(),
                                                   GL_DYNAMIC_DRAW);
            
            numIndices = 6*(ViewStateInfo::vertices.size()/4);
            //generate buffer object name(s) (names are ints) (indexBuffer is an GLuint)
            openGLContext.extensions.glGenBuffers (1, &indexBuffer); //Gets id of indexBuffer
            
            //bind a named buffer object (to a buffer type such as GL_ELEMENT_ARRAY_BUFFER)
            openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                                                   static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof (juce::uint32)),
                                                   ViewStateInfo::indices.getRawDataPointer(), GL_STATIC_DRAW);
            sequenceChanged = false;
        }
        if (processor->resetViewer)
        {
            processor->resetViewer = false;
            setHorizontalShift(0);
        }
        if (numIndices==0)
        {
            rendering = false;
            return;
        }
        glEnable (GL_BLEND);
        glEnable(GL_MULTISAMPLE);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * ViewStateInfo::viewHeight));
        shader->use();
        static double prevTime;
        timeShiftInPixels = -processor->getTimeInTicks()*pixelsPerTick;

        prevTime = processor->getTimeInTicks();
		openGLContext.extensions.glUniformMatrix4fv(Uniforms::viewMatrixHandle, 1, false, getViewMatrix(timeShiftInPixels+(xPositionOfBaseLine+horizontalShift)/horizontalScale).mat);
		openGLContext.extensions.glUniformMatrix4fv(Uniforms::projectionMatrixHandle, 1, false, getProjectionMatrix(horizontalScale, ViewStateInfo::verticalScale).mat);
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
                String note = MidiMessage::getMidiNoteName (stepsThatChanged[j], true, true, 3);
                if (stepsThatChanged[j]<0) //If was an off
                {
                    const int step = -(stepsThatChanged[j]+1);
                    setRectangleColour(pSequence->at(step)->rectHead, colourInactiveNoteHead);//Head
                }
                else if (stepsThatChanged[j]>0) //It was an on
                {
                    setRectangleColour(pSequence->at(stepsThatChanged[j]-1)->rectHead,   colourNoteOn); //Head
                }
            }
        }
        if (!processor->playing() || processor->waitingForFirstNote)
        {
            int step = processor->lastPlayedSeqStep+1;
            if (step<pSequence->size())
            {
                const float timeStamp = pSequence->at(step)->getTimeStamp();
                const float width = 4.0f;// (sequence->at(step+1).timeStamp-timeStamp)*pixelsPerTick;
                setRectanglePos(nextNoteRect, timeStamp*pixelsPerTick, 0.0f, width, 15.f);
            }
        }
        else
            setRectanglePos(nextNoteRect, 0.0f, 2.f, 0.0f, 13.f); //Make invisible with zero width
    } catch (const std::exception& e) {
        std::cout << " error noteviewer: render openGl" << e.what()<< "\n";
    }
    rendering = false;
    if (!ViewStateInfo::openGLStarted)
    {
        ViewStateInfo::openGLStarted = true;
    }
//    int64 renderDuration = Time::getHighResolutionTicks()-renderStart;
//    if (renderDuration > 0.003)
//    String rd = String(renderDuration);
//    double renderStartMS = ((double)renderStart)/Time::getHighResolutionTicksPerSecond();
//    renderDuration++;
//    std::cout << "renderDuration " <<renderDuration<<"\n";
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
    String statusText = String();
    
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
    }
    else
    {
        std::cout <<  newShader->getLastError()<<"\n";
    }
}

//==============================================================================
void ScrollingNoteViewer::makeKeyboard()
{
//    std::cout << "Entering makeKeyboard \n";
    ticksPerQuarter = processor->sequenceObject.getPPQ();
    timeSigChanges = processor->sequenceObject.getTimeSigInfo();
    timeSigChanges[0].getTimeSignatureInfo(numerator, denominator);
    pixelsPerTick = ViewStateInfo::initialWidth/(ticksPerQuarter * numerator * initialMeasuresAcrossWindow);
    leadTimeInTicks = ((getWidth()-wKbd+leftMargin)/pixelsPerTick)*leadTimeProportionOfWidth;
    processor->setLeadTimeInTicks(leadTimeInTicks);
    xPositionOfBaseLine = leadTimeInTicks*pixelsPerTick;
    processor->sequenceObject.getNotesUsed(minNote,maxNote);
    nKeys = maxNote-minNote+1;
    int height = ViewStateInfo::viewHeight;
    if (height==0)
        height = 300;
    keysImage = Image(Image::RGB, leftMargin+wKbd, height, true);
    Graphics keysGr (keysImage);
    
    const Colour textColour (findColour (textLabelColourId));
    ViewStateInfo::trackVerticalSize = ((float)ViewStateInfo::viewHeight-ViewStateInfo::verticalScale*topMargin)/nKeys;
    if (ViewStateInfo::trackVerticalSize<1)
        ViewStateInfo::trackVerticalSize = 1;
    
    //left margin
    keysGr.setColour(Colours::black);
    keysGr.fillRect(0,0,roundToInt(leftMargin),roundToInt(ViewStateInfo::viewHeight));
    keysGr.setColour(Colour(64,64,64));
    //topMargin
    int topMargHeight = ViewStateInfo::verticalScale*(topMargin+1.0f);
    keysGr.fillRect(0, 0, roundToInt(wKbd+leftMargin), topMargHeight);
    
    const float fontHeight = jmin (12.0f, ViewStateInfo::trackVerticalSize * 0.9f);
    for (int note = maxNote; note >= minNote; note--) //Draw keys
    {
        const float noteY = (maxNote-note) * ViewStateInfo::trackVerticalSize;
        noteYs[note]= noteY;
        const String text (getNoteText (note));
        if (processor->sequenceObject.isBlackNote(note))
        {
            keysGr.setColour (Colour(Colours::black));
            keysGr.fillRect((float)leftMargin, noteY+topMargHeight, wKbd, ViewStateInfo::trackVerticalSize);
        }
        else
        {
            keysGr.setColour (Colour(Colours::white));
            keysGr.fillRect((float)leftMargin, noteY+topMargHeight, wKbd, ViewStateInfo::trackVerticalSize);
            keysGr.setColour (Colour(Colours::black));
            keysGr.drawLine((float)leftMargin, ViewStateInfo::trackVerticalSize+noteY+topMargHeight,
                                    leftMargin+wKbd, ViewStateInfo::trackVerticalSize+noteY+topMargHeight,1.5f);
        }
        if (ViewStateInfo::trackVerticalSize>6)
        {
            keysGr.setFont (Font (fontHeight).withHorizontalScale (0.95f));
            if (processor->sequenceObject.isBlackNote(note))
                keysGr.setColour(Colours::white);
            else
                keysGr.setColour(Colours::black);
            keysGr.drawText (text,3, roundToInt(noteY+2.f+topMargHeight), (int)wKbd-4,(int)ViewStateInfo::trackVerticalSize-4,
                             Justification::centredLeft, false);
        }
    }
    keysGr.setColour(Colours::grey);
    keysGr.fillRect(roundToInt(wKbd+leftMargin-2), 0, 2, roundToInt(ViewStateInfo::viewHeight));
}

void ScrollingNoteViewer::makeNoteBars()
{
    try
    {
        std::vector<std::shared_ptr<NoteWithOffTime>> *pSequence = &(processor->sequenceObject.theSequence);
        rebuidingGLBuffer = true;
        if (processor->initialWindowHeight < topMargin || ViewStateInfo::viewHeight < 0.0000001f)
        {
            std::cout << "... but failed to enter makeNoteBars " << "\n";
            return;
        }
        horizontalScale = processor->sequenceObject.sequenceProps.getDoubleValue("horizontalScale", var(1.0));
        const float rescaleHeight = ((float) ViewStateInfo::initialHeight) / ViewStateInfo::viewHeight;
        const float unscaledTVS = ViewStateInfo::trackVerticalSize / ViewStateInfo::verticalScale;
        ViewStateInfo::vertices.clear();
        ViewStateInfo::indices.clear();
        processor->sequenceObject.getNotesUsed(minNote, maxNote);
        nKeys = maxNote - minNote + 1;
        const float h = unscaledTVS * noteBarWidthRatio; //Note bar height
        if (seqSize != static_cast<int>(pSequence->size()))
            clearSelectedNotes();
        seqSize = static_cast<int>(pSequence->size());
        int seqDurationInTicks = processor->sequenceObject.getSeqDurationInTicks();
        const int sequenceWidthPixels = seqDurationInTicks + xPositionOfBaseLine;

        const float noteBarVerticalSize = unscaledTVS * noteBarWidthRatio;

        //Top margin
        addRectangle(-sequenceWidthPixels, 0.f, sequenceWidthPixels * 30, topMargin, Colour(44, 44, 44));
        addRectangle(-sequenceWidthPixels, topMargin - 1.0f, sequenceWidthPixels * 30, 1.0f,
                     Colour(0xFFB0B0B0).darker());
        addRectangle(-sequenceWidthPixels, topMargin + 2.f, sequenceWidthPixels * 30, 2.f, Colour(127, 127, 127));

        if (seqSize == 0)
        {
            rebuidingGLBuffer = false;
            return;
        }

        //Black & white note track highlighting
        for (int note = minNote; note <= maxNote; note++)
        {
            if (processor->sequenceObject.isBlackNote(note))
                addRectangle(-sequenceWidthPixels, noteYs[note] * rescaleHeight + topMargin, sequenceWidthPixels * 30,
                             unscaledTVS, Colours::black);
            else
                addRectangle(-sequenceWidthPixels, noteYs[note] * rescaleHeight + topMargin, sequenceWidthPixels * 30,
                             unscaledTVS, Colour(0xFF404040));
        }

        //Beat & measure lines
        if (!processor->sequenceObject.hideMeasureLines)
        {
            for (int beat = 0; beat < processor->sequenceObject.beatTimes.size(); beat++)
                addRectangle(processor->sequenceObject.beatTimes[beat] * pixelsPerTick - 1.6, topMargin, 0.8 / sqrt(horizontalScale),
                             ViewStateInfo::initialHeight - topMargin, Colour(0xFFB0B0B0).darker());
            for (int measure = 0; measure < processor->sequenceObject.measureTimes.size(); measure++)
                addRectangle(processor->sequenceObject.measureTimes[measure] * pixelsPerTick - 2.2, topMargin, 1.0 / sqrt(horizontalScale),
                             ViewStateInfo::initialHeight - topMargin, Colour(0xFFE0E0E0));
        }

        //First line
        addRectangle(processor->sequenceObject.measureTimes[0] * pixelsPerTick - 2.2,
                     topMargin,
                     3.0 / sqrt(horizontalScale),
                     ViewStateInfo::initialHeight,
                     Colour(0xFFE0E0E0));
        //Last line
        addRectangle(processor->sequenceObject.measureTimes[processor->sequenceObject.measureTimes.size()-1] * pixelsPerTick - 2.2,
                     0.f,
                     3.0 / sqrt(horizontalScale),
                     ViewStateInfo::initialHeight,
                     Colour(0xFFE0E0E0));

        //Velocity graph
        const double graphHeight =
                (300.0 - 15.0) - 2 * toolbarHeight; //(300.0-15.0) the original viewer height set in MainComponent.cpp

        double prevY = graphHeight * pSequence->at(0)->highestVelocityInChain;
        double prevX = -1;
        for (int index = 0; index < static_cast<int>(pSequence->size()); index++)
        {
            const double startPixel = pSequence->at(index)->getTimeStamp() * pixelsPerTick;
            double x = startPixel;
            if (pSequence->at(index)->targetNote)
            {
                const float velocityOfTargetNote = pSequence->at(index)->velocity;
                const double scaledVelocity = graphHeight * velocityOfTargetNote;
                const double thisY = scaledVelocity;
                if (prevX != -1)
                {
                    addLine(prevX, prevY, x, scaledVelocity, 1.0f, Colour(0xFFF0F0FF));
                    prevY = thisY;
                }
                prevX = x;
            }
        }

        //Tempo
        if (processor->sequenceObject.scaledTempoChanges.size() > 1)
        {
            double startTempo =
                    60.0 / processor->sequenceObject.scaledTempoChanges.at(0).getTempoSecondsPerQuarterNote();
            double prevY = graphHeight * (startTempo / 300.0);
            double prevX = 0;
            for (int i = 1; i < processor->sequenceObject.scaledTempoChanges.size(); i++)
            {
                const double timeStamp = processor->sequenceObject.scaledTempoChanges.at(i).getTimeStamp();

                const double thisX = timeStamp * pixelsPerTick;
                const double tempo =
                        60.0 / processor->sequenceObject.scaledTempoChanges.at(i).getTempoSecondsPerQuarterNote();
                const double thisY = graphHeight * (tempo / 300.0);
                addLine(prevX, prevY, thisX, prevY, 1.0f, Colour(Colours::red));
                //              std::cout << "addLine " << timeStamp <<" "<<tempo<< " "<<prevX<<" "<< prevY<<"\n";
                prevY = thisY;
                prevX = thisX;
            }
        }

        //Note Bars ###
        const double readHead = processor->getSequenceReadHead();
        const float headToBarHeightRatio = 1.0 + 0.4 * (std::max(nKeys - 10.0, 0.0)) / 88.0;
        const float headHeight = h * headToBarHeightRatio;
        Array<NoteBarDescription> deferredNoteBars; //Info to defer making active note bars until inactive ones are made
        int prevChordIndex = 0;
        RectangleList<float> chordRects;
        bool prevInChord = false;
        int size = (int) pSequence->size();
        for (int index = 0; index < size; index++)
        {
            //        if (index>=21 && index<=31)
            //            std::cout<< "noteBar: step, ts "<< index<<" "<<pSequence->at(index)->getTimeStamp()<< "\n";
            const double startPixel = pSequence->at(index)->getTimeStamp() * pixelsPerTick;
            double endPixel = pSequence->at(index)->getOffTime() * pixelsPerTick;
            const int noteNumber = pSequence->at(index)->noteNumber;
            const double thisEndTime = pSequence->at(index)->getOffTime();
            const float barY = noteYs[noteNumber] * rescaleHeight + topMargin;
            //        if (index<2)
            //            std::cout
            //            << " i "<<index
            //            << " noteYs[noteNumber] " << noteYs[noteNumber]
            //            << " barY "<<barY
            //            << " rescaleHeight "<<rescaleHeight
            //            << " topMargin "<<topMargin
            //            << "\n";
            int indexOfNextSameNote = -1;
            const double minSpacing = 1.0;
            float headWidth = 6.0f;

            //Adjust the head width if other note of same note number follows closely
            for (int nxtNoteIndex = index + 1; nxtNoteIndex < size - 2; nxtNoteIndex++)
            {
                const double spacing = (pSequence->at(nxtNoteIndex)->getTimeStamp() - thisEndTime) * pixelsPerTick;
                if (spacing > headWidth)
                    break;
                if (pSequence->at(nxtNoteIndex)->noteNumber == noteNumber)
                {
                    indexOfNextSameNote = nxtNoteIndex;
                    //                std::cout<< "index,  indexOfNextSameNote "<< index<<" "<<indexOfNextSameNote<< "\n";
                    break;
                }
            }
            const float barX = startPixel;
            const float w = endPixel - startPixel;
            double startPixelOfNextSameNote;

            if (indexOfNextSameNote == -1)
                startPixelOfNextSameNote = DBL_MAX;
            else
                startPixelOfNextSameNote = pSequence->at(indexOfNextSameNote)->getTimeStamp() * pixelsPerTick;

            if (headWidth > startPixelOfNextSameNote - barX)
                headWidth = std::max(minSpacing, startPixelOfNextSameNote - barX - minSpacing);
            if (headWidth < 3.0f)
                headWidth = 3.0f;

            const float vel = pSequence->at(index)->velocity;
            const Colour vBasedNoteBar = Colour::fromFloatRGBA(0.3f + 0.7f * vel, 0.2f + 0.6f * vel, 0.3f + 0.7f * vel,
                                                               1.0f);
            pSequence->at(index)->head = Rectangle<float>(barX, barY - (headHeight - h) / 2.0, headWidth, headHeight);
            pSequence->at(index)->bar = Rectangle<float>(barX, barY - (headHeight - h) / 2.0, w, headHeight);
            //        if (index<25)
            //            std::cout << "Step " << index << " head X " <<pSequence->at(index)->head.getX()<<"\n";
            //Determine the cord rectangle, if part of a chord
            if (processor->sequenceObject.chords.size() > 0)
            {
                const int thisChordIndex = pSequence->at(index)->chordIndex;
                //            if (index>25)
                //                std::cout << "Step " << index << " chordIndex " <<thisChordIndex<<"\n";
                if (index == size - 1 || (!(pSequence->at(index)->inChord) && prevInChord) ||
                    thisChordIndex != prevChordIndex)
                { //Ended a chord so save its rectangle in ChordDetail
                    //            std::cout << "At " << index-1 << " End chord "<< prevChordIndex <<"\n";
                    RectangleList<float> rList;
                    for (int i = 0; i < processor->sequenceObject.chords.at(prevChordIndex).notePointers.size(); i++)
                    {
                        Rectangle<float> head = processor->sequenceObject.chords.at(prevChordIndex).notePointers.at(
                                i)->head;
                        rList.add(head);
                    }
                    Rectangle<float> chordRect = rList.getBounds();
                    chordRect.translate(-1.0, 0.0);
                    processor->sequenceObject.chords.at(prevChordIndex).chordRect = chordRect.withWidth(1.5);
                }
                if (pSequence->at(index)->inChord)
                {
                    prevChordIndex = pSequence->at(index)->chordIndex;
                }
            }

            //        std::cout << "makeNoteBars at step "<<index;
            if (pSequence->at(index)->targetNote) //If it's a target note
            {
                if (processor->getNotesEditable() || pSequence->at(index)->getTimeStamp() >= readHead)
                {
                    NoteBarDescription nbd;
                    nbd.seqIndex = index;
                    nbd.x = barX;
                    nbd.y = barY - 0.5;
                    nbd.w = w;
                    nbd.headWidth = headWidth;
                    nbd.headHeight = headHeight + 1.0;
                    if (index > 0 && (pSequence->at(index)->getTimeStamp() == pSequence->at(index - 1)->getTimeStamp()))
                        nbd.colHead = colourActiveNoteHead.darker().darker();
                    else
                        nbd.colHead = colourActiveNoteHead;
                    nbd.colBar = vBasedNoteBar;
                    deferredNoteBars.add(nbd);
                } else
                {
                    pSequence->at(index)->rectBar = addNote(barX, barY, w, noteBarVerticalSize, headWidth, headHeight,
                                                            colourInactiveNoteHead, vBasedNoteBar);
                }
            } else
            {
                if (index > 0 && (pSequence->at(index)->getTimeStamp() == pSequence->at(index - 1)->getTimeStamp()) &&
                    (processor->getNotesEditable()
                     || pSequence->at(index)->getTimeStamp() >= readHead))
                {
                    pSequence->at(index)->rectBar =
                            addNote(barX, barY, w, noteBarVerticalSize, headWidth, headHeight,
                                    colourInactiveNoteHead.darker(), vBasedNoteBar);
                } else
                {
                    pSequence->at(index)->rectBar =
                            addNote(barX, barY, w, noteBarVerticalSize, headWidth, headHeight,
                                    colourInactiveNoteHead.brighter(), vBasedNoteBar);
                }
            }
            //        std::cout << " rectHead index "<<sequence->at(index)->rectHead<<"\n";
            pSequence->at(index)->rectHead = pSequence->at(index)->rectBar + 1;
            prevInChord = pSequence->at(index)->inChord;
        }
        for (int i = 0; i < deferredNoteBars.size(); i++)
        {
            pSequence->at(deferredNoteBars[i].seqIndex)->rectBar =
                    addNote(deferredNoteBars[i].x, deferredNoteBars[i].y,
                            deferredNoteBars[i].w, noteBarVerticalSize + 1.0,
                            deferredNoteBars[i].headWidth, deferredNoteBars[i].headHeight,
                            deferredNoteBars[i].colHead, deferredNoteBars[i].colBar);
        }
        //Sustain bars
        if (processor->sequenceObject.sustainPedalChanges.size() > 0)
        {
            //        std::cout << "In make note bars, sustainPedalChanges "<<processor->sequenceObject.sustainPedalChanges.size()<<"\n";
            Array<Rectangle<double>> sustainBars;
            double sustainStartTick = 0;
            //First make an array of Rectangles each with just a bar's start tick and width in ticks
            for (int i = 0; i < processor->sequenceObject.sustainPedalChanges.size(); i++)
            {
//                if (i<20)
//                    std::cout << "In make note bars " << i << " " << processor->sequenceObject.sustainPedalChanges.at(i).timeStamp
//                    << " value " << processor->sequenceObject.sustainPedalChanges.at(i).pedalOn
//                    <<"\n";
                if (processor->sequenceObject.sustainPedalChanges.at(i).pedalOn)
                {
                    sustainStartTick = processor->sequenceObject.sustainPedalChanges.at(i).timeStamp;
                } else if (sustainStartTick != -1 && !processor->sequenceObject.sustainPedalChanges.at(i).pedalOn)
                {
                    sustainBars.add(Rectangle<double>(sustainStartTick, 0,
                                                      processor->sequenceObject.sustainPedalChanges.at(i).timeStamp -
                                                      sustainStartTick, 0));
                }
            }
            //Then scan the sequence looking for the highest note in the range of each bar and draw the bar just higher than hignest note
            
            bool inSustainBar = false;
            int highestNote = -1;
            int sustainBarNum = 0;
//            std::cout << "pSequence->size() " << pSequence->size() << std::endl;
            for (int step = 0; step < static_cast<int>(pSequence->size()); step++)
            {
                //            const NoteWithOffTime msg = sequence->at(step);
                const double barLeft = sustainBars[sustainBarNum].getX();
                const double barRight = sustainBars[sustainBarNum].getRight();
                const double msgTimeStamp = pSequence->at(step)->getTimeStamp();
                if (!inSustainBar) //Msg is after the start of this bar
                {
//                    std::cout << "Not in sustain bar "<<step<<" "<<msgTimeStamp<<" "<<barRight<<"\n";
                    if (msgTimeStamp > barRight)
                    {
                        const double barLeft = sustainBars[sustainBarNum].getX();
                        const double barRight = sustainBars[sustainBarNum].getRight();
                        if (highestNote != -1)
                        {
                            const float y = noteYs[highestNote] * rescaleHeight + topMargin - 0. * unscaledTVS;
                            addRectangle(barLeft * pixelsPerTick, y, (barRight - barLeft) * pixelsPerTick, 1.5,
                                         Colour(Colours::orange).brighter());
                            sustainBarNum++;
                        }
                        if (msgTimeStamp >= barLeft)
                        {
//                            std::cout << "Sustain bar ended before next note "<<step<<" "<<msgTimeStamp<<" "<<barRight<<"\n";
							if (highestNote == -1)
								highestNote = pSequence->at(step)->noteNumber;
                            const float y = noteYs[highestNote] * rescaleHeight + topMargin - 0. * unscaledTVS;
                            addRectangle(barLeft * pixelsPerTick, y, (barRight - barLeft) * pixelsPerTick, 1.5,
                                         Colour(Colours::orange).brighter());
                            sustainBarNum++;
                        }
                    } else if (msgTimeStamp >= barLeft)
                    {
                        inSustainBar = true;
                    }
                }
                else
                {
//                    std::cout << "In sustain bar"<<step<<"\n";
                    if (inSustainBar && msgTimeStamp > barRight)
                    {
                        inSustainBar = false;
                        const float y = noteYs[highestNote] * rescaleHeight + topMargin - 0.5 * unscaledTVS;
                        addRectangle(barLeft * pixelsPerTick, y, (barRight - barLeft) * pixelsPerTick, 1.5,
                                     Colour(Colours::orange).brighter());
//                        std::cout << "Make sustain bar x, y " <<barLeft * pixelsPerTick<<" "<<y  << std::endl;
                        sustainBarNum++;
                        if (msgTimeStamp > barRight)
                            highestNote = pSequence->at(step)->noteNumber;
                        else
                            highestNote = -1;
                    }
                }
                if (inSustainBar)
                {
                    if (pSequence->at(step)->noteNumber > highestNote)
                        highestNote = pSequence->at(step)->noteNumber;
                }
            }
        } //End sustain bars

        //soft bars
        if (processor->sequenceObject.softPedalChanges.size() > 0)
        {
            Array<Rectangle<double>> softBars;
            double softStartTick = 0;
            //First make an array of Rectangles each with just a bar's start tick and width in ticks
            for (int i = 0; i < processor->sequenceObject.softPedalChanges.size(); i++)
            {
                if (processor->sequenceObject.softPedalChanges.at(i).pedalOn)
                {
                    softStartTick = processor->sequenceObject.softPedalChanges.at(i).timeStamp;
                } else if (softStartTick != -1 && !processor->sequenceObject.softPedalChanges.at(i).pedalOn)
                {
                    softBars.add(Rectangle<double>(softStartTick, 0,
                                                   processor->sequenceObject.softPedalChanges.at(i).timeStamp -
                                                   softStartTick, 0));
                }
            }
            //Then scan the sequence looking for the highest note in the range of each bar and draw the bar just higher than hignest note
            int countSofts = 0;
            bool insoftBar = false;
            int highestNote = -1;
            int softBarNum = 0;
            for (int step = 0; step < static_cast<int>(pSequence->size()); step++)
            {
                const double barLeft = softBars[softBarNum].getX();
                const double barRight = softBars[softBarNum].getRight();
                const double msgTimeStamp = pSequence->at(step)->getTimeStamp();
                if (!insoftBar) //Msg is after the start of this bar
                {
                    if (msgTimeStamp > barRight)
                    {
                        const double barLeft = softBars[softBarNum].getX();
                        const double barRight = softBars[softBarNum].getRight();
                        const float y = noteYs[highestNote] * rescaleHeight + topMargin - 2.3 * unscaledTVS;
                        countSofts++;
                        addRectangle(barLeft * pixelsPerTick, y, (barRight - barLeft) * pixelsPerTick, 1.5,
                                     Colour(Colours::lightblue).brighter());
                        softBarNum++;
                    } else if (msgTimeStamp >= barLeft)
                    {
                        insoftBar = true;
                    }
                } else if (insoftBar && msgTimeStamp >= barRight)
                {
                    insoftBar = false;
                    const float y = 3.0;
                    countSofts++;
                    addRectangle(barLeft * pixelsPerTick, y, (barRight - barLeft) * pixelsPerTick, 2.0,
                                 Colour(Colours::lightblue).brighter());

                    softBarNum++;
                    if (msgTimeStamp > barRight)
                        highestNote = pSequence->at(step)->noteNumber;
                    else
                        highestNote = -1;
                }
                if (insoftBar)
                {
                    if (pSequence->at(step)->noteNumber > highestNote)
                        highestNote = pSequence->at(step)->noteNumber;
                }
            }
        } //End soft bars
        //Bookmarks
        //        std::cout << processor->sequenceObject.bookmarkTimes.size() << "\n";
        for (int i = 0; i < processor->sequenceObject.bookmarkTimes.size(); i++)
        {
            const double x = processor->sequenceObject.bookmarkTimes[i].time * pixelsPerTick;
            Colour col;
            if (processor->sequenceObject.bookmarkTimes[i].tempoChange)
                col = Colour(Colours::red);
            else
                col = juce::Colour(Colours::whitesmoke);
            addRectangle(x - 1.95, 0.0f, 5 / sqrt(horizontalScale), topMargin, col);
        }

        //    //Position of next note to play
        //    if (processor->lastPlayedSeqStep+1 < processor->sequenceObject.theSequence.size())
        //    {
        //        const double x = processor->sequenceObject.theSequence.at(processor->lastPlayedSeqStep+1)->getTimeStamp()*pixelsPerTick;
        //        nextNoteRect = addRectangle(x-1.95, 0,     4, (topMargin),Colours::green);
        //    }
        rebuidingGLBuffer = false;

    } catch (...)
    {
        std::cout << "Error in make note bars" << "\n";
    }
//    std::cout << "Leaving  makeNoteBars: Indices, Vertices " <<  ViewStateInfo::indices.size()<<" "<<ViewStateInfo::vertices.size() << "\n";
//    std::cout << "Exit MNB: theSequence.size " << "\n";
}

void ScrollingNoteViewer::updatePlayedNotes()
{

}

//<#paint#>
//###
void ScrollingNoteViewer::paint (Graphics& g)
{
    if (rendering)
        return;
    std::vector<std::shared_ptr<NoteWithOffTime>> *pSequence = &(processor->sequenceObject.theSequence);
    //Start of most recently played note
    if (processor->isPlaying && !processor->waitingForFirstNote)
    {
        const double hLinePos = 2.8 * horizontalScale + xPositionOfBaseLine + processor->leadLag * pixelsPerTick * horizontalScale;
        g.setColour (colourNoteOn);
        g.fillRect(Rectangle<float>(hLinePos,topMargin*ViewStateInfo::verticalScale, 1.1, ViewStateInfo::viewHeight-topMargin*ViewStateInfo::verticalScale));
        int timePlayed = processor->leadLag + processor->getTimeInTicks();
        int spacingInPixels = (processor->nextDueTargetNoteTime - processor->lastPlayedTargetNoteTime)*pixelsPerTick * horizontalScale;
        int nextDueTargetNoteXPos = hLinePos + spacingInPixels;
        std::cout
//        <<" xPositionOfBaseLine "<< xPositionOfBaseLine
//        <<" processor->leadLag "<< processor->leadLag
        <<" timePlayed " << timePlayed
//        << " lastPlayedTargetNoteTime  "<< processor->lastPlayedTargetNoteTime
//        << " nextDueTargetNoteTime  "<< processor->nextDueTargetNoteTime
        << " xPositionOfBaseLine  "<< xPositionOfBaseLine
        << " nextDueTargetNoteXPos " << nextDueTargetNoteXPos
        << " leadLag " << processor->leadLag
        << " hLinePos  "<< hLinePos
        << " prevHLinePos " << prevHLinePos
        << " late/early " << prevHLinePos - hLinePos
        << " late/early in ticks " << (prevHLinePos - hLinePos)/pixelsPerTick
        << "\n";
        g.setColour (Colours::mediumseagreen);
        g.fillRect(Rectangle<float>(nextDueTargetNoteXPos,topMargin*ViewStateInfo::verticalScale, 1.1, ViewStateInfo::viewHeight-topMargin*ViewStateInfo::verticalScale));
        prevHLinePos = hLinePos;
    }
    else
    {
        if (processor->getLastUserPlayedStepTime()>=0.0)
        {
            const double lastTime = processor->getLastUserPlayedStepTime() - processor->getTimeInTicks();
            const double hLinePos = 2.8 * horizontalScale + xPositionOfBaseLine + lastTime * pixelsPerTick * horizontalScale + horizontalShift;
            g.setColour (colourNoteOn);
            g.fillRect(Rectangle<float>(hLinePos,topMargin*ViewStateInfo::verticalScale, 1.1, ViewStateInfo::viewHeight-topMargin*ViewStateInfo::verticalScale));
        }
    }
    
    //ZTL & relative adjusted velocity indicator
    Colour ztlCol;
    if (processor->isPlaying)
    {
        if (processor->waitingForFirstNote)
            ztlCol = Colour(Colours::orange);
        else
            ztlCol = Colour(Colours::green).brighter();
    }
    else
        ztlCol = Colour(30,30,255).brighter(); //Blue

    g.setColour (ztlCol);
    g.fillRect(Rectangle<float>(xPositionOfBaseLine-1.f,0.0, 2.0, ViewStateInfo::viewHeight)); //ZTL
    
    const float vtrIndicatorLevel = (ViewStateInfo::viewHeight-topMargin*ViewStateInfo::verticalScale) *
                    ( 0.5 + (1.0f-processor->variableTempoRatio));
    const float vtrIndicatorNullPt = (ViewStateInfo::viewHeight-topMargin*ViewStateInfo::verticalScale) * 0.5;
    const float tempoIndicatorLength = 20;
    g.setColour (Colours::yellow);
    g.fillRect(Rectangle<float>(xPositionOfBaseLine-1.f-tempoIndicatorLength/2,vtrIndicatorLevel+
                                topMargin*ViewStateInfo::verticalScale, tempoIndicatorLength, 2.0)); //Moving indicator
    g.setColour (ztlCol);
    g.fillRect(Rectangle<float>(xPositionOfBaseLine-1.f-tempoIndicatorLength/2,vtrIndicatorNullPt+
                                topMargin*ViewStateInfo::verticalScale, tempoIndicatorLength, 2.0)); //Zero point
    
    const int meas = processor->getMeasure(horizontalShift);
    const int totalMeas = (int) processor->sequenceObject.measureTimes.size();
    Font f = Font (10.0*ViewStateInfo::verticalScale);
    f.setStyleFlags(Font::FontStyleFlags::bold);
    g.setFont(f);
    g.setColour (Colour(199,199,199));
    const String measTxt = String(meas)+"/"+String(totalMeas-1)+" ["+String(processor->getZTLTime(horizontalShift)/10.0,1)+"]";
    if (xPositionOfBaseLine!=-1)
        if (processor->sequenceObject.measureTimes.size()>0)
            g.drawText(measTxt, xPositionOfBaseLine+6, 3.0*ViewStateInfo::verticalScale, 150,
                       9*ViewStateInfo::verticalScale, juce::Justification::centredLeft);
    if (!processor->isPlaying)
    {
        if (processor->undoMgr->inRedo || processor->undoMgr->inUndo)
        {
            displayedSelection.clear();
            for (int i=0;i<processor->sequenceObject.selectionToRestoreForUndoRedo.size();i++)
            {
                displayedSelection.add(processor->sequenceObject.selectionToRestoreForUndoRedo.at(i)->currentStep);
            }
            setSelectedNotes(displayedSelection);
            processor->undoMgr->inUndo = false;
            processor->undoMgr->inRedo = false;
        }
        if (!(marqueeAddingNotes||marqueeRemovingNotes||markingSelectedNotes||clearingSelectedNotes) &&
            (((hoveringOver == HOVER_NOTEHEAD && showingVelocities.getValue()) || draggingVelocity) && hoverStep>=0))
        {
            const float vel = pSequence->at(hoverStep)->velocity;
            const float graphHeight = ViewStateInfo::viewHeight - topMargin*ViewStateInfo::verticalScale;
            const float velY = ((1.0-vel) * graphHeight) + topMargin*ViewStateInfo::verticalScale;
            const Rectangle<float> scaledHead = pSequence->at(hoverStep)->head;
            const float velX = scaledHead.getX()*horizontalScale+xPositionOfBaseLine+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale;
            const Line<float> velLine = Line<float> (velX,
                                                     velY,
                                                     velX+6.0f*horizontalScale,
                                                     velY);
            const float fontHeight = jmin (14.0f, ViewStateInfo::trackVerticalSize * 8.0f);
            Font f = Font (fontHeight).withHorizontalScale (0.95f);
            f.setStyleFlags(Font::FontStyleFlags::bold);
            g.setFont (f);
            g.setColour (Colours::white);
            const String velString = String (std::round(vel*127.0));
            double textY = noteYs[pSequence->at(hoverStep)->noteNumber]+(topMargin+0.0)*ViewStateInfo::verticalScale;
            g.drawText (velString, velX-32.0, textY, 28.0, 8.0, Justification::centredRight, false);
            g.setColour (Colours::seagreen);
            g.drawLine(velLine, 5.0f);
        }
        //###
        if (showingChords)
        {
            for (int ch=0;ch<processor->sequenceObject.chords.size();ch++)
            {
                const Rectangle<float> rct = processor->sequenceObject.chords.at(ch).chordRect.expanded(0.15, 0.0);
                float widthFactor;
                if (hoverChord==ch || processor->sequenceObject.chords.at(ch).chordSelected)
                {
                    if (hoverChord==ch)
                        g.setColour (Colours::yellow);
                    else
                        g.setColour (Colours::white);
                    widthFactor = 1.6;
                }
                else
                {
                    g.setColour (Colours::white);
                    widthFactor = 1.0;
                }
                const Rectangle<float> chordRect = Rectangle<float>(
                               rct.getX()*horizontalScale+xPositionOfBaseLine+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                               rct.getY()*ViewStateInfo::verticalScale,
                               rct.getWidth()*horizontalScale * widthFactor,
                               rct.getHeight()*ViewStateInfo::verticalScale);
                g.fillRect(chordRect);
                
                for (int np=0;np<processor->sequenceObject.chords.at(ch).notePointers.size();np++)
                {
                    Rectangle<float> headRct = processor->sequenceObject.chords.at(ch).notePointers.at(np)->head;
                    headRct = Rectangle<float>(
                                                                        headRct.getX()*horizontalScale+xPositionOfBaseLine+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                                                                        headRct.getY()*ViewStateInfo::verticalScale,
                                                                        headRct.getWidth()*horizontalScale,
                                                                        headRct.getHeight()*ViewStateInfo::verticalScale);
                    Rectangle<float> connectorRect = Rectangle<float>(chordRect.getX(), headRct.getBottom()-1.0*ViewStateInfo::verticalScale,
                                                        headRct.getRight()-chordRect.getX(),1.0*ViewStateInfo::verticalScale);
                    connectorRect.setLeft(chordRect.getTopLeft().getX());
                    g.fillRect(connectorRect);
                }
            }
        }
        Point<float> prevVelPoint;
        for (int i=0;i<displayedSelection.size();i++)
        {
            const Rectangle<float> scaledHead = pSequence->at(displayedSelection[i])->head;
            const Rectangle<float> head = Rectangle<float>(
                 scaledHead.getX()*horizontalScale+xPositionOfBaseLine+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                 scaledHead.getY()*ViewStateInfo::verticalScale,
                 scaledHead.getWidth()*horizontalScale,
                 scaledHead.getHeight()*ViewStateInfo::verticalScale).expanded(2, 2);
            g.setColour (Colours::whitesmoke);
            g.drawRect(head, 1.5);
            
            if (showingVelocities.getValue())
            {
                const float vel = pSequence->at(displayedSelection[i])->velocity;
                const float graphHeight = ViewStateInfo::viewHeight - topMargin*ViewStateInfo::verticalScale;
                const float velY = ((1.0-vel) * graphHeight) + topMargin*ViewStateInfo::verticalScale;
                const float velX = scaledHead.getX()*horizontalScale+xPositionOfBaseLine+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale;
                Point<float> velPoint(velX, velY);
                if (i>0)
                {
                    const Line<float> velLine = Line<float> (prevVelPoint, velPoint);
                    g.setColour (Colours::seagreen.brighter().brighter());
                    g.drawLine(velLine, 2.5f);
                }
                prevVelPoint = velPoint;
            }
        }
        g.setColour (Colours::yellow);
        if (selecting)
            g.drawRect(selectionRect,2);
        
        if (!(marqueeAddingNotes||marqueeRemovingNotes||markingSelectedNotes||clearingSelectedNotes))
        {
            if (draggingTime && hoverStep>=0)
            {
                g.setColour (Colour(0xFFF0F0FF));
                const Rectangle<float> scaledHead = pSequence->at(hoverStep)->head;
                Rectangle<float> guideLine = Rectangle<float>(
                       (timeAfterDrag*pixelsPerTick)*horizontalScale+xPositionOfBaseLine+horizontalShift -
                                                               processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                       (scaledHead.getY())*ViewStateInfo::verticalScale,
                                                               
                       2.0*horizontalScale,
                       scaledHead.getHeight()*ViewStateInfo::verticalScale);
                
                g.drawRect(guideLine, 1.5);
                guideLine.setY(topMargin*ViewStateInfo::verticalScale);
                guideLine.setWidth(0.5f*horizontalScale);
                guideLine.setHeight(ViewStateInfo::viewHeight - topMargin*ViewStateInfo::verticalScale);
                g.drawRect(guideLine, 0.5);
            }
            else if (draggingOffTime && hoverStep>=0)
            {
                g.setColour (Colour(0xFFF0F0FF));
                const Rectangle<float> scaledHead = pSequence->at(hoverStep)->head;
                Rectangle<float> guideLine = Rectangle<float>(
                       (offTimeAfterDrag*pixelsPerTick)*horizontalScale+xPositionOfBaseLine+horizontalShift -
                       processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                       (scaledHead.getY())*ViewStateInfo::verticalScale,
                       2.0*horizontalScale,
                       scaledHead.getHeight()*ViewStateInfo::verticalScale);
                g.drawRect(guideLine, 1.0);
                guideLine.setY(topMargin*ViewStateInfo::verticalScale);
                guideLine.setWidth(0.5f*horizontalScale);
                guideLine.setHeight(ViewStateInfo::viewHeight - topMargin*ViewStateInfo::verticalScale);
                g.drawRect(guideLine, 0.5);
            }
        }
    }
    if (!ViewStateInfo::finishedPaintAfterRewind)
    {
        ViewStateInfo::finishedPaintAfterRewind = true;
        prevHLinePos = 2.8 * horizontalScale + xPositionOfBaseLine + processor->leadLag * pixelsPerTick * horizontalScale;
//        std::cout <<"prevHLinePos, rewind leadLag, xPositionOfBaseLine "
//        <<prevHLinePos
//        << " " <<processor->leadLag
//        << " " <<xPositionOfBaseLine
//        <<"\n";
    }
}

void ScrollingNoteViewer::changeListenerCallback (ChangeBroadcaster*
                                                  broadcaster)
{
  try {
    if ((MIDIProcessor*)broadcaster == processor) //Triggered at the end of rewind() in MIDIProcessor
    {
//        std::cout << " ViewerCallback:  " <<  processor->changeMessageType <<" animationStep "<<animationStep<<"\n";
        if (processor->changeMessageType == CHANGE_MESSAGE_REWIND)
        {
            if (processor->sequenceObject.fileToLoad != prevFileLoaded)
                clearSelectedNotes();
//            std::cout <<"animationStep "<<animationStep<<"\n";
            if (animationStep<=1) //Skip rebuilding gl objects in the middle of tweening
            {
                    if (processor->getTimeInTicks()==0)
                    {
                        makeKeyboard();
                        makeNoteBars();
                        sequenceChanged = true;
                    }
                    else
                    {
                        makeKeyboard();
                        makeNoteBars();
                        sequenceChanged = true;
                    }
            }
//            else
//            {
//                std::cout << "animationStep "<<animationStep<<"\n";
//            }
            setHorizontalShift(0);
            prevFileLoaded = processor->sequenceObject.fileToLoad;
            ViewStateInfo::openGLStarted = false;
            ViewStateInfo::finishedPaintAfterRewind = false;
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
//            animationStep = 0;
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
            animationStep = 0;
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
//            std::cout << "in CHANGE_MESSAGE_UNDO "<< "\n";
            processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getTimeInTicks());
            if (processor->undoMgr->inRedo || processor->undoMgr->inUndo)
            {
                displayedSelection.clear();
                for (int i=0;i<processor->sequenceObject.selectionToRestoreForUndoRedo.size();i++)
                {
                    displayedSelection.add(processor->sequenceObject.selectionToRestoreForUndoRedo.at(i)->currentStep);
                }
                //            displayedSelection = processor->sequenceObject.selectionToRestoreForUndoRedo;
                selectedNotes = displayedSelection;
                
                setSelectedNotes(displayedSelection);
                processor->setCopyOfSelectedNotes(displayedSelection);
                
                double goToTime;
                if (selectedNotes.size()>0)
                {
                    bool anyTickVisible = false;
                    for (int i=0; i<selectedNotes.size();i++)
                        if(tickIsVisible(processor->sequenceObject.theSequence.at(selectedNotes[i])->getTimeStamp()))
                        {
                            anyTickVisible=true;
                            break;
                        }
                    if (anyTickVisible)
                        goToTime = processor->getZTLTime(-1);
                    else
                        goToTime = processor->sequenceObject.theSequence.at(selectedNotes[0])->getTimeStamp();
                    processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, goToTime);
                }
            }
            if (animationStep==0)
            {
                makeKeyboard();
                makeNoteBars();
            }
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
  } catch (const std::out_of_range& ex) {
      std::cout << " error in SNV: changeListener callback " << "\n";
  }
}

void ScrollingNoteViewer::timerCallback (int timerID)
{
	//return;
    int wid = selectionRect.getWidth();
  try {
    std::vector<std::shared_ptr<NoteWithOffTime>> *pSequence = &(processor->sequenceObject.theSequence);
    if (timerID == TIMER_PERIODIC)
    {
		if (processor->isPlaying || processor->isListening)
			return;
        if (prevShowingVelocities != (bool)showingVelocities.getValue() && !showingVelocities.getValue())
        {
            drawingVelocities.setValue(false);
            adjustingVelocities.setValue(false);
        }
        else if (drawingVelocities.getValue() || adjustingVelocities.getValue())
        {
            showingVelocities.setValue(true);
        }
        if ((prevDrawingVelocities != (bool)drawingVelocities.getValue())
            || (prevAdjustingVelocities != (bool)adjustingVelocities.getValue()))
        {
            bool drawingVelocitiesChanged;
            if (prevDrawingVelocities != (bool)drawingVelocities.getValue())
                drawingVelocitiesChanged = true;
            else
                drawingVelocitiesChanged = false;
            if (drawingVelocitiesChanged)
            {
                drawingVelocities.setValue(!prevDrawingVelocities);
                prevDrawingVelocities = drawingVelocities.getValue();
                if (drawingVelocities.getValue())
                {
                    adjustingVelocities.setValue(false);
                    prevAdjustingVelocities = false;
                }
            }
            else
            {
                adjustingVelocities.setValue(!prevAdjustingVelocities);
                prevAdjustingVelocities = adjustingVelocities.getValue();
                if (adjustingVelocities.getValue())
                {
                    drawingVelocities.setValue(false);
                    prevDrawingVelocities = false;
                }
            }
        }
        prevShowingVelocities = showingVelocities.getValue();
        
        //Cursor setting
        if (marqueeAddingNotes)
        {
            if (getMouseCursor()!=marqueeAddingCursor)
                setMouseCursor(marqueeAddingCursor);
        }
        else if (marqueeRemovingNotes)
        {
            if (getMouseCursor()!=marqueeRemovingCursor)
                setMouseCursor(marqueeRemovingCursor);
        }
        else if (drawingVelocities.getValue())
        {
            if (getMouseCursor()!=editVelocityCursor)
                setMouseCursor(editVelocityCursor);
        }
        else
            setMouseCursor(MouseCursor::NormalCursor);
        repaint();
    }
    else if (timerID == TIMER_TWEEN)
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
            animationStep = 0;
            stopTimer(TIMER_TWEEN);
        }
    }
    else if (timerID == TIMER_TOGGLE_TARGET_NOTE)
    {
        stopTimer(TIMER_TOGGLE_TARGET_NOTE);
        processor->undoMgr->beginNewTransaction();
        MIDIProcessor::ActionSetNoteActivity* action;
        bool setNotesActive = !processor->getNoteActivity(hoverStep);
//        std::cout << "setNotesActive " << setNotesActive << "\n";
        if (!selectedNotes.contains(hoverStep))
        {
            Array<int> oneStep;
            oneStep.add(hoverStep);
//            setSelectedNotes(oneStep);
            action = new MIDIProcessor::ActionSetNoteActivity(*processor, setNotesActive, oneStep);
        }
        else
            action = new MIDIProcessor::ActionSetNoteActivity(*processor, setNotesActive, selectedNotes);
//        std::cout << "setNoteActivity " << setNotesActive <<" step "<<selectedNotes[0]<< "\n";
        processor->undoMgr->perform(action);
    }
    else if (timerID == TIMER_MOUSE_HOLD)
    {
        stopTimer(TIMER_MOUSE_HOLD);
//        std::cout << "Here - mouse hold " << selectionAnchor.getY() << "\n";
        if (hoverChord<0 && !ModifierKeys::getCurrentModifiers().isCommandDown())// && (marqueeAddingNotes||marqueeRemovingNotes))
        {
            selecting = true;
            if(ModifierKeys::getCurrentModifiers().isShiftDown())
                marqueeAddingNotes = true;
            if(ModifierKeys::getCurrentModifiers().isAltDown())
                marqueeRemovingNotes = true;
        }
    }
    else if (timerID == TIMER_MOUSE_UP)
    {
        stopTimer(TIMER_MOUSE_UP);
        processor->catchUp();
        processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getTimeInTicks());
    }
    else if (timerID == TIMER_MOUSE_DRAG)
    {
//        std::cout <<"Entering TIMER_MOUSE_DRAG "<<"\n";
        stopTimer(TIMER_MOUSE_DRAG);
        static double prevY;
        double y = curDragPosition.getY();
        if (showingVelocities.getValue() && drawingVelocities.getValue() && !drawingVelocity && !zoomDragStarting)
        {
            velsStartDrag.clear();
            for (int i=0;i<selectedNotes.size();i++)
                velsStartDrag.add(pSequence->at(selectedNotes[i])->getVelocity());
            drawingVelocity = true;
        }
        else if (drawingVelocity && !zoomOrScrollDragging)
        {
//            std::cout
//            << " curDragPosition " << curDragPosition.getX()<<","<<curDragPosition.getY()
//            << " mouseXinTicks " << mouseXinTicks
//            << "\n";
//            float mouseY = y - topMargin/verticalScale - toolbarHeight;
            selecting = false;
            int ntNdx;
            for (ntNdx=0;ntNdx<selectedNotes.size();ntNdx++)
            {
                const int noteTS = pSequence->at(selectedNotes[ntNdx])->getTimeStamp();
                const int xInTicks = std::round(mouseXinTicks);
                const float diff = noteTS > xInTicks ? noteTS - xInTicks : xInTicks - noteTS;
                if (diff<=100/horizontalScale)
                {
                    float velocity = 1.0f-(y + topMargin/ViewStateInfo::verticalScale - toolbarHeight)/(ViewStateInfo::viewHeight - topMargin);
                    velocity = velocity < 0.0f?0.0f:velocity;
                    velocity = velocity > 1.0f?1.0f:velocity;
//                    std::cout
//                    << " set step " <<selectedNotes[ntNdx]
//                    << " yy " << yy
//                    << " mouseY " << mouseY
//                    << " velocity " << velocity
////                    << " mouseXinTicks " << mouseXinTicks
//                    << "\n";
                    pSequence->at(selectedNotes[ntNdx])->velocity = velocity;
                }
            }
            repaint();
            return;
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
                double proportionOfSequenceLeftOfMouse = (preDragXinTicks-processor->getTimeInTicks())/processor->sequenceObject.seqDurationInTicks;
                double shiftLeft = preDragHorizShift-seqLengthInPixels *
                proportionOfSequenceLeftOfMouse*(absScaleChangeWithCorrectSign);
                float shift = (curDragPosition.getX() - mouseBeforeDrag.getX());
                setHorizontalShift(shiftLeft+shift);
                processor->sequenceObject.sequenceProps.setValue("horizontalScale", var(horizontalScale));
                repaint();
            }
        }
        else
        {
            if (selecting)// && !ModifierKeys::getCurrentModifiers().isAltDown())
            {
                int xInTicksLeft = 0;
                int xInTicksRight = -1;
                Rectangle<float> selRect = Rectangle<float>();
                if (!ModifierKeys::getCurrentModifiers().isCommandDown())
                {
                    //Use latest point to extend the selection region (region may be a Rectangle or Path)
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
                }
                
                xInTicksLeft = ((selectionRect.getX() - (horizontalShift+xPositionOfBaseLine))/pixelsPerTick)/
                horizontalScale + processor->getTimeInTicks();
                
                xInTicksRight = ((selectionRect.getRight() - (horizontalShift+xPositionOfBaseLine))/pixelsPerTick)/
                horizontalScale + processor->getTimeInTicks();
                selRect = Rectangle<float>::leftTopRightBottom((float)selectionRect.getX(),
                                                                                (float)selectionRect.getY(),
                                                                                (float)selectionRect.getRight(),
                                                                                (float)selectionRect.getBottom()
                                                                                );

                bool suppressAutoscroll = false;
                if (xInTicksLeft < 0)
                {
                    int xReversed = ((0-processor->getTimeInTicks())*horizontalScale)*pixelsPerTick+(horizontalShift+xPositionOfBaseLine);
                    selectionRect.setX(xReversed);
                    selectionRect.setWidth(wid);
                    suppressAutoscroll = true;
//                    std::cout << "set selectionRect " <<selectionRect.getX()<<" "<<selectionRect.getWidth()  << std::endl;
                }

                if (xInTicksRight > processor->sequenceObject.seqDurationInTicks)
                {
                    int xReversed = ((processor->sequenceObject.seqDurationInTicks-processor->getTimeInTicks())*horizontalScale)*pixelsPerTick+(horizontalShift+xPositionOfBaseLine);
                    selectionRect.setWidth(wid);
                    selectionRect.setRight(xReversed);
                    suppressAutoscroll = true;
//                    std::cout << "set selectionRect " <<selectionRect.getX()<<" "<<selectionRect.getWidth()  << std::endl;
                }
                
                float shiftDelta = 0;
                bool autoScrolling = false;
                if (!suppressAutoscroll && curDragPosition.getX()<10)
                {
                    shiftDelta = 5;
                    autoScrolling = true;
                }
                else if (!suppressAutoscroll && curDragPosition.getX()>(ViewStateInfo::viewWidth-10))
                {
                    shiftDelta = -5;
                    autoScrolling = true;
                }
                if (autoScrolling && !(horizontalShift==0 && shiftDelta>0))
                {
                    startTimer(TIMER_MOUSE_DRAG, 50);
                    float shift = horizontalShift+shiftDelta;
                    selectionAnchor = selectionAnchor.translated(shiftDelta, 0);
                    if (horizontalShift<0)
                        horizontalShift = 0;
                    const double seqStartRelToLeftEdgeInPixels =
                    (xPositionOfBaseLine - processor->getTimeInTicks()*pixelsPerTick*horizontalScale +
                     shift)*horizontalScale;
                    double seqDurationInPixels = processor->sequenceObject.seqDurationInTicks*pixelsPerTick;
                    double scaledSeqDurationInPixels = seqDurationInPixels*horizontalScale*horizontalScale;
                    const double seqEndRelToLeftEdgeInPixels = seqStartRelToLeftEdgeInPixels + scaledSeqDurationInPixels;
                    if (seqStartRelToLeftEdgeInPixels > xPositionOfBaseLine*horizontalScale)
                        shift = processor->getTimeInTicks()*pixelsPerTick*horizontalScale;
                    else if (seqEndRelToLeftEdgeInPixels < xPositionOfBaseLine*horizontalScale)
                        shift =  processor->getTimeInTicks()*pixelsPerTick*horizontalScale - scaledSeqDurationInPixels/horizontalScale;
                    setHorizontalShift(shift);
                    processor->sendChangeMessage();
                    repaint();
                } else
                    stopTimer(TIMER_MOUSE_DRAG);
                
                //Check all sequence steps within time range of region to see if they are in the region
                newlySelectedNotes.clear();
                for (int step=0;step<pSequence->size();step++)
                {
                    if (pSequence->at(step)->getTimeStamp() >= xInTicksLeft
                        && pSequence->at(step)->getTimeStamp() <= xInTicksRight)
                    {
                        const Rectangle<float> scaledHead = pSequence->at(step)->head;
                        const Rectangle<float> head = Rectangle<float>(
                                                                 scaledHead.getX()*horizontalScale+xPositionOfBaseLine+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                                                                 scaledHead.getY()*ViewStateInfo::verticalScale,
                                                                 scaledHead.getWidth()*horizontalScale,
                                                                 scaledHead.getHeight()*ViewStateInfo::verticalScale);
                        
                        if (head.intersects(selRect))
                        {
                             newlySelectedNotes.add(step);
                        }
                        
                    }
                }
                displayedSelection.clear();
                int minSelNoteTime = INT_MAX;
                int maxSelNoteTime = 0;
                for (int i=0;i<newlySelectedNotes.size();i++)
                {
                    if (!selectedNotes.contains(newlySelectedNotes[i]) && !marqueeRemovingNotes)
                    {
                        displayedSelection.add(newlySelectedNotes[i]);
                    }
                    if (pSequence->at(newlySelectedNotes[i])->getTimeStamp()<minSelNoteTime)
                        minSelNoteTime = pSequence->at(newlySelectedNotes[i])->getTimeStamp();
                    if (pSequence->at(newlySelectedNotes[i])->getTimeStamp()>maxSelNoteTime)
                        maxSelNoteTime = pSequence->at(newlySelectedNotes[i])->getTimeStamp();
                }
                for (int i=0;i<selectedNotes.size();i++)
                {
                    if (marqueeAddingNotes && !newlySelectedNotes.contains(selectedNotes[i]))
                        displayedSelection.add(selectedNotes[i]);
                    else if (marqueeRemovingNotes && newlySelectedNotes.contains(selectedNotes[i]))
                        ;
                    else
                        displayedSelection.add(selectedNotes[i]);
                }

                NoteTimeComparator comparator(processor);
                displayedSelection.sort(comparator);

//                std::cout
//                <<  " selected "<<selectedNotes.size()
//                <<  " newlySelected "<<newlySelectedNotes.size()
//                <<  " displayedSelection "<<displayedSelection.size()
//                <<  "\n";
                if (newlySelectedNotes.size()==0)
                    hoverInfo.clear();
                else
                    {
                        hoverInfo = "Range in ticks: "+ String(minSelNoteTime/10.0,1)+ " to "+String(maxSelNoteTime/10.0,1)
                        +" width: "+String((maxSelNoteTime-minSelNoteTime)/10.0,1);
                    }

                sendChangeMessage();  //Being sent to VieweFrame to display the info in the toolbar
                repaint();
            }
            else if ((markingSelectedNotes || clearingSelectedNotes) && !ModifierKeys::getCurrentModifiers().isAltDown())
            {
//                if (!ModifierKeys::getCurrentModifiers().isCommandDown())
//                    clearSelectedNotes();
//                int mouseY = curDragPosition.getY()/verticalScale-topMargin;
                Point<float> mousePt = Point<float>(curDragPosition.getX(),curDragPosition.getY());
//                std::cout << "mousex, Y "<<curDragPosition.getX()<<" "<<curDragPosition.getY()<<"\n";
                newlySelectedNotes.clear();
                for (int step=0;step<pSequence->size();step++)
                {
                    if (true)///*pSequence->at(step)->chordTopStep==-1 &&*/
                        //pSequence->at(step)->getTimeStamp() >= xInTicksLeft
                        //&& pSequence->at(step)->getTimeStamp() <= xInTicksRight)
                    {
                        const Rectangle<float> scaledHead = pSequence->at(step)->head;
                        const Rectangle<float> head = Rectangle<float>(
                                                                       scaledHead.getX()*horizontalScale+xPositionOfBaseLine+horizontalShift - processor->getTimeInTicks()*pixelsPerTick*horizontalScale,
                                                                       scaledHead.getY()*ViewStateInfo::verticalScale,
                                                                       scaledHead.getWidth()*horizontalScale,
                                                                       scaledHead.getHeight()*ViewStateInfo::verticalScale);
//                        std::cout << "head "<<head.getX()<<" "<<head.getY()<<"\n";
                        if (head.contains (mousePt))
                        {
                            newlySelectedNotes.add(step);
//                            std::cout << "hit "<<step<<"\n";
                        }
                    }
                }
                int minSelNoteTime = INT_MAX;
                int maxSelNoteTime = 0;
                for (int i=0;i<selectedNotes.size();i++)
                {
                    if (!displayedSelection.contains(selectedNotes[i]))
                        displayedSelection.add(selectedNotes[i]);
                }
                for (int i=0;i<newlySelectedNotes.size();i++)
                {
                    if (markingSelectedNotes && !displayedSelection.contains(newlySelectedNotes[i]))
                    {
                        displayedSelection.add(newlySelectedNotes[i]);
                        selectedNotes.add(newlySelectedNotes[i]);
                        pSequence->at(newlySelectedNotes[i])->isSelected = true;
                    }
                    else if (clearingSelectedNotes && displayedSelection.contains(newlySelectedNotes[i]))
                    {
                        displayedSelection.remove(displayedSelection.indexOf(newlySelectedNotes[i]));
                        selectedNotes.remove(selectedNotes.indexOf(newlySelectedNotes[i]));
                    }
                    
                    if (pSequence->at(newlySelectedNotes[i])->getTimeStamp()<minSelNoteTime)
                        minSelNoteTime = pSequence->at(newlySelectedNotes[i])->getTimeStamp();
                    if (pSequence->at(newlySelectedNotes[i])->getTimeStamp()>maxSelNoteTime)
                        maxSelNoteTime = pSequence->at(newlySelectedNotes[i])->getTimeStamp();
                }
                
                NoteTimeComparator comparator(processor);
                displayedSelection.sort(comparator);
                
//                std::cout
//                <<  " selected "<<selectedNotes.size()
//                <<  " newlySelected "<<newlySelectedNotes.size()
//                <<  " displayedSelection "<<displayedSelection.size()
//                <<  "\n";
                if (newlySelectedNotes.size()==0)
                    hoverInfo.clear();
                else
                {
                    hoverInfo = "Range in ticks: "+ String(minSelNoteTime)+ " to "+String(maxSelNoteTime)
                    +" width in sixteenths: "+String((maxSelNoteTime-minSelNoteTime)/60.0);
                }
                
                sendChangeMessage();  //Being sent to VieweFrame to display the info in the toolbar
                repaint();
            }
            else if (editingNote) //Dragging on note head
            {
//                std::cout << "Note head drag : hover step = "<<hoverStep <<"\n";
                if (!displayedSelection.contains(hoverStep))
                {
                    displayedSelection.clear();
                    displayedSelection.add(hoverStep);
                    setSelectedNotes(displayedSelection);
                }
                float deltaX = noteEditAnchor.getX() - Desktop::getMousePosition().getX();
                float deltaY = noteEditAnchor.getY() - Desktop::getMousePosition().getY();
                if (!draggingVelocity && !draggingTime && !draggingOffTime && !drawingVelocity)
                {
                    velocityAfterDrag = -1;
                    timeAfterDrag = -1;
                    deltaTimeDrag = -1;
                    offTimeAfterDrag = -1;
                    if (std::abs(deltaX)>std::abs(deltaY))
                    {//Dragged first horizontally so it's a time changing drag
                        if (hoveringOver == HOVER_NOTEBAR || ModifierKeys::getCurrentModifiers().isCommandDown())
                        {
                            draggingOffTime = true;
                            noteBeingDraggedOn = hoverStep;
                        }
                        else
                        {
                            draggingTime = true;
                            noteBeingDraggedOn = hoverStep;
                        }
                        timeStartDrag = pSequence->at(hoverStep)->getTimeStamp();
                        offTimeStartDrag = pSequence->at(hoverStep)->getOffTime();
                    }
                    else if (hoveringOver != HOVER_NOTEBAR && std::abs(deltaX)<std::abs(deltaY))
                    {//Dragged first vertically so it's a velocity changing drag
                        if (!drawingVelocities.getValue() &&
                            !(marqueeAddingNotes||marqueeRemovingNotes||markingSelectedNotes||clearingSelectedNotes))
                        {
                            draggingVelocity = true;
                            velStartDrag = pSequence->at(hoverStep)->velocity;
                            noteBeingDraggedOn = hoverStep;
                            notesBeingDraggedOn.clear();
                            if (adjustingVelocities.getValue() && selectedNotes.contains(noteBeingDraggedOn))
                            {
                                notesBeingDraggedOn = selectedNotes;
                                int highestVelDragStep = INT_MIN;
                                int lowestVelDragStep = INT_MAX;
                                for (int i=0;i<selectedNotes.size();i++)
                                {
                                    if (notesBeingDraggedOn[i]>highestVelDragStep)
                                        highestVelDragStep = notesBeingDraggedOn[i];
                                    if (notesBeingDraggedOn[i]<lowestVelDragStep)
                                        lowestVelDragStep = notesBeingDraggedOn[i];
                                    if (processor->sequenceObject.theSequence.at(notesBeingDraggedOn[i])->inChord)
                                    {
                                        const int chordIndex = processor->sequenceObject.theSequence.at(notesBeingDraggedOn[i])->chordIndex;
                                        int iHighest = 0;
                                        for (int i=1;i<processor->sequenceObject.chords[chordIndex].notePointers.size();i++)
                                            if (processor->sequenceObject.chords[chordIndex].notePointers.at(i)->noteNumber >
                                                processor->sequenceObject.chords[chordIndex].notePointers.at(iHighest)->noteNumber)
                                                iHighest = i;
                                        if (notesBeingDraggedOn[i] == processor->sequenceObject.chords[chordIndex].notePointers[iHighest]->currentStep)
                                        {
                                            //If it's the chord's top step add all the chord's notes to steps to be changed
                                            for (int i=0;i<processor->sequenceObject.chords[chordIndex].notePointers.size();i++)
                                                notesBeingDraggedOn.addIfNotAlreadyThere(processor->sequenceObject.chords[chordIndex].notePointers.at(i)->currentStep);
                                            processor->sequenceObject.chords[chordIndex].chordTimeStamp += delta;
                                        }
                                    }
                                }
                                selectionLeftTick = processor->sequenceObject.theSequence.at(lowestVelDragStep)->getTimeStamp();
                                selectionWidthTicks = processor->sequenceObject.theSequence.at(highestVelDragStep)->getTimeStamp() -
                                        selectionLeftTick;
                                if (lowestVelDragStep==hoverStep)
                                    velocityDragType = VelocityDragLeftEnd;
                                else if (highestVelDragStep==hoverStep)
                                    velocityDragType = VelocityDragRightEnd;
                                else
                                    velocityDragType = VelocityDragMiddle;
                                if (selectionWidthTicks==0)
                                    velocityDragType = VelocityDragMiddle;
                            }
                            else
                            {
                                notesBeingDraggedOn.add(hoverStep);
                                velocityDragType = VelocityDragMiddle;
                            }
                            
                            velsStartDrag.clear();
                            for (int i=0;i<notesBeingDraggedOn.size();i++)
                                velsStartDrag.add(pSequence->at(notesBeingDraggedOn[i])->getVelocity());
                        }
                    }
                }
                if (draggingVelocity)
                {
                    velocityAfterDrag = std::min(1.0,velStartDrag + (deltaY/3.0)/127.0);
                    velocityAfterDrag = std::max((float)(1.001/127.0), velocityAfterDrag); //No less than midi velocity 1.0f
                    float ratio = velocityAfterDrag/velStartDrag;
                    for (int i=0;i<notesBeingDraggedOn.size();i++)
                    {
                        float adjustedRatio = ratio;
                        if (velocityDragType==VelocityDragRightEnd)
                            adjustedRatio = (1-ratio) * (1.0-(processor->sequenceObject.theSequence.at(notesBeingDraggedOn[i])->getTimeStamp()-selectionLeftTick)/selectionWidthTicks)+ratio;
                        else if (velocityDragType==VelocityDragLeftEnd)
                            adjustedRatio = (1-ratio) * ((processor->sequenceObject.theSequence.at(notesBeingDraggedOn[i])->getTimeStamp()-selectionLeftTick)/selectionWidthTicks)+ratio;
                        float newVel = velsStartDrag[i]*adjustedRatio;
                        if (newVel>1.0)
                            newVel=1.0;
                        else if (newVel<1.001/127.0)
                            newVel = 1.001/127.0;
                        processor->sequenceObject.theSequence.at(notesBeingDraggedOn[i])->velocity = newVel;
                    }
                    
//                    for (int i=0;i<notesBeingDraggedOn.size();i++)
//                        std::cout <<processor->sequenceObject.theSequence.at(notesBeingDraggedOn[i])->velocity<<" ";
//                    processor->catchUp();
                    processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits,
                                                 processor->getZTLTime(horizontalShift));
//   processor->buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, processor->getSequenceReadHead());
                    
                    repaint();
                }
                else if (draggingTime)
                {
                    timeAfterDrag = std::max(0.0,timeStartDrag - (deltaX/0.5));
                    timeAfterDrag = std::min(processor->sequenceObject.seqDurationInTicks, timeAfterDrag);
//                    std::cout << "deltaTimeDrag " << deltaTimeDrag  << "\n";
                    deltaTimeDrag = -(deltaX/0.5);
                    repaint();
                }
                else if (draggingOffTime)
                {
                    offTimeAfterDrag = std::max(1.0,offTimeStartDrag - (deltaX/0.5));
                    if (offTimeAfterDrag<timeStartDrag+1)
                        offTimeAfterDrag = timeStartDrag+1;
                    offTimeAfterDrag = std::min(processor->sequenceObject.seqDurationInTicks, offTimeAfterDrag);
//                    std::cout << "onTime, offTime " << timeAfterDrag <<" " << offTimeAfterDrag << "\n";
                    repaint();
                }
                else
                    ;//No movement yet
            }
        }
//        std::cout <<"Leaving timerCallback mouseeDrag"<<"\n";
    }
  } catch (const std::out_of_range& ex) {
      std::cout << " error noteviewer: timer callback " << "\n";
  }
}

void ScrollingNoteViewer::resized() 
{
    if (!grabbedInitialWindowSize)
    {
        ViewStateInfo::initialWidth = getWidth();
        processor->initialWindowHeight = getHeight ();
        grabbedInitialWindowSize = true;
    }
    ViewStateInfo::initialHeight = processor->initialWindowHeight;
    if (getWidth()>0)
        ViewStateInfo::viewWidth = getWidth();
    if (getHeight()>0)
    {
        ViewStateInfo::viewHeight = getHeight ();
        ViewStateInfo::verticalScale = (float)ViewStateInfo::viewHeight/ViewStateInfo::initialHeight;
    }

//    std::cout << "resized - ViewStateInfo::trackVerticalSize, verticalScale: before " << trackVerticalSize <<" "<<verticalScale;

    if (processor->sequenceObject.theSequence.size()>0)
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
 fixedSize="0" initialWidth="600" ViewStateInfo::initialHeight="400">
 <BACKGROUND backgroundColour="ffffffff"/>
 </JUCER_COMPONENT>
 
 END_JUCER_METADATA
 */
#endif
//[EndFile] You can add extra defines here...
//[/EndFile]

