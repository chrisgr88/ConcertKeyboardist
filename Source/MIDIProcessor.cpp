 /*
  ==============================================================================
  
  ==============================================================================
*/

#include "MIDIProcessor.h"

//==============================================================================
MIDIProcessor::MIDIProcessor() :
    noteOnOffFifo(FIFO_SIZE)
{
    sequenceObject.addChangeListener(this);
    undoMgr = new MyUndoManager();
    synthMessageCollectorIsReset = false;
    reset(44.1);
    isPlaying = false;
    isListening = false;
    startListenTime = -1;
    endListenTime = -1;
    lastStartTime = 0;
    waitingForFirstNote = false;
    resetViewer = true; //Cleared by NoteViewer after reset
    timerIntervalInMS = 1;
//    startTimer(timerIntervalInMS);
    panic = false;
    midiOutput = MidiOutput::createNewDevice("ConcertKeyboardist");
    notesEditable=true;
    MultiTimer::startTimer(TIMER_APP_ACTIVE, 1000);
//    addActionListener(Main);
}

MIDIProcessor::~MIDIProcessor()
{
    HighResolutionTimer::stopTimer();
    delete(midiOutput);
    undoMgr->clearUndoHistory();
    delete undoMgr;
}

void MIDIProcessor::changeListenerCallback (ChangeBroadcaster* broadcaster)
{
    if (broadcaster == &sequenceObject)
    {
//        std::cout << "MidiProcessor received change message from Sequence" << "\n";
        if (sequenceObject.loadDoc)
        {
            buildSequenceAsOf(Sequence::loadFile, Sequence::doNotRetainEdits, 0.0);
            sequenceObject.loadDoc = false;
        }
        else if (undoMgr->inUndo || undoMgr->inRedo)
        {
//            std::cout << "MidiProcessor received undo change message from Sequence" << "\n";
        }
    }
}

void MIDIProcessor::timerCallback (int timerID)
{
    if (timerID == TIMER_STOPLISTEN)
    {
        endListen();
        play(false,"current");
        MultiTimer::stopTimer(TIMER_STOPLISTEN);
    }
    if (timerID == TIMER_APP_ACTIVE)
    {
//        std::cout << "Active flag in MIDIProcessor " << appIsActive <<"\n";
        if (appIsActive)
            HighResolutionTimer::startTimer(timerIntervalInMS);
        else
            HighResolutionTimer::startTimer(100);
    }
}
void MIDIProcessor::play (bool ply, String fromWhere)
{
//    std::cout << "xInTicksFromViewer " << xInTicksFromViewer <<"\n";
    if (sequenceObject.theSequence.size()==0)
        return;
    if (!isPlaying && ply)
    {
        if (xInTicksFromViewer !=0)
            catchUp();
//        variableTempoRatio = 1.0;
        double now;
        if (fromWhere=="ZTL")
        {
            if (currentSeqStep+1<sequenceObject.theSequence.size())
                now = sequenceObject.theSequence[currentSeqStep+1]->getTimeStamp();
            else
                now = sequenceObject.theSequence[currentSeqStep]->getTimeStamp();
        }
        else if (fromWhere=="previousStart")
        {
            now = lastStartTime;
        }
        else if (fromWhere == "currentPlayhead")
        {
            if (lastPlayedSeqStep+1<sequenceObject.theSequence.size())
                now = sequenceObject.theSequence[lastPlayedSeqStep+1]->getTimeStamp();
            else
                now = sequenceObject.theSequence[0]->getTimeStamp();
        }
        else
            now = 0;
        rewind(now);
//        lastStartTime = now;
        
//        std::cout << "First note, timeStamp " << currentSeqStep+1 << " " << now<< "\n";
        setTimeInTicks(now);
//        accompTimeInTicks = now;
        changeMessageType = CHANGE_MESSAGE_START_PLAYING;
        sendSynchronousChangeMessage();
//        for (meas=0;meas<sequenceObject.measureTimes.size();meas++)
//        {
//            if (now < sequenceObject.measureTimes[meas])
//                break;
//        }
//        meas--;
//        changeMessageType = CHANGE_MESSAGE_MEASURE_CHANGED;
//        sendChangeMessage();
        if (sequenceObject.waitForFirstNote)
        {
            MidiBuffer midiMessages;
            removeNextBlockOfMessages(midiMessages, 50); //Clear input
            scheduledNotes.clear();
            waitingForFirstNote = true;
            HighResolutionTimer::startTimer(timerIntervalInMS);
        }
        for (int chan=0;chan<16;chan++)
        {
            if (sequenceObject.programChanges[chan]>=0)
            {
                MidiMessage prgCh = MidiMessage::programChange(chan+1, sequenceObject.programChanges[chan]);
//                std::cout << "pgCh " << sequenceObject.programChanges[chan] << " " << prgCh.getProgramChangeNumber() <<"\n";
                sendMidiMessage(prgCh);
            }
        }
        leadLag = 0;
    }
    isPlaying = ply;
    if (!isPlaying)
    {
        changeMessageType = CHANGE_MESSAGE_STOP_PLAYING;
        sendSynchronousChangeMessage();
        for (int chan=1;chan<=16;chan++)
        {
            MidiMessage allNotesOff = MidiMessage::controllerEvent(chan, 123, 0);
            sendMidiMessage(allNotesOff);
        }
        HighResolutionTimer::stopTimer();
        listenSequence.clear();
    }
    if (!isPlaying && isListening)
    {
        endListen();
    }
//    changeMessageType = CHANGE_MESSAGE_MEASURE_CHANGED;
    sendChangeMessage();
}

void MIDIProcessor::rewind (double time) //Rewind to given timeInTicks
{
//    std::cout << "Rewind: time " << time
//    << " lastPlayedSeqStep " << lastPlayedSeqStep
//    << "\n";
    
    listenStep = 0;
    if (listenSequence.size()>0)
    {
        while (time > listenSequence[listenStep].timeStamp)
            listenStep++;
    }
    timeIncrement =  sequenceObject.tempoMultiplier * 96.0*sequenceObject.getTempo(time)/60000.0;
    variableTimeIncrement = timeIncrement;
    leadLag = 0;
    changeMessageType = CHANGE_MESSAGE_NOTE_PLAYED;
    sendSynchronousChangeMessage(); //For some reason the Viewer receives this message twice! But seems to cause no problem.
//    std::vector<std::shared_ptr<NoteWithOffTime>> theSequence = sequenceObject.getSequence();
    HighResolutionTimer::stopTimer();
    panic = true;
    for (int chan=1;chan<=16;chan++)
    {
        MidiMessage controllersOff = MidiMessage::allControllersOff(chan);
        sendMidiMessage(controllersOff);
        MidiMessage allNotesOff = MidiMessage::controllerEvent(chan, 123, 0);
        sendMidiMessage(allNotesOff);
    }
//    isPlaying = false;
    autoPlaying = false;
    waitingForFirstNote = false;
    meas = 0; //Current measure, updated when timeInTicks passes next measure division
    currentBeat = 0;
    changeMessageType = CHANGE_MESSAGE_MEASURE_CHANGED;
    sendSynchronousChangeMessage();
    duplicateNote = -1;
    prevNoteOnLag = 0;
    prevTimeInTicks = 0;
//    sequenceObject.suppressSpeedAdjustment = false;
    sequenceObject.noteIsOn.clear();
    for (int i=0;i<(16*128);i++)
        sequenceObject.noteIsOn.push_back(false);
    for (int i=0; i<sequenceObject.theSequence.size();i++)
    {
        sequenceObject.theSequence.at(i)->noteOffNow = false;
        sequenceObject.theSequence.at(i)->sustaining = false;
        sequenceObject.theSequence.at(i)->selected = false;
    }
    noteOnOffFifo.reset();
    onNotes.clear();
    scheduledNotes.clear();
    
    int step = 0;
    if (time==0)
    {
        variableTempoRatio = 1.0;
        sequenceReadHead = 0;
        currentSeqStep = -1;
        lastPlayedNoteStep = 0;
        lastPlayedSeqStep = currentSeqStep;
        timeInTicks = 0;
        metTimeInTicks = 0;
        metronomeLighted = false;
        currentBeat = 0;
//        accompTimeInTicks = 0;
//        beatTickCounter = 0;
    }
    else //Set to position
    {
        for (step=0;step<sequenceObject.theSequence.size();step++)
        {
            if (sequenceObject.theSequence[step]->getTimeStamp()>=(time-0.001) && sequenceObject.theSequence[step]->triggeredBy==-1)
//            if (std::abs(sequenceObject.theSequence[step].timeStamp-time)<0.001 && sequenceObject.theSequence[step].triggeredBy==-1)
                break;
        }
        if (step==sequenceObject.theSequence.size())
            step = sequenceObject.theSequence.size() - 1;
        if (step==-1)
            step = 0;
        sequenceReadHead = sequenceObject.theSequence[step]->getTimeStamp();
        currentSeqStep = step-1;
        lastPlayedNoteStep = currentSeqStep;
        timeInTicks = time;
//        accompTimeInTicks = time;
    }
//    std::cout
//    << "In Rewind: time " << time
//    << " sequenceReadHead " << sequenceReadHead
//    << " currentSeqStep " << currentSeqStep
//    << " lastPlayedSeqStep " << lastPlayedSeqStep
//    << "\n";
    
//    startTimer(tempInterval);
    if (sequenceObject.autoPlaySustains)
    {
        int k=0; //This will be the value if there are no sustainPedalChanges.  i.e. One step past the the last, which is 0
        if (sequenceObject.sustainPedalChanges.size()>0)
        {
            for (k=0;k<sequenceObject.sustainPedalChanges.size();k++)
            {
    //            std::cout
    //            << " sustainPedalChange " << k
    //            << " at " << sequenceObject.sustainPedalChanges[k].timeStamp
    //            << " value " << sequenceObject.sustainPedalChanges[k].getControllerValue()
    //            << "\n";
                if (sequenceObject.sustainPedalChanges[k].getTimeStamp()>=time)
                    break;
            }
            //        if (k<sequenceObject.sustainPedalChanges.size())
            nextSustainStep = k; //Note that this could be past the end of sustainPedalChanges[ ] if there are no more events
            if(nextSustainStep-1>=0)
            {
                sendMidiMessage(sequenceObject.sustainPedalChanges[nextSustainStep-1]);
//                std::cout
//                << " send " << nextSustainStep-1
//                << " value " << sequenceObject.sustainPedalChanges[nextSustainStep-1].getControllerValue()
//                << "\n";
            }
        }
    }
    if (sequenceObject.autoPlaySofts)
    {
        int k=0; //This will be the value if there are no softPedalChanges.  i.e. One step past the the last, which is 0
        if (sequenceObject.softPedalChanges.size()>0)
        {
            for (k=0;k<sequenceObject.softPedalChanges.size();k++)
            {
//                std::cout
//                << " softPedalChange " << k
//                << " at " << sequenceObject.softPedalChanges[k].timeStamp
//                << " value " << sequenceObject.softPedalChanges[k].getControllerValue()
//                << "\n";
                if (sequenceObject.softPedalChanges[k].getTimeStamp()>=time)
                    break;
            }
            //        if (k<softPedalChanges())
            nextSoftStep = k; //Note that this could be past the end of softPedalChanges[ ] if there are no more events
            if(nextSoftStep-1>=0)
            {
                sendMidiMessage(sequenceObject.softPedalChanges[nextSoftStep-1]);
//                std::cout
//                << " send " << nextSoftStep-1
//                << " value " << sequenceObject.softPedalChanges[nextSoftStep-1].getControllerValue()
//                << "\n";
            }
        }
    }
    
    changeMessageType = CHANGE_MESSAGE_REWIND;
    sendSynchronousChangeMessage();
}

void MIDIProcessor::listenToSelection()
{
//    undoMgr->beginNewTransaction();
//    MIDIProcessor::ActionChain* action;
    if (copyOfSelectedNotes.size()>=0)
    {
//        if (copyOfSelectedNotes.size()==0)
//        {
            catchUp();
            startListenTime = sequenceObject.theSequence[currentSeqStep+1]->getTimeStamp();
            endListenTime =  sequenceObject.theSequence.back()->getTimeStamp();
//        }
//        else
//        {
//            startListenTime =  sequenceObject.theSequence[copyOfSelectedNotes[0]].timeStamp;
//            endListenTime =  sequenceObject.theSequence[copyOfSelectedNotes.getLast()].timeStamp;
//        }
        setListenSequence(startListenTime, endListenTime, Array<int>());
//        rewind(startListenTime);
        lastStartTime = startListenTime;
        play(true,"previousStart");
        isListening = true;
        MidiMessage msg = MidiMessage::noteOn(1, 60, (uint8)127);
        msg.setTimeStamp(99.0); //Value doesn't matter.
        msg.setChannel(16); //Channel 16 indicates notes from the computer keyboardk
        addMessageToQueue(msg);
    }
}

void::MIDIProcessor::endListen()
{
    isListening = false;
    rewind (startListenTime);
//    endListenTime = -1;
//    undoMgr->undo();
}

void MIDIProcessor::tweenMove (double targetTime, double transTime)
{
    tweenTo = targetTime;
    transitionTime = transTime;
    changeMessageType = CHANGE_MESSAGE_TWEEN;
    sendChangeMessage();
}

void MIDIProcessor::playableStepForwardBack(bool direction)
{
    play(false,"current");
    if (!direction)
    {
        for (int step=currentSeqStep; 0<=step ; step--)
            if (sequenceObject.theSequence[step]->triggeredBy == -1)
            {
//                rewind(sequenceObject.theSequence[step].timeStamp, true);
                tweenMove(sequenceObject.theSequence[step]->getTimeStamp(), 5.0);
                break;
            }
    }
    else
    {
        int nextStep;
        if (timeInTicks == sequenceObject.theSequence[currentSeqStep+1]->getTimeStamp())
            nextStep = currentSeqStep+2;
        else
            nextStep = currentSeqStep+1;
        for (int step=nextStep; step<sequenceObject.theSequence.size()-1; step++)
            if (sequenceObject.theSequence[step]->triggeredBy == -1)
            {
//                rewind(sequenceObject.theSequence[step].timeStamp, true);
                tweenMove(sequenceObject.theSequence[step]->getTimeStamp(), 5.0);
                break;
            }
    }
}

int MIDIProcessor::getMeasure(double horizontalShift)
{
    if (!isPlaying) //Compute measure based on horizontalShift, else meas was set when updating timeInTicks
    {
        double ztlTime;
        if (xInTicksFromViewer==0)
            ztlTime = getTimeInTicks();
        else
            ztlTime = getTimeInTicks()-xInTicksFromViewer;
        if (ztlTime<0.0)
            ztlTime = 0.0;
        for (meas=0;meas<sequenceObject.measureTimes.size();meas++)
        {
            if (ztlTime < sequenceObject.measureTimes[meas])
                break;
        }
        meas--;
//        if (meas==-1)
//            std::cout << "Should not be negative \n";
    }
    return meas+1;
}

void MIDIProcessor::measureForwardBack(bool direction)
{
    play(false,"current");
    int m;
    for (m=0; m<sequenceObject.measureTimes.size(); m++)
    {
        if (sequenceObject.measureTimes[m] >= timeInTicks)
            break;
    }
    currentMeasure = m;
    if (direction)
    {
        if (m<sequenceObject.measureTimes.size())
        {
            tweenMove(sequenceObject.measureTimes[m+1], 80);
        }
    }
    else
    {
        if (m>0)
        {
            tweenMove(sequenceObject.measureTimes[m-1], 80);
        }
    }
}
void MIDIProcessor::bookmarkForwardBack(bool direction)
{
    play(false,"current");
    double ztlTime = timeInTicks-xInTicksFromViewer;
//    std::cout << "compareTime " << timeInTicks-xInTicksFromViewer <<"\n";
    int b;
    if (ztlTime < sequenceObject.bookmarkTimes.getFirst())
    {
        if (direction)
            tweenMove(sequenceObject.bookmarkTimes.getFirst(), 300);
        else
            return;
    }
    else if (ztlTime > sequenceObject.bookmarkTimes.getLast())
    {
        if (!direction)
            tweenMove(sequenceObject.bookmarkTimes.getLast(), 300);
        else
            return;
    }
    else
    {
        for (b=1; b<sequenceObject.bookmarkTimes.size(); b++)
        {
            if (direction && sequenceObject.bookmarkTimes[b-1] <= ztlTime && ztlTime < sequenceObject.bookmarkTimes[b])
            {
//                rewind(sequenceObject.bookmarkTimes[b]);
                tweenMove(sequenceObject.bookmarkTimes[b], 300);
                return;
            }
            else if (!direction && sequenceObject.bookmarkTimes[b-1] < ztlTime && ztlTime <= sequenceObject.bookmarkTimes[b])
            {
//                rewind(sequenceObject.bookmarkTimes[b-1]);
                tweenMove(sequenceObject.bookmarkTimes[b-1], 300);
                return;
            }
        }
    }
}

bool MIDIProcessor::atZTL()
{
    return (fabs(xInTicksFromViewer)<4.0);
}

double MIDIProcessor::atBookmark()
{
    int bookmark = -1;
    for (int i=0;i<sequenceObject.bookmarkTimes.size();i++)
    {
        //                std::cout << "Bookmark at " << processor->sequenceObject.bookmarkTimes[i] << "\n";
        if (fabs(sequenceObject.bookmarkTimes[i]+xInTicksFromViewer-getTimeInTicks())<4.0)
        {
            bookmark = i;
            return sequenceObject.bookmarkTimes[i];
        }
    }
    return -1;
}

void MIDIProcessor::addRemoveBookmark (int action)
{
    int bookmark = -1;
    for (int i=0;i<sequenceObject.bookmarkTimes.size();i++)
    {
        //                std::cout << "Bookmark at " << processor->sequenceObject.bookmarkTimes[i] << "\n";
        if (fabs(sequenceObject.bookmarkTimes[i]+xInTicksFromViewer-getTimeInTicks())<4.0)
        {
            bookmark = i;
            break;
        }
    }
    if (bookmark!=-1 && (action==BOOKMARK_REMOVE || action==BOOKMARK_TOGGLE))
    {
        sequenceObject.bookmarkTimes.remove(bookmark);
    }
    else if (action==BOOKMARK_ADD || action==BOOKMARK_TOGGLE)
    {
        sequenceObject.bookmarkTimes.add(getTimeInTicks());
        sequenceObject.bookmarkTimes.sort();
        //                processor->sequenceObject.setChangedFlag(true);
        buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getTimeInTicks());
    }
    //            processor->sequenceObject.setChangedFlag(true);
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getTimeInTicks());
}

//<#processBlock#>
void MIDIProcessor::processBlock ()
{
//    std::vector<NoteWithOffTime*> * theSequence;
    if (sequenceObject.loadingFile)
        return;
    if (panic)
    {
        for (int chan=1;chan<=16;chan++)
        {
            MidiMessage allNotesOff = MidiMessage::controllerEvent(chan, 123, 0);
            sendMidiMessage(allNotesOff);
        }
        panic = false;
    }
    if (isPlaying)
    {
//        ttheSequence = sequenceObject.getSequence();
        if (!waitingForFirstNote)
        {
            timeInTicks += variableTimeIncrement;
            if (isListening && timeInTicks>endListenTime)
            {
//                std::cout << "end listening " <<"\n";
                MultiTimer::startTimer(TIMER_STOPLISTEN, 1);
            }
            if (timeInTicks<=sequenceObject.seqDurationInTicks)
            {
                if (meas < sequenceObject.measureTimes.size() && timeInTicks >= sequenceObject.measureTimes[meas+1])
                {
                    meas++;
//                    currentMeasure = meas;  //TODO Can we unify "meas" and currentMeasure into a single variable?
//                    std::cout << "timeInTicks, sequenceObject.measureTimes[meas+1] " << timeInTicks
//                    <<", "<<sequenceObject.measureTimes[meas+1]<< " "<< meas<<"\n";
                    changeMessageType = CHANGE_MESSAGE_MEASURE_CHANGED;
                    sendChangeMessage(); //For some reason the Viewer receives this message twice! But seems to cause no problem.
                }
            }
        }
//        const double tempo = sequenceObject.getTempo(sequenceObject.theSequence[currentSeqStep+1]timeStamp);
        if (lastPlayedNoteStep>=0)
        {
            const double tempo = sequenceObject.getTempo(sequenceObject.theSequence[lastPlayedNoteStep]->getTimeStamp());
            timeIncrement = sequenceObject.tempoMultiplier * tempo / 625;
            variableTimeIncrement = variableTempoRatio * sequenceObject.tempoMultiplier * tempo / 625;
//            std::cout << "timeInTicks, timeIncrement,  " <<lastPlayedNoteStep<<" "<< meas<<" "<<tempo<<" "<<timeInTicks
//            <<", "<<timeIncrement<< ", "<< variableTimeIncrement<<"\n";
        }
//        if (metTimeInTicks > sequenceObject.getPPQ())
//        {
////            std::cout << "BEAT " << "\n";
//            metronomeLighted = true;
////            changeMessageType = CHANGE_MESSAGE_BEAT_CHANGED;
////            sendChangeMessage(); //For some reason the Viewer receives this message twice! But seems to cause no problem.
//            metTimeInTicks = 0;
////            MidiMessage noteOn = MidiMessage::noteOn(16, 40, (uint8) 100);
////            sendMidiMessage(noteOn);
//        }
//        else if (metronomeLighted)
//        {
//            if (metTimeInTicks > sequenceObject.getPPQ()/32.0) //Turn off indicator after a 32nd note duration
//            {
////                std::cout << "BEAT OFF" << "\n";
//                metronomeLighted = false;
//                sendChangeMessage();
////                MidiMessage noteOff = MidiMessage::noteOff(2, 40, (uint8) 0);
////                sendMidiMessage(noteOff);
//            }
//            metTimeInTicks += timeIncrement;
//        }
//        else
//            metTimeInTicks += timeIncrement;
    }
    else
        return;
    
    if (isListening && listenSequence.size()>0)
    {
        while (timeInTicks > listenSequence[listenStep].timeStamp)
        {
            MidiMessage note = MidiMessage::noteOn(listenSequence[listenStep].channel,
                                                      listenSequence[listenStep].noteNumber,
                                                     listenSequence[listenStep].velocity);
//            std::cout << listenStep
//            << " Autoplay note " << listenSequence[listenStep].getNoteNumber()
//            << " Velocity " << (int) listenSequence[listenStep].getVelocity()
//            << "\n";
            sendMidiMessage(note);
            listenStep++;
            lastPlayedNoteStep = listenSequence[listenStep].triggeredBy;
        }
    }
    
    MidiBuffer midiMessages;
    removeNextBlockOfMessages(midiMessages, 50);
    int samplePosition=0;
    MidiMessage msg;
//    if (midiMessages.getNumEvents()>0)
//    {
////        std::cout << "removeNextBlockOfMessages. Count = " << midiMessages.getNumEvents() << "\n";
//        for (MidiBuffer::Iterator it (midiMessages); it.getNextEvent (msg, samplePosition);)
//        {
//            if (msg.isNoteOn())
//                ;//std::cout << "NoteOn " << (int)msg.getNoteNumber() <<" "<<(int)msg.getVelocity()<<" "<<msg.getChannel()<< "\n";
//            else if (msg.isNoteOff())
//                ;//std::cout << "NoteOff " << (int)msg.getNoteNumber() <<" "<<(int)msg.getVelocity()<<" "<<msg.getChannel()<< "\n";
//        }
//    }
//    const int64 tis = playHeadInfo.timeInSamples;
//    if (tis==0 && prevTis!=0)
//        rewind(0);
    //------------------------------------------------------------------
    //Put expression control events in expr array
    if (midiMessages.getNumEvents() > 0)
    {
        for (MidiBuffer::Iterator it (midiMessages); it.getNextEvent (msg, samplePosition);) //Get next event into msg
            exprEvents.add(msg);
    }
    midiMessages.clear();
    
    
    
    //###------------------------------------------------------------------
    //Turn off onNotes that are due to turn off
    if (onNotes.size()>0)
    {
//        std::cout << onNotes.size() << " onNotes ";
//        for (int nt=0;nt<onNotes.size();nt++)
//            std::cout <<" ["<<nt<<","<<theSequence->at(onNotes[nt]).getChannel()<<","<<theSequence->at(onNotes[nt]).getChannel()<<"]";
//        std::cout << "\n";
        
        Array<int> deHighlightSteps;
        for (int onNoteIndex=onNotes.size()-1;onNoteIndex>=0;onNoteIndex--)
        {
//            std::cout << "noteOffs: " <<timeInTicks;
            const int step = onNotes[onNoteIndex];
            if (sequenceObject.theSequence.size()<step) //This should never happen but testing this may prevent a crash
            {
                onNotes.remove(onNoteIndex);
                continue;
            }
            //Note offs for triggered notes
            if (sequenceObject.theSequence.at(step)->noteOffNow)
            {
//                MidiMessage noteOff = MidiMessage::noteOn(sequenceObject.theSequence.at(step)->channel,sequenceObject.theSequence.at(step)->noteNumber,(uint8) 0);
//                sendMidiMessage(noteOff);
//                sequenceObject.setNoteActive(sequenceObject.theSequence.at(step)->noteNumber, sequenceObject.theSequence.at(step)->channel, false);
//                onNotes.remove(onNoteIndex);
//                deHighlightSteps.add(-(step+1)); //Negative steps in queue will be dehighlighted by viewer
                //60000.0*timeIncrement/(sequenceObject.getTempo(time)*sequenceObject.tempoMultiplier) =  96.0;
                int nextNoteOn = -1;
                if (step<sequenceObject.theSequence.size())
                {
                    for (int st=step;st<sequenceObject.theSequence.size();st++)
                    {
                        if(sequenceObject.theSequence[st]->getTimeStamp()>sequenceObject.theSequence[step]->getTimeStamp())
                        {
                            nextNoteOn=st;
                            break;
                        }
                    }
                }
                if (nextNoteOn==-1 || sequenceObject.theSequence[nextNoteOn]->getTimeStamp()>timeInTicks)
                { //No need for legato so do noteoff now
                    MidiMessage noteOff = MidiMessage::noteOn(sequenceObject.theSequence.at(step)->channel,sequenceObject.theSequence.at(step)->noteNumber,(uint8) 0);
                    sendMidiMessage(noteOff);
                    sequenceObject.setNoteActive(sequenceObject.theSequence.at(step)->noteNumber, sequenceObject.theSequence.at(step)->channel, false);
                    onNotes.remove(onNoteIndex);
                    deHighlightSteps.add(-(step+1)); //Negative steps in queue will be dehighlighted by viewer
                }
                else
                { //Add a note overlap time to the current time and schedule the noteoff for then
                    std::cout<<"delayed off at "<<step<<" nextNoteOn "<<nextNoteOn<<"\n";
                    const double keyOverlapTimeMs = 100;
                    const double msPerTick = (60000.0/sequenceObject.getTempo(timeInTicks))/96.0;
                    const double keyOverlapTimeTicks = keyOverlapTimeMs/msPerTick;
                    sequenceObject.theSequence.at(step)->noteOffNow = false;
                    sequenceObject.theSequence.at(step)->sustaining = true;
                    const double newOffTime = timeInTicks+keyOverlapTimeTicks;
                    if (newOffTime<sequenceObject.getSeqDurationInTicks())
                        sequenceObject.theSequence.at(step)->scheduledOffTime = timeInTicks+keyOverlapTimeTicks;
                }
            }
            else if ((sequenceObject.theSequence.at(step)->autoplayedNote || sequenceObject.theSequence.at(step)->sustaining) && sequenceObject.theSequence.at(step)->scheduledOffTime <= timeInTicks)
            {
                MidiMessage noteOff = MidiMessage::noteOn(sequenceObject.theSequence.at(step)->channel,
                                                          sequenceObject.theSequence.at(step)->noteNumber,(uint8) 0);
                sendMidiMessage(noteOff);
                sequenceObject.setNoteActive(sequenceObject.theSequence.at(step)->noteNumber,sequenceObject.theSequence.at(step)->channel, false);
                onNotes.remove(onNoteIndex);
                deHighlightSteps.add(-(step+1)); //Negative steps in queue will be dehighlighted by scrollingNoteViewer
            }
        }
//        std::cout << "\n";
        int start1, size1, start2, size2; //Put in fifo for sending to note viewer
        noteOnOffFifo.prepareToWrite (deHighlightSteps.size(), start1, size1, start2, size2);
        int n = 0;
        for (int i = 0; i < size1; ++i)
            noteOnOffFifoBuffer [start1 + i] = deHighlightSteps[n++];
        for (int i = 0; i < size2; ++i)
            noteOnOffFifoBuffer [start2 + i] = deHighlightSteps[n++];
        noteOnOffFifo.finishedWrite (size1 + size2);
    }
    
    if (sequenceObject.autoPlaySustains)
    {
        if(nextSustainStep<sequenceObject.sustainPedalChanges.size() &&
              sequenceObject.sustainPedalChanges[nextSustainStep].getTimeStamp() < timeInTicks+leadLag)
        {
            sendMidiMessage(sequenceObject.sustainPedalChanges[nextSustainStep]);
//            std::cout
//            << " send " << nextSustainStep
//            << " value " << sequenceObject.sustainPedalChanges[nextSustainStep].getControllerValue()
//            << "\n";
            nextSustainStep++;
        }
    }
    if (sequenceObject.autoPlaySofts)
    {
        if(nextSoftStep<sequenceObject.softPedalChanges.size() &&
           sequenceObject.softPedalChanges[nextSoftStep].getTimeStamp() < timeInTicks+leadLag)
        {
            sendMidiMessage(sequenceObject.softPedalChanges[nextSoftStep]);
//            std::cout
//            << " send " << nextSoftStep
//            << " value " << sequenceObject.softPedalChanges[nextSoftStep].getControllerValue()
//            << "\n";
            nextSoftStep++;
        }
    }
    
    //###Turn on scheduledNotes due to turn on
    if (scheduledNotes.size() > 0)
    {
//        std::cout << "noteOns: " <<timeInTicks;
        Array<int> highlightSteps;
        while (scheduledNotes.size()>0)
        {
            const int step = scheduledNotes.front();
            lastPlayedNoteStep = step;
            if (sequenceObject.theSequence.size()<step) //This should never happen but testing this may prevent a crash
            {
                scheduledNotes.pop_front();
                continue;
            }
            const double schedTime = sequenceObject.theSequence.at(step)->scheduledOnTime;
            if (schedTime > timeInTicks)
            {
                //                std::cout << timeInTicks << " SKIP NOTE-ONS" <<" "<<theSequence->at(step).scheduledOnTime<<"\n";
                break;
            }

            if (sequenceObject.isNoteActive(sequenceObject.theSequence.at(step)->noteNumber,sequenceObject.theSequence.at(step)->channel))
            {
                //Do an noteOff of this note and remove it from onNotes
//                std::cout << timeInTicks << " Repeated note forced note off\n";
                //Do note-offs for all sequence notes in onNotes that were triggered by this expr noteOn, and were not sustained
                //The onNotes time stamp **ACTUALLY CONTAINS THE NOTE NUMBER OF THE EXPR EVENT** that started this onNote
                //..so turn off all onNotes that this expr note-off is related to.
                int stepToTurnOff = -1;
                for (int i=0;i<onNotes.size();i++)
                {
//                    std::cout << "channels " << theSequence->at(onNotes[i]).getChannel()
//                    <<" "<< theSequence->at(step).getChannel()<<"\n";
                    if (sequenceObject.theSequence.at(onNotes[i])->noteNumber == sequenceObject.theSequence.at(step)->noteNumber
                        && sequenceObject.theSequence.at(onNotes[i])->channel == sequenceObject.theSequence.at(step)->channel)
                    {
                        stepToTurnOff = onNotes[i];
                        break;
                    }
                }
//                std::cout << " forceOff" << stepToTurnOff<< "\n";
                if (stepToTurnOff!=-1)
                {
                    
                    MidiMessage noteOff = MidiMessage::noteOn(sequenceObject.theSequence.at(stepToTurnOff)->channel,
                                                              sequenceObject.theSequence.at(stepToTurnOff)->noteNumber, (uint8) 0);
//                    String note = MidiMessage::getMidiNoteName (theSequence->at(stepToTurnOff).getNoteNumber(), true, true, 3);
//                    std::cout << timeInTicks << " Repeated note forced noteOff " << step << " " << note
//                    << " triggeredBy "<<theSequence->at(stepToTurnOff).triggeredBy
//                    << " timeStamp "<<theSequence->at(stepToTurnOff).timeStamp
//                    << " firstInChain "<<theSequence->at(stepToTurnOff).firstInChain
//                    << " firstInChain TS "<<theSequence->at(theSequence->at(stepToTurnOff).firstInChain).timeStamp
//                    << "\n";
//                    midiMessages.addEvent(noteOff,0);
//                    midiOutput->sendMessageNow(noteOff);
//                    noteOff.setTimeStamp(99.0);
//                    synthMessageCollector.addMessageToQueue (noteOff);
                    sendMidiMessage(noteOff);
                    sequenceObject.setNoteActive(sequenceObject.theSequence.at(step)->noteNumber,
                                                 sequenceObject.theSequence.at(step)->channel, false);
                    const int index = onNotes.indexOf(stepToTurnOff);
                    onNotes.remove(index);
                    Array<int> deHighlightSteps;
                    deHighlightSteps.add(-(stepToTurnOff+1)); //Negative steps in queue will be dehighlighted by viewer
                    int start1, size1, start2, size2; //Put in fifo for sending to note viewer
                    noteOnOffFifo.prepareToWrite (deHighlightSteps.size(), start1, size1, start2, size2);
                    int n = 0;
                    for (int i = 0; i < size1; ++i)
                        noteOnOffFifoBuffer [start1 + i] = deHighlightSteps[n++];
                    for (int i = 0; i < size2; ++i)
                        noteOnOffFifoBuffer [start2 + i] = deHighlightSteps[n++];
                    noteOnOffFifo.finishedWrite (size1 + size2);
                }
//                else
//                    assert(false);//Should not get to here
            }
            
//            String note = MidiMessage::getMidiNoteName (theSequence->at(step).getNoteNumber(), true, true, 3);
//            std::cout << timeInTicks << " noteOn " << step << " " << note
//            << " triggeredBy "<<theSequence->at(step).triggeredBy
//            << " timeStamp "<<theSequence->at(step).timeStamp
//            << " firstInChain "<<theSequence->at(step).firstInChain
//            << " firstInChain TS "<<theSequence->at(theSequence->at(step).firstInChain).timeStamp
//            << "\n";
            MidiMessage noteOn = MidiMessage::noteOn(sequenceObject.theSequence.at(step)->channel,
                                                    sequenceObject.theSequence.at(step)->noteNumber,
                                                    sequenceObject.theSequence.at(step)->adjustedVelocity);
//            std::cout << timeInTicks << " noteOn vel " << step <<" "<< noteOn.getFloatVelocity() <<"\n";
            noteOn.setTimeStamp(samplePosition);
            sendMidiMessage(noteOn);
//            String note = MidiMessage::getMidiNoteName (noteOn.getNoteNumber(), true, true, 3);
//            if(sequenceObject.isNoteActive(theSequence->at(step).getNoteNumber()))
//            {
//                std::cout << timeInTicks << " Note Already On! " << step <<" "<<note
//                <<" "<<theSequence->at(step).scheduledOffTime<<"\n";
//            }
            sequenceObject.setNoteActive(sequenceObject.theSequence.at(step)->noteNumber,
                                         sequenceObject.theSequence.at(step)->channel, true);
//            std::cout << timeInTicks << " Do the noteOn " << step <<" "<<note
//            <<" "<<theSequence.at(step).adjustedVelocity
//            <<" "<<theSequence.at(step).scheduledOffTime
//            <<"\n";
            const double duration = sequenceObject.theSequence.at(step)->offTime - sequenceObject.theSequence.at(step)->getTimeStamp();
            sequenceObject.theSequence.at(step)->scheduledOffTime = duration+timeInTicks;
            onNotes.add(step);
            highlightSteps.add(step+1);
            scheduledNotes.pop_front();
        }
//        std::cout << "\n";
        int start1, size1, start2, size2; //Put in fifo for sending to note viewer
        noteOnOffFifo.prepareToWrite (highlightSteps.size(), start1, size1, start2, size2);
        int n = 0;
        for (int i = 0; i < size1; ++i)
            noteOnOffFifoBuffer [start1 + i] = highlightSteps[n++];
        for (int i = 0; i < size2; ++i)
            noteOnOffFifoBuffer [start2 + i] = highlightSteps[n++];
        noteOnOffFifo.finishedWrite (size1 + size2);
    }
    else
    {
        autoPlaying = false;
    }
    
    //------------------------------------------------------------------
    //------------------------------------------------------------------
    //###Process next group of expr events.  Skip if none. <#Process next group of expr events#>
    bool skipProcessingTheseEvents = false;
    for (int exprEventIndex=0;exprEventIndex<exprEvents.size();exprEventIndex++)
    {
        if (exprEvents[exprEventIndex].isNoteOn())
        {
//                std::cout
//                << timeInTicks << " Received a note on"
//                <<" "<<exprEvents[exprEventIndex].getNoteNumber()<<"\n";
            if (waitingForFirstNote)
            {
//                std::cout << " Set lastStartTime " << timeInTicks <<"\n";
                lastStartTime = timeInTicks;
                waitingForFirstNote = false;
            }
            //Process noteOns
            Array<int> availableNotes;
            double nPrimaryNotes = 0;
            double sumPrimaryVel = 0;
            int noteIndex;
            double mostRecentNoteTime;

//                std::cout << "noteOn " << exprEvents[exprEventIndex].getNoteNumber() << "\n";
            //Add next "firstInChain" note to availableNotes
            for (noteIndex=currentSeqStep+1;noteIndex<sequenceObject.theSequence.size();noteIndex++)
            {
                if (sequenceObject.theSequence.at(noteIndex)->getTimeStamp()>=sequenceReadHead &&
                    sequenceObject.theSequence.at(noteIndex)->firstInChain == noteIndex)
                {
                    earliness = sequenceObject.theSequence.at(noteIndex)->getTimeStamp()-timeInTicks;
                    const int stepPlayed = sequenceObject.theSequence.at(currentSeqStep+1)->firstInChain;
                    lastUserPlayedSeqStep = stepPlayed;
                    const double noteTimeStamp = sequenceObject.theSequence.at(stepPlayed)->getTimeStamp();
                    changeMessageType = CHANGE_MESSAGE_NOTE_PLAYED;
                    sendChangeMessage(); //For some reason the Viewer receives this message twice! But seems to cause no problem.
                    double howEarlyIsAllowed;
                    if (autoPlaying)
                        howEarlyIsAllowed = 1000;//sequenceObject.notePlayWindowAutoplaying;
                    else
                        howEarlyIsAllowed = 9999999;
                        
                    if (earliness < howEarlyIsAllowed)
                    {
//                        std::cout << "scheduledNotes " << scheduledNotes.size() << "\n";
                        if (scheduledNotes.size()==0)
                            leadLag = noteTimeStamp - timeInTicks;
                        availableNotes.add(noteIndex); //This is the triggering note
                        mostRecentNoteTime = sequenceObject.theSequence.at(noteIndex)->getTimeStamp();
                        const double vel = sequenceObject.theSequence.at(noteIndex)->velocity;
//                        if (sequenceObject.isPrimaryTrack(theSequence->at(noteIndex)->track))
//                        {
                            nPrimaryNotes++;
                            sumPrimaryVel += vel;
//                        }
                    }
                    else
                        skipProcessingTheseEvents = true;//std::cout << "Ignore noteOn" << "\n";
                    break;
                }
            }
            if (skipProcessingTheseEvents)
                break;
            
            //Add the chain of notes that are autoTriggered by this note to availableNotes
            if (availableNotes.size()>0) //We added a note so there may be more
            {
                int triggeredStep = sequenceObject.theSequence.at(noteIndex)->triggers;
                while (triggeredStep < sequenceObject.theSequence.size() && (triggeredStep != -1))
                {
                    availableNotes.add(triggeredStep);
//                    if (sequenceObject.isPrimaryTrack(theSequence->at(triggeredStep)->track))
//                    {
                        nPrimaryNotes++;
                        sumPrimaryVel += sequenceObject.theSequence.at(triggeredStep)->velocity;
//                    }
                    triggeredStep = sequenceObject.theSequence.at(triggeredStep)->triggers;
                }
            }

            //Set sequenceReadHead to time stamp of next note to be played
            //----------------------------------------------------------------------
            if (sequenceObject.theSequence.size()>noteIndex)
            {
                if (availableNotes.size()>0)
                {
                    sequenceReadHead = sequenceObject.theSequence.at(noteIndex)->getTimeStamp()+1;
                    const double noteOnLag = mostRecentNoteTime-timeInTicks;
                    const double deltaNoteOnLag = noteOnLag - prevNoteOnLag;
                    prevNoteOnLag = noteOnLag;
//                    int latePlayAdjustmentWindow = 100; //To prevent big speed jumps if the user plays far too early
                    if (!autoPlaying)// && /*!sequenceObject.suppressSpeedAdjustment &&*/ (-noteOnLag)<sequenceObject.latePlayAdjustmentWindow)
                    {
                        float timeDelta = sequenceObject.kV*deltaNoteOnLag + (timeInTicks-prevTimeInTicks)*noteOnLag*sequenceObject.kX;
//                        std::cout
//                        << "noteOnLag " <<noteOnLag
//                        << " leadTimeInTicks " <<leadTimeInTicks
//                        << " timeDelta "<<timeDelta
//                        << " variableTimeIncrement "<<variableTimeIncrement
//                        << " speedCorrection "<<(variableTempoRatio-1)*0.05
//                        << " newDelta " << timeDelta-(variableTempoRatio-1)*0.05
//                        <<"\n";
//                        std::cout
//                        << "leadTimeInTicks " <<leadTimeInTicks
//                        << " noteOnLag " <<noteOnLag
//                        <<" deltaNoteOnLag "<<deltaNoteOnLag
//                        <<" timeDelta "<<timeDelta
//                        <<"\n";
                        
//                        if (leadTimeInTicks>-noteOnLag) //Prevent adjustment if play head is off left edge of view
//                        {
                            if (timeDelta<0) //Reduce adjustment for trailing time deltas
                                timeDelta = timeDelta * sequenceObject.leadLagAdjustmentFactor;

                        const double proposedTimeIncrement = variableTimeIncrement+timeDelta;
                        if (timeIncrement*sequenceObject.lowerTempoLimit < proposedTimeIncrement &&
                            proposedTimeIncrement < timeIncrement*sequenceObject.upperTempoLimit)
                        {
                            if (timeDelta+variableTimeIncrement>0.0) //Don't let speed go too negative or zero
                            {
//                                std::cout << "ADJUST\n" ;
                                variableTimeIncrement = variableTimeIncrement + timeDelta;
                            }
                            variableTempoRatio = variableTimeIncrement/timeIncrement;
//                            std::cout << "RTT " << getRealTimeTempo() << "\n" ;
                        }
                    }
//                sequenceObject.suppressSpeedAdjustment = autoPlaying; //The next note after autoplaying should not cause speed adjustment
                    prevTimeInTicks = timeInTicks;
                }
            }
            else //Last note
            {
                sequenceReadHead = INT_MAX;
            }

            currentSeqStep = availableNotes[availableNotes.size()-1];
            lastPlayedSeqStep = currentSeqStep;

            double lastScheduledNoteTime = -1;
            int lastScheduledNote = 0;
            if (autoPlaying)//scheduledNotes.size()>0)
            {
                lastScheduledNote = scheduledNotes.back();
                lastScheduledNoteTime = sequenceObject.theSequence.at(scheduledNotes.back())->scheduledOnTime;
//                std::cout << "scheduledNotes back, lastScheduledNoteTime  "
//                << scheduledNotes.back() << " "<<lastScheduledNoteTime << "\n";
                
                int start1, size1, start2, size2; //Put in fifo for sending to note viewer
                noteOnOffFifo.prepareToWrite (1, start1, size1, start2, size2);
                for (int i = 0; i < size1; ++i)
                    noteOnOffFifoBuffer [start1 + i] = -(1+availableNotes[0]);
                for (int i = 0; i < size2; ++i)
                    noteOnOffFifoBuffer [start2 + i] = -(1+availableNotes[0]);
                noteOnOffFifo.finishedWrite (size1 + size2);
            }
            
            //------------------------------------------------------------------------------
            //Schedule note-ons of the all notes in availableNotes
            for (int noteToStart = 0;noteToStart < availableNotes.size();noteToStart++)
            {
                const int step = availableNotes[noteToStart];
                float velocity;
               // std::cout << timeInTicks << "NoteOn Channel "<< exprEvents[exprEventIndex].getChannel() <<"\n";
                //The method of computing velocities is determined by the noteOn's midi channel
                if (exprEvents[exprEventIndex].getChannel() == 16) //Computer keyboard sends on channel 16
                {
                    //All velocities from the original sequence
                    velocity = sequenceObject.theSequence.at(availableNotes[noteToStart])->velocity;
                }
                else if (exprEvents[exprEventIndex].getChannel() <= 13)
                {
                    //Highest note in chain velocity is a blend of the expr and the score vel using exprVelToScoreVelRatio
                    //Lower vel notes in chain vel are proportioned from highest note's output vel
                    //The exprVelToScoreVelRatio is set by the "vr" command
                    double highVelInChain = sequenceObject.theSequence.at(availableNotes[noteToStart])->highestVelocityInChain;
                    float exprVel = exprEvents[exprEventIndex].getVelocity();
                    if (exprVel>=1.0f)
                        exprVel = exprVel/127.0;
                    double thisNoteOriginalVelocity = sequenceObject.theSequence.at(availableNotes[noteToStart])->velocity;

                    double proportionedVelocity = sequenceObject.exprVelToScoreVelRatio*exprVel
                                    + (1.0-sequenceObject.exprVelToScoreVelRatio)*thisNoteOriginalVelocity;
                    
                    if (exprVel<highVelInChain)
                        velocity = std::round((proportionedVelocity/highVelInChain) * thisNoteOriginalVelocity);
                    else
                        velocity = proportionedVelocity;
                }
//                else if (exprEvents[exprEventIndex].getChannel() == 14)
//                {
//                    double highVelInChain = theSequence->at(availableNotes[noteToStart]).highestVelocityInChain;
//                    double exprVel = exprEvents[exprEventIndex].getVelocity();
//                    double thisNoteOriginalVelocity = theSequence->at(availableNotes[noteToStart]).getVelocity();
//                    if (exprVel<highVelInChain)
//                        velocity = (exprVel/highVelInChain) * thisNoteOriginalVelocity;
//                    else
//                        velocity = thisNoteOriginalVelocity +
//                                    ((exprVel-highVelInChain)/(127-highVelInChain)) * (127-thisNoteOriginalVelocity);
//                }
                else if(exprEvents[exprEventIndex].getChannel() == 15) //All velocities equal expr velocity
                {
                    velocity = exprEvents[exprEventIndex].getVelocity(); //All velocities equal expr velocity
                    velocity = velocity/127.0;
                }
                else
                    assert(false);
                
                double scheduledOnTime;
                if (lastScheduledNoteTime>0)
                    scheduledOnTime = lastScheduledNoteTime +
                                    (sequenceObject.theSequence.at(step)->getTimeStamp() -
                                     sequenceObject.theSequence.at(lastScheduledNote)->getTimeStamp());
                else
                    scheduledOnTime = timeInTicks +
                                    (sequenceObject.theSequence.at(step)->getTimeStamp() -
                                    sequenceObject.theSequence.at(availableNotes[0])->getTimeStamp());
//                if (lastScheduledNoteTime>0)
//                    std::cout << "step, lastScheduledNoteTime " << step << " " <<lastScheduledNoteTime << "\n";
                sequenceObject.theSequence.at(step)->noteOffNow = false;
                sequenceObject.theSequence.at(step)->scheduledOnTime = scheduledOnTime;
                sequenceObject.theSequence.at(step)->adjustedVelocity = velocity;
                sequenceObject.theSequence.at(step)->scheduledOffTime = scheduledOnTime +
                     (sequenceObject.theSequence.at(step)->offTime-sequenceObject.theSequence.at(step)->getTimeStamp());
                if (sequenceObject.theSequence.at(step)->triggeredNote)
                    sequenceObject.theSequence.at(step)->triggeringExprNote = exprEvents[exprEventIndex].getNoteNumber();
                else
                    sequenceObject.theSequence.at(step)->triggeringExprNote = -1;
//                String note = MidiMessage::getMidiNoteName (theSequence->at(step).getNoteNumber(), true, true, 3);
//                    std::cout << timeInTicks << " scheduledNotes.push_back (at 1)"<< step <<" "<<note<<" "<<theSequence.at(step).scheduledOnTime<< "\n";
                if (sequenceObject.theSequence.at(step)->autoplayedNote)
                {
                    autoPlaying = true;
//                        std::cout << "autoplay on at " << step << "\n";
                }
                scheduledNotes.push_back(step);
            }
            
        }
        //------------------------------------------------------------------
        else if (exprEvents[exprEventIndex].isNoteOff())
        {
//                std::cout
//                << timeInTicks << " Received a note off\n"
//                <<" "<<exprEvents[exprEventIndex].getNoteNumber()<<"\n";
            //Do note-offs for all sequence notes in onNotes that were triggered by this expr noteOn, and have played at least their scheduled time
            for (int onNoteIndex=0;onNoteIndex<onNotes.size();onNoteIndex++)
            {
                //Turn off all onNotes that this expr note-off is related to.
                const int seqStep = onNotes[onNoteIndex];
                const int exprNoteThatStartedThisOnNote = sequenceObject.theSequence.at(seqStep)->triggeringExprNote;
//                String note = MidiMessage::getMidiNoteName (sequenceObject.theSequence[seqStep].getNoteNumber(), true, true, 3);
                if (exprNoteThatStartedThisOnNote == exprEvents[exprEventIndex].getNoteNumber())
                {
                    if (sequenceObject.theSequence.at(seqStep)->triggeredOffNote)
                    {
                        sequenceObject.theSequence.at(seqStep)->noteOffNow = true;
                    }
                    else
                        sequenceObject.theSequence.at(seqStep)->sustaining = true;
                }
            }
        }
        //------------------------------------------------------------------
        //Process other expr events that are not a note note-on or or note-off
        else
        {
            MidiMessage msg = exprEvents[exprEventIndex];
            if (msg.isController())
                std::cout << "Controller " << msg.getControllerNumber() << " " << msg.getControllerValue()<<"\n";
            sendMidiMessage(exprEvents[exprEventIndex]);
        }
    }
//    loopStartTick = 300.0;
//    loopEndStep = 50;
//    loopEndTickOffset = 50.0;  //Number of ticks past time stamp of loopEndStep that the time should be returned to loopStartTick.
//    loopEndTick = DBL_MAX;
//    if (currentSeqStep>=loopEndStep)
//    {
//        loopPending = true;
//        loopEndTick = theSequence.at(loopEndStep).scheduledOnTime + loopEndTickOffset;
//    }
//    if (timeInTicks>=loopEndTick)
//    {
//        std::cout << "Looped ****** timeInTicks " << timeInTicks << "\n";
//        rewind(loopStartTick, true);
//        isPlaying = true;
//        loopEndTick = DBL_MAX;
//        loopPending = false;
//    }
//    else
        exprEvents.clear();
}

void MIDIProcessor::catchUp()
{
    rewind(timeInTicks-xInTicksFromViewer);
}

void MIDIProcessor::hiResTimerCallback()
{
    processBlock();
}
