  /*
  ==============================================================================
  
  ==============================================================================
*/

#include "MIDIProcessor.h"

//==============================================================================
MIDIProcessor::MIDIProcessor() :
    noteOnOffFifo(FIFO_SIZE)
{
    pauseProcessing = false;
    sequenceObject.addChangeListener(this);
    undoMgr = new MyUndoManager(100000,50);
    pluginMessageCollectorIsReset = false;
    pluginMessageCollector = nullptr;
    reset(44.1);
    isPlaying = false;
    isListening = false;
    startListenTime = -1;
    endListenTime = -1;
    lastStartTime = 0;
    waitingForFirstNote = false;
    resetViewer = true; //Cleared by NoteViewer after reset
    timerIntervalInMS = 1;
    panic = false;
#if JUCE_MAC || JUCE_IOS
    ckMidiOutput = MidiOutput::createNewDevice("ConcertKeyboardist");
#endif
    notesEditable=true;
    MultiTimer::startTimer(TIMER_APP_ACTIVE, 1000);
}

MIDIProcessor::~MIDIProcessor()
{
    HighResolutionTimer::stopTimer();
#if JUCE_MAC || JUCE_IOS
    delete(ckMidiOutput);
#endif
    undoMgr->clearUndoHistory();
    delete undoMgr;
}

bool MIDIProcessor::getCurrentPosition (CurrentPositionInfo& result)
{
    if (timeInTicks<=0)
        result.bpm = sequenceObject.getTempo(0, sequenceObject.scaledTempoChanges) * variableTempoRatio;
    else
    {
        result.bpm = sequenceObject.getTempo(timeInTicks, sequenceObject.scaledTempoChanges) * variableTempoRatio;
    }
    result.timeSigNumerator = sequenceObject.numerator;
    result.timeSigDenominator = sequenceObject.denominator;
    result.timeInSamples =sampleRate*timerIntervalInMS*timeInTicks/1000.0;
//    std::cout << " timeInSamples " << result.timeInSamples << "\n";
    result.timeInSeconds = timerIntervalInMS*timeInTicks/1000.0;
    result.editOriginTime = 0.0;
    result.ppqPosition = variableTimeIncrement*timeInTicks;
    result.ppqPositionOfLastBarStart = 0;
//    result.frameRate = 0.0f;
    result.isPlaying = isPlaying;
    result.isRecording = false;
    result.ppqLoopStart = 0;
    result.ppqLoopEnd = 0;
    result.isLooping = false;
    return true;
}

void MIDIProcessor::changeListenerCallback (ChangeBroadcaster* broadcaster)
{
    if (broadcaster == &sequenceObject)
    {
//        std::cout << "MidiProcessor received change message from Sequence" << "\n";
        if (sequenceObject.loadDoc)
        {
//            pluginEnabled = false;
            midiOutEnabled = true;
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
        if (appIsActive)// && !HighResolutionTimer::isTimerRunning())
            HighResolutionTimer::startTimer(timerIntervalInMS);
        else if (HighResolutionTimer::isTimerRunning())
            HighResolutionTimer::stopTimer();
    }
}
void MIDIProcessor::play (bool ply, String fromWhere)
{
//    std::cout << "xInTicksFromViewer " << xInTicksFromViewer <<"\n";
    if (sequenceObject.theSequence.size()==0)
        return;
    bool stoppedPlaying = !ply && isPlaying;
//    if (!ply)
//         std::cout << "stopped playing " << timeInTicks <<"\n";
    if (!isPlaying && ply)
    {
        if (xInTicksFromViewer !=0)
            catchUp();
//        variableTempoRatio = 1.0;
        double now;
        if (fromWhere=="ZTL")
        {
            if (currentSeqStep+1<sequenceObject.theSequence.size())
                now = sequenceObject.theSequence.at(currentSeqStep+1)->getTimeStamp();
            else
                now = sequenceObject.theSequence.at(currentSeqStep)->getTimeStamp();
        }
        else if (fromWhere=="previousStart")
        {
            now = lastStartTime;
        }
        else if (fromWhere == "currentPlayhead")
        {
            if (lastPlayedSeqStep+1<sequenceObject.theSequence.size())
                now = sequenceObject.theSequence.at(lastPlayedSeqStep+1)->getTimeStamp();
            else
                now = sequenceObject.theSequence.at(0)->getTimeStamp();
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
        for (int i=0; i<onNotes.size(); i++)
        {
            const MidiMessage noteOff = MidiMessage::noteOff(sequenceObject.theSequence.at(onNotes[i])->channel,
                                                             sequenceObject.theSequence.at(onNotes[i])->noteNumber,
                                                             sequenceObject.theSequence.at(onNotes[i])->getVelocity());
            sendMidiMessage(noteOff);
        }
        if (stoppedPlaying)
        {
            for (int chan=1;chan<=16;chan++)
            {
                for (int i=0; i<onNotes.size(); i++)
                {
                    const MidiMessage noteOff = MidiMessage::noteOff(sequenceObject.theSequence.at(onNotes[i])->channel,
                                                                     sequenceObject.theSequence.at(onNotes[i])->noteNumber,
                                                                     0.0f);
                                    sendMidiMessage(noteOff);
                }
                MidiMessage allNotesOff = MidiMessage::controllerEvent(chan, 123, 0);
                sendMidiMessage(allNotesOff);
                const MidiMessage sustOff = MidiMessage::controllerEvent(chan, 64, 0);
                sendMidiMessage(sustOff);
                const MidiMessage softOff = MidiMessage::controllerEvent(chan, 67, 0);
                sendMidiMessage(softOff);
            }
            onNotes.clear();
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

void MIDIProcessor::rewind (double time, bool sendChangeMessages) //Rewind to given timeInTicks
{
//    std::cout << "Rewind: time " << time
//    << " lastPlayedSeqStep " << lastPlayedSeqStep
//    << "\n";
//    std::cout << "Rewind " << "\n";
    if (sequenceObject.theSequence.size()==0)
    {
        std::cout << "Failed rewind " << "\n";
        return;
    }
    try {
        pauseProcessing = true;
        listenStep = 0;
        lastPlayedSeqStep = -1;
        lastUserPlayedSeqStep = -1;
        lastPlayedNoteStep = -1;
        if (listenSequence.size()>0)
        {
            while (time > listenSequence.at(listenStep).timeStamp)
                listenStep++;
        }
        timeIncrement =  960.0*sequenceObject.getTempo(time, sequenceObject.scaledTempoChanges)/60000.0;
        variableTimeIncrement = timeIncrement;
        leadLag = 0;
        changeMessageType = CHANGE_MESSAGE_NOTE_PLAYED;
        if (sendChangeMessages)
            sendSynchronousChangeMessage(); //For some reason the Viewer receives this message twice! But seems to cause no problem.
        HighResolutionTimer::stopTimer();
        panic = true;
        isPlaying = false;
        autoPlaying = false;
        waitingForFirstNote = false;
        meas = 0; //Current measure, updated when timeInTicks passes next measure division
        currentBeat = 0;
        changeMessageType = CHANGE_MESSAGE_MEASURE_CHANGED;
        if (sendChangeMessages)
            sendSynchronousChangeMessage();
        duplicateNote = -1;
        prevNoteOnLag = 0;
        prevTimeInTicks = 0;
        sequenceObject.noteIsOn.clear();
        for (int i=0;i<(16*128);i++)
            sequenceObject.noteIsOn.push_back(false);
        for (int i=0; i<sequenceObject.theSequence.size();i++)
        {
            sequenceObject.theSequence.at(i)->noteOffNow = false;
            sequenceObject.theSequence.at(i)->sustaining = false;
        }
        noteOnOffFifo.reset();
        scheduledNotes.clear();
    } catch (const std::out_of_range& ex) {
        std::cout << " error 1 in rewind " << "\n";
    }
    
    try {
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
    //        std::cout << "rewind start Set to position" << "\n";
            for (step=0;step<sequenceObject.theSequence.size();step++)
            {
                if (sequenceObject.theSequence.at(step)->getTimeStamp()>=(time-0.001) &&
                    sequenceObject.theSequence.at(step)->targetNote)
                    break;
            }
            if (step>=sequenceObject.theSequence.size())
                step = (int)sequenceObject.theSequence.size() - 1;
            if (step==-1)
                step = 0;
    //        std::cout << "rewind Set to position: step =  " <<step<< "\n";

            sequenceReadHead = sequenceObject.theSequence.at(step)->getTimeStamp();
    //        std::cout << "rewind Set read head " <<"\n";
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
    //    std::cout << "rewind autoPlaySustains  " << "\n";
        if (sequenceObject.autoPlaySustains)
        {
            int k=0; //This will be the value if there are no sustainPedalChanges.  i.e. One step past the the last, which is 0
            if (sequenceObject.sustainPedalChanges.size()>0)
            {
                for (k=0;k<sequenceObject.sustainPedalChanges.size();k++)
                {
        //            std::cout
        //            << " sustainPedalChange " << k
        //            << " at " << sequenceObject.sustainPedalChanges.at(k).timeStamp
        //            << " value " << sequenceObject.sustainPedalChanges.at(k).getControllerValue()
        //            << "\n";
                    if (sequenceObject.sustainPedalChanges.at(k).timeStamp>=time)
                        break;
                }
                //        if (k<sequenceObject.sustainPedalChanges.size())
                nextSustainStep = k; //Note that this could be past the end of sustainPedalChanges[ ] if there are no more events
                if(nextSustainStep<sequenceObject.sustainPedalChanges.size() && nextSustainStep-1 >= 0)
                {
                    if (sequenceObject.sustainPedalChanges.at(k-1).pedalOn)
                    {
                        MidiMessage msgOn = MidiMessage::controllerEvent(1, 64, 127);
                        msgOn.setTimeStamp(sequenceObject.sustainPedalChanges.at(k).timeStamp);
                        sendMidiMessage(msgOn);
                    }
                    else
                    {
                        MidiMessage msgOff = MidiMessage::controllerEvent(1, 64, 0);
                        msgOff.setTimeStamp(sequenceObject.sustainPedalChanges.at(k).timeStamp);
                        sendMidiMessage(msgOff);
                    }
                }
            }
        }
    //    std::cout << "rewind autoPlaySofts  " << "\n";
            
        if (sequenceObject.autoPlaySofts)
        {
            int k=0; //This will be the value if there are no softPedalChanges.  i.e. One step past the the last, which is 0
            if (sequenceObject.softPedalChanges.size()>0)
            {
                for (k=0;k<sequenceObject.softPedalChanges.size();k++)
                {
    //                std::cout
    //                << " softPedalChange " << k
    //                << " at " << sequenceObject.softPedalChanges.at(k).timeStamp
    //                << " value " << sequenceObject.softPedalChanges.at(k).getControllerValue()
    //                << "\n";
                    if (sequenceObject.softPedalChanges.at(k).timeStamp>=time)
                        break;
                }
                //        if (k<softPedalChanges())
                nextSoftStep = k; //Note that this could be past the end of softPedalChanges[ ] if there are no more events
                if(nextSoftStep<sequenceObject.softPedalChanges.size() && nextSoftStep-1>=0)
                {
    //                sendMidiMessage(sequenceObject.softPedalChanges[nextSoftStep-1]);
    //                std::cout
    //                << " send " << nextSoftStep-1
    //                << " value " << sequenceObject.softPedalChanges.at(nextSoftStep-1).getControllerValue()
    //                << "\n";
                    if (sequenceObject.softPedalChanges.at(k-1).pedalOn)
                    {
                        MidiMessage msgOn = MidiMessage::controllerEvent(1, 67, 127);
                        msgOn.setTimeStamp(sequenceObject.softPedalChanges.at(k-1).timeStamp);
                        sendMidiMessage(msgOn);
                    }
                    else
                    {
                        MidiMessage msgOff = MidiMessage::controllerEvent(1, 67, 0);
                        msgOff.setTimeStamp(sequenceObject.softPedalChanges.at(k-1).timeStamp);
                        sendMidiMessage(msgOff);
                    }
                }
            }
        }
    } catch (const std::out_of_range& ex) {
        std::cout << " error 2 in rewind " << ex.what()<<"\n";
    }
    try {
        pauseProcessing = false;
        changeMessageType = CHANGE_MESSAGE_REWIND;
        if (sendChangeMessages)
            sendSynchronousChangeMessage();
    } catch (const std::out_of_range& ex) {
        std::cout << " error 3 in rewind " << "\n";
    }
//    std::cout << " leaving rewind " << "\n";
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
            startListenTime = sequenceObject.theSequence.at(currentSeqStep+1)->getTimeStamp();
            endListenTime =  sequenceObject.theSequence.back()->getTimeStamp();
//        }
//        else
//        {
//            startListenTime =  sequenceObject.theSequence.at(copyOfSelectedNotes.at(0)).timeStamp;
//            endListenTime =  sequenceObject.theSequence.at(copyOfSelectedNotes.getLast()).timeStamp;
//        }
        setListenSequence(startListenTime, endListenTime, Array<int>());
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
    rewind(startListenTime);
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
            if (sequenceObject.theSequence.at(step)->targetNote)
            {
                tweenMove(sequenceObject.theSequence.at(step)->getTimeStamp(), 5.0);
                break;
            }
    }
    else
    {
        int nextStep;
        if (timeInTicks == sequenceObject.theSequence.at(currentSeqStep+1)->getTimeStamp())
            nextStep = currentSeqStep+2;
        else
            nextStep = currentSeqStep+1;
        for (int step=nextStep; step<sequenceObject.theSequence.size()-1; step++)
            if (sequenceObject.theSequence.at(step)->targetNote)
            {
                tweenMove(sequenceObject.theSequence.at(step)->getTimeStamp(), 5.0);
                break;
            }
    }
    sendChangeMessage();
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

int MIDIProcessor::getZTLTime(double horizontalShift)
{
//    if (!isPlaying) //Compute measure based on horizontalShift, else meas was set when updating timeInTicks
//    {
        double ztlTime;
        if (xInTicksFromViewer==0)
            ztlTime = getTimeInTicks();
        else
            ztlTime = getTimeInTicks()-xInTicksFromViewer;
        if (ztlTime<0.0)
            ztlTime = 0.0;    //std::cout << "Should not be negative \n";
//    }
    return ztlTime;
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
    sendChangeMessage();
}
void MIDIProcessor::bookmarkForwardBack(bool direction)
{
    play(false,"current");
    double ztlTime = timeInTicks-xInTicksFromViewer;
//    std::cout << "compareTime " << timeInTicks-xInTicksFromViewer <<"\n";
    int b;
    if (ztlTime < sequenceObject.bookmarkTimes.getFirst().time)
    {
        if (direction)
            tweenMove(sequenceObject.bookmarkTimes.getFirst().time, 300);
        else
            return;
    }
    else if (ztlTime > sequenceObject.bookmarkTimes.getLast().time)
    {
        if (!direction)
            tweenMove(sequenceObject.bookmarkTimes.getLast().time, 300);
        else
            return;
    }
    else
    {
        for (b=1; b<sequenceObject.bookmarkTimes.size(); b++)
        {
            if (direction && sequenceObject.bookmarkTimes[b-1].time <= ztlTime && ztlTime < sequenceObject.bookmarkTimes[b].time)
            {
                tweenMove(sequenceObject.bookmarkTimes[b].time, 300);
                return;
            }
            else if (!direction && sequenceObject.bookmarkTimes[b-1].time < ztlTime && ztlTime <= sequenceObject.bookmarkTimes[b].time)
            {
                tweenMove(sequenceObject.bookmarkTimes[b-1].time, 300);
                return;
            }
        }
    }
    sendChangeMessage();
}

bool MIDIProcessor::atZTL()
{
    return (fabs(xInTicksFromViewer)<4.0);
}

Sequence::Bookmark MIDIProcessor::atBookmark()
{
//    int bookmark = -1;
    for (int i=0;i<sequenceObject.bookmarkTimes.size();i++)
    {
        //                std::cout << "Bookmark at " << processor->sequenceObject.bookmarkTimes[i] << "\n";
        if (fabs(sequenceObject.bookmarkTimes[i].time+xInTicksFromViewer-getTimeInTicks())<40)
        {
//            bookmark = i;
            return sequenceObject.bookmarkTimes[i];
        }
    }
    return Sequence::Bookmark();
}

void MIDIProcessor::addRemoveBookmark (int action, bool tempoChange, double tempoScale)
{
    int bookmark = -1;
    for (int i=0;i<sequenceObject.bookmarkTimes.size();i++)
    {
        //                std::cout << "Bookmark at " << processor->sequenceObject.bookmarkTimes[i] << "\n";
        if (fabs(sequenceObject.bookmarkTimes[i].time+xInTicksFromViewer-getTimeInTicks())<4.0)
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
        Sequence::Bookmark bm;
        bm.time = getTimeInTicks();
        bm.tempoChange = tempoChange;
        bm.tempoScaleFactor = tempoScale;
        sequenceObject.bookmarkTimes.add(bm);
        sequenceObject.bookmarkTimes.sort();
        buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getTimeInTicks());
    }
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getTimeInTicks());
}

//<#processBlock#>
//###
void MIDIProcessor::processBlock ()
{
//    std::vector<NoteWithOffTime*> * theSequence;
    try {
    if (pauseProcessing || sequenceObject.loadingFile || sequenceObject.theSequence.size()==0)
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
//        if (lastPlayedNoteStep>=0)
//        {
////            const double tempo = sequenceObject.getTempo(sequenceObject.theSequence.at(lastPlayedNoteStep)->getTimeStamp(),
////                                                         sequenceObject.scaledTempoChanges);
//            const double tempo = sequenceObject.getTempo(timeInTicks+leadLag,
//                                                         sequenceObject.scaledTempoChanges);
//            std::cout << "timeInTicks, leadLag,  " <<timeInTicks<<" "<< leadLag<<" "<<timeInTicks+leadLag<<" "<<tempo<<"\n";
//            timeIncrement = 10.0*tempo / 625;
//            variableTimeIncrement = 10*variableTempoRatio * tempo / 625;
////            std::cout << "timeInTicks, timeIncrement,  " <<lastPlayedNoteStep<<" "<< meas<<" "<<tempo<<" "<<timeInTicks
////            <<", "<<timeIncrement<< ", "<< variableTimeIncrement<<"\n";
//        }
        
        const double tempo = sequenceObject.getTempo(timeInTicks+leadLag,sequenceObject.scaledTempoChanges);
        timeIncrement = 10.0*tempo / 625;
        variableTimeIncrement = 10*variableTempoRatio * tempo / 625;
    }
    else
    {
        pauseProcessing = false;
        return;
    }
    
    if (isListening && listenSequence.size()>0)
    {
        while (timeInTicks > listenSequence.at(listenStep).timeStamp)
        {
            MidiMessage note = MidiMessage::noteOn(listenSequence.at(listenStep).channel,
                                                      listenSequence.at(listenStep).noteNumber,
                                                     listenSequence.at(listenStep).velocity);
//            std::cout << listenStep
//            << " Autoplay note " << listenSequence[listenStep].getNoteNumber()
//            << " Velocity " << (int) listenSequence[listenStep].getVelocity()
//            << "\n";
            sendMidiMessage(note);
            listenStep++;
            lastPlayedNoteStep = listenSequence.at(listenStep).triggeredBy;
        }
    }
    
    MidiBuffer midiMessages;
    removeNextBlockOfMessages(midiMessages, 50);
    int samplePosition=0;
    MidiMessage msg;
    //------------------------------------------------------------------
    //Put expression control events in expr array
    if (midiMessages.getNumEvents() > 0)
    {
        for (MidiBuffer::Iterator it (midiMessages); it.getNextEvent (msg, samplePosition);) //Get next event into msg
        {
            exprEvents.add(msg);
//            std::cout<< "nn "<< (int) msg.getNoteNumber()<<"\n";
        }
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
                int nextNoteOn = -1;
                if (step<sequenceObject.theSequence.size())
                {
                    for (int st=step;st<sequenceObject.theSequence.size();st++)
                    {
                        if(sequenceObject.theSequence.at(st)->getTimeStamp()>sequenceObject.theSequence.at(step)->getTimeStamp())
                        {
                            nextNoteOn=st;
                            break;
                        }
                    }
                }
                if (nextNoteOn==-1 || sequenceObject.theSequence.at(nextNoteOn)->getTimeStamp()>timeInTicks)
                { //No need for legato so do noteoff now
                    MidiMessage noteOff = MidiMessage::noteOn(sequenceObject.theSequence.at(step)->channel,sequenceObject.theSequence.at(step)->noteNumber,(uint8) 0);
//                    std::cout<<"at 1 noteOff "<<step<<"\n";
                    sendMidiMessage(noteOff);
                    sequenceObject.setNoteActive(sequenceObject.theSequence.at(step)->noteNumber, sequenceObject.theSequence.at(step)->channel, false);
                    onNotes.remove(onNoteIndex);
                    deHighlightSteps.add(-(step+1)); //Negative steps in queue will be dehighlighted by viewer
                }
                else
                { //Add a note overlap time to the current time and schedule the noteoff for then
//                    std::cout<<"delayed off at "<<step<<" nextNoteOn "<<nextNoteOn<<"\n";
                    const double keyOverlapTimeMs = 100;
                    const double msPerTick = (60000.0/sequenceObject.getTempo(timeInTicks, sequenceObject.scaledTempoChanges))/960.0;
                    const double keyOverlapTimeTicks = keyOverlapTimeMs/msPerTick;
                    sequenceObject.theSequence.at(step)->noteOffNow = false;
                    sequenceObject.theSequence.at(step)->sustaining = true;
                    const double newOffTime = timeInTicks+keyOverlapTimeTicks;
                    if (newOffTime<=sequenceObject.getSeqDurationInTicks())
                        sequenceObject.theSequence.at(step)->scheduledOffTime = timeInTicks+keyOverlapTimeTicks;
                }
            }
            else if ((sequenceObject.theSequence.at(step)->autoplayedNote || sequenceObject.theSequence.at(step)->sustaining) && sequenceObject.theSequence.at(step)->scheduledOffTime <= timeInTicks)
            {
                MidiMessage noteOff = MidiMessage::noteOn(sequenceObject.theSequence.at(step)->channel,
                                                          sequenceObject.theSequence.at(step)->noteNumber,(uint8) 0);
//                std::cout<<"at 2 noteOff "<<step<<"\n";
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
              sequenceObject.sustainPedalChanges.at(nextSustainStep).timeStamp < timeInTicks+leadLag)
        {
            if (sequenceObject.sustainPedalChanges.at(nextSustainStep).pedalOn)
            {
                MidiMessage msgOn = MidiMessage::controllerEvent(1, 64, 127);
                msgOn.setTimeStamp(sequenceObject.sustainPedalChanges.at(nextSustainStep).timeStamp);
                sendMidiMessage(msgOn);
            }
            else
            {
                MidiMessage msgOff = MidiMessage::controllerEvent(1, 64, 0);
                msgOff.setTimeStamp(sequenceObject.sustainPedalChanges.at(nextSustainStep).timeStamp);
                sendMidiMessage(msgOff);
            }
            nextSustainStep++;
        }
    }
    if (sequenceObject.autoPlaySofts)
    {
        if(nextSoftStep<sequenceObject.softPedalChanges.size() &&
           sequenceObject.softPedalChanges.at(nextSoftStep).timeStamp < timeInTicks+leadLag)
        {
//            sendMidiMessage(sequenceObject.softPedalChanges[nextSoftStep]);
//            std::cout
//            << " send " << nextSoftStep
//            << " value " << sequenceObject.softPedalChanges.at(nextSoftStep).getControllerValue()
//            << "\n";
            if (sequenceObject.softPedalChanges.at(nextSoftStep).pedalOn)
            {
                MidiMessage msgOn = MidiMessage::controllerEvent(1, 67, 127);
                msgOn.setTimeStamp(sequenceObject.softPedalChanges.at(nextSoftStep).timeStamp);
                sendMidiMessage(msgOn);
            }
            else
            {
                MidiMessage msgOff = MidiMessage::controllerEvent(1, 67, 0);
                msgOff.setTimeStamp(sequenceObject.softPedalChanges.at(nextSoftStep).timeStamp);
                sendMidiMessage(msgOff);
            }
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
                    String note = MidiMessage::getMidiNoteName(sequenceObject.theSequence.at(stepToTurnOff)->noteNumber,true,true,3);
//                    std::cout << timeInTicks << " Repeated note forced noteOff " << step << " " << note
//                    << " triggeredBy "<<theSequence->at(stepToTurnOff).triggeredBy
//                    << " timeStamp "<<theSequence->at(stepToTurnOff).timeStamp
//                    << " firstInChain "<<theSequence->at(stepToTurnOff).firstInChain
//                    << " firstInChain TS "<<theSequence->at(theSequence->at(stepToTurnOff).firstInChain).timeStamp
//                    << "\n";
//                    midiMessages.addEvent(noteOff,0);
//                    midiOutput->sendMessageNow(noteOff);
                    double t = Time::getMillisecondCounterHiRes()*0.001;
                    noteOff.setTimeStamp(t);
                    if (pluginMessageCollectorIsReset)
                        sendMidiMessage(noteOff);
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
            const double duration = sequenceObject.theSequence.at(step)->getOffTime() - sequenceObject.theSequence.at(step)->getTimeStamp();
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
                        if (scheduledNotes.size()==0)
                            leadLag = noteTimeStamp - timeInTicks;
//                        std::cout << "leadLag " << leadLag << "\n";
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
                    nPrimaryNotes++;
                    sumPrimaryVel += sequenceObject.theSequence.at(triggeredStep)->velocity;
                    if (sequenceObject.theSequence.at(triggeredStep)->triggers>triggeredStep)
                        triggeredStep = sequenceObject.theSequence.at(triggeredStep)->triggers;
                    else //if (sequenceObject.theSequence.at(triggeredStep)->triggers !=-1)
                    //To catch cases where the chain ending note has been moved to be before the chain trigger note
                    {
                        if (sequenceObject.theSequence.at(triggeredStep)->triggers != -1)
                        {
                            std::cout <<"Chain end before chain trigger! curTriggeredStep, nextTriggeredStep "
                            << triggeredStep<<" " <<sequenceObject.theSequence.at(triggeredStep)->triggers<<"\n";
                        }
                        break;
                    }
                }
            }

            //Set sequenceReadHead to time stamp of next note to be played
            //----------------------------------------------------------------------
            if (sequenceObject.theSequence.size()>noteIndex)
            {
                if (availableNotes.size()>0)
                {
                    sequenceReadHead = sequenceObject.theSequence.at(noteIndex)->getTimeStamp()+1;
                    const double noteOnLag = (mostRecentNoteTime-timeInTicks)/10.0;
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
            if (autoPlaying && scheduledNotes.size()>0)
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
                        velocity = (proportionedVelocity/highVelInChain) * thisNoteOriginalVelocity;
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
                //else
                //    assert(false);
                
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
                     (sequenceObject.theSequence.at(step)->getOffTime()-sequenceObject.theSequence.at(step)->getTimeStamp());
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
//                String note = MidiMessage::getMidiNoteName (sequenceObject.theSequence.at(seqStep).getNoteNumber(), true, true, 3);
                if (exprNoteThatStartedThisOnNote == exprEvents[exprEventIndex].getNoteNumber())
                {
                    if (sequenceObject.theSequence.at(seqStep)->triggeredOffNote)
                        sequenceObject.theSequence.at(seqStep)->noteOffNow = true;
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
    exprEvents.clear();
    pauseProcessing = false;

    } catch (const std::out_of_range& ex) {
        std::cout << " error in process block " << "\n";
    }
}

void MIDIProcessor::catchUp (bool sendChangeMessages)
{
    rewind(timeInTicks-xInTicksFromViewer, sendChangeMessages);
}


double MIDIProcessor::getLastUserPlayedStepTime()
{
    double time;
    if (lastUserPlayedSeqStep==-1)
        time = -1.0;
    else
    {
        time = sequenceObject.theSequence.at(lastUserPlayedSeqStep)->getTimeStamp();
    }
    return time;
}

Array<Sequence::StepActivity> MIDIProcessor::setNoteListActivity(bool setNotesActive, Array<int> steps) //Used only by Perform in undo
{
    Array<Sequence::StepActivity> stepActivityList;
    if (setNotesActive==true)
    {
        for (int i=0;i<steps.size();i++)
        {
            if (sequenceObject.theSequence.at(steps[i])->targetNote)
            {
                const Sequence::StepActivity act = {steps[i], true};
                stepActivityList.add(act);
            }
            else
            {
                const Sequence::StepActivity act = {steps[i], false};
                stepActivityList.add(act);
            }
            sequenceObject.theSequence.at(steps[i])->targetNote = true;
        }
        //Remove any duplicate target notes at the same time stamp
        int prevTargetNoteTime = sequenceObject.theSequence.at(0)->getTimeStamp();
        int prevTargetNoteStep = 0;
        for (int step=1;step<sequenceObject.theSequence.size();step++)
        {
            if (sequenceObject.theSequence.at(step)->getTimeStamp() == prevTargetNoteTime &&
                sequenceObject.theSequence.at(step)->targetNote)
            {
                sequenceObject.theSequence.at(step)->targetNote = false;
                int i;
                for (i=0;i<stepActivityList.size();i++)
                    if (stepActivityList[i].step == step)
                    {
                        break;
                    }
                stepActivityList.remove(i);
            }
            if (sequenceObject.theSequence.at(step)->getTimeStamp()!=prevTargetNoteTime && sequenceObject.theSequence.at(step)->targetNote)
            {
                prevTargetNoteTime = sequenceObject.theSequence.at(step)->getTimeStamp();
                prevTargetNoteStep = step;
            }
        }
    }
    else //set Notes inActive
    {
        int firstStep = 0;
        if (steps[0]==0)
            firstStep = 1; //Start at 1 because step 0 must always be active
        for (int i=firstStep;i<steps.size();i++) //Start at 1 because step 0 must always be active
        {
            if (sequenceObject.theSequence.at(steps[i])->targetNote)
            {
                const Sequence::StepActivity act = {steps[i], true};
                stepActivityList.add(act);
            }
            else
            {
                const Sequence::StepActivity act = {steps[i], false};
                stepActivityList.add(act);
            }
            sequenceObject.theSequence.at(steps[i])->targetNote = false;
        }
    }
    if (undoMgr->inUndo || undoMgr->inRedo)
        ;//setTimeInTicks(sequenceObject.theSequence.at(sequenceObject.selectionToRestoreForUndoRedo[0])->getTimeStamp());
    else
    {
        sequenceObject.setChangedFlag(true);
        catchUp();
    }
    //        inUndoRedo = true;
    changeMessageType = CHANGE_MESSAGE_UNDO;
    ;//sequenceObject.selectionToRestoreForUndoRedo = steps;
    sendSynchronousChangeMessage(); //To viewer
    return stepActivityList;
}

Array<Sequence::PrevNoteTimes> MIDIProcessor::timeHumanizeChords (Array<int> steps, String timeSpec)
{
    Array<Sequence::PrevNoteTimes> prevNoteTimesList;
    try {
        if (!sequenceObject.getLoadingFile())
        {
            for (int i=0; i<steps.size(); i++)
            {
                const Sequence::PrevNoteTimes act = {sequenceObject.theSequence.at(steps[i]),
                    sequenceObject.theSequence.at(steps[i])->getTimeStamp()};
                prevNoteTimesList.add(act);
            }
        }
        
        pauseProcessing = true;
//        String timeSpec = sequenceObject.chordTimeHumanize;
        const String randomnessStr = timeSpec.initialSectionContainingOnly("0123456789");
        String chordDurationStr =  timeSpec.getLastCharacters(timeSpec.length()-randomnessStr.length());
        String seedStr = chordDurationStr.fromFirstOccurrenceOf(":", false, true);
        chordDurationStr = chordDurationStr.upToFirstOccurrenceOf(":", false, true).substring(1,999);
        double maxVariation = randomnessStr.getDoubleValue();
//        int chordDirection =  timeSpec.containsAnyOf("/") ? 1 : -1; //It contains either '/' or '\'
//        double chordDuration = chordDurationStr.getDoubleValue();
        struct {
            bool operator()(std::shared_ptr<NoteWithOffTime> a, std::shared_ptr<NoteWithOffTime> b) const
            {
                if (a->noteNumber == b->noteNumber)
                    return a->channel < b->channel;
                else
                    return a->noteNumber > b->noteNumber;
            }
        } customCompare2;
        Array<int> chordsToProcess;
        for (int step=0;step<steps.size();step++)
        {
            if (sequenceObject.theSequence.at(steps[step])->chordIndex != -1)
                chordsToProcess.addIfNotAlreadyThere(sequenceObject.theSequence.at(steps[step])->chordIndex);
        }
        for (int chNum=0;chNum<chordsToProcess.size();chNum++)
        {
            const int chIndex = chordsToProcess[chNum];
            double thisChordTimeStamp = sequenceObject.chords[chIndex].chordTimeStamp;
            const double tempo = sequenceObject.getTempo(sequenceObject.theSequence.
                            at(sequenceObject.chords[chIndex].notePointers.at(0)->currentStep)->getTimeStamp(),
                                                         sequenceObject.scaledTempoChanges);
            double timeIncrement = 10.0*tempo / 625.0;
            std::vector<std::shared_ptr<NoteWithOffTime>> chordNotes = sequenceObject.chords.at(chIndex).notePointers;
            if (chordNotes.size()==0)
                continue;
            std::sort(chordNotes.begin(), chordNotes.end(),customCompare2);
            
            thisChordTimeStamp = sequenceObject.chords[chIndex].chordTimeStamp;
            sequenceObject.chords[chIndex].offsets.clear();
            sequenceObject.chords[chIndex].offsets.push_back(0);
            //Note that some of this code is for use in the future ability to do broken chords
            //                      for (int i=0; i<chordNotes.size(); i++)
            //                          chordNotes.at(i)->chordTopStep = chordTopStep;
            int seed;
            if (seedStr.length()>0)
                seed=seedStr.getIntValue();
            else
                seed = (int) thisChordTimeStamp;

            double shortestDuration = DBL_MAX;
            for (int i=0;i<chordNotes.size();i++)
                if ((chordNotes.at(i)->offTime-chordNotes.at(i)->getTimeStamp()) < shortestDuration)
                    shortestDuration = (chordNotes.at(i)->offTime-chordNotes.at(i)->getTimeStamp());
            
            if (maxVariation > shortestDuration)
                maxVariation = shortestDuration;
            double standardDeviation = maxVariation/3.0;
            
//            std::cout<< "chIndex "<<chIndex<<" "
//            <<" thisChordTimeStamp "<< thisChordTimeStamp
//            <<" topChordNote "<< topChordNote
//            <<" shortestDuration "<< shortestDuration
//            <<" maxVariation "<< maxVariation
//            <<" standardDeviation "<< standardDeviation
//            <<std::endl;
            
            std::default_random_engine generator(seed);
            std::normal_distribution<double> distribution(0.0,standardDeviation);
            sequenceObject.chords[chIndex].timeRandSeed = seed;
            for (int i=1; i<chordNotes.size(); i++) //Don't change top chord note so start at 1
            {
                double proposedNoteTime = thisChordTimeStamp;// + i*increment;
                double rawRandomAdd;
                rawRandomAdd = distribution(generator);
                if (rawRandomAdd<0.0)
                    rawRandomAdd=-rawRandomAdd;
                double randomAdd = rawRandomAdd;
                if (randomAdd>maxVariation)
                    randomAdd=maxVariation-1.0;
                randomAdd = randomAdd*timeIncrement; //This allows variation to be stated in milliseconds in the UI
                proposedNoteTime += randomAdd;
                const double noteDuration = chordNotes.at(i)->getOffTime()-chordNotes.at(i)->getTimeStamp();
                if (proposedNoteTime<sequenceObject.seqDurationInTicks)
                    chordNotes.at(i)->setTimeStamp(proposedNoteTime);
                chordNotes.at(i)->setOfftime(chordNotes.at(i)->getOffTime()+randomAdd);
                if (chordNotes.at(i)->getTimeStamp()+noteDuration <= sequenceObject.seqDurationInTicks)
                    chordNotes.at(i)->setOfftime(chordNotes.at(i)->getTimeStamp()+noteDuration);
                else
                    chordNotes.at(i)->setOfftime(sequenceObject.seqDurationInTicks);
                    std::cout <<  chordNotes.at(i)->currentStep
                      << " randomAdd " << randomAdd
                      << " timeStamp " << chordNotes.at(i)->getTimeStamp()
                      << std::endl;
                const int offset = chordNotes[i]->getTimeStamp() - thisChordTimeStamp;
                sequenceObject.chords[chIndex].offsets.push_back(offset);
            }
        }
        for (int chNum=0;chNum<chordsToProcess.size();chNum++)
        {
            const int chIndex = chordsToProcess[chNum];
            std::vector<std::shared_ptr<NoteWithOffTime>> chordNotes = sequenceObject.chords.at(chIndex).notePointers;
            if (chordNotes.size()==0)
                continue;
            bool foundTargetNote = false;
            double latestNoteTime = -1;
            Array<int> chordSteps;
            for (int i=0; i<chordNotes.size(); i++)
            {
                chordSteps.add(chordNotes.at(i)->currentStep);
                if (chordNotes.at(i)->getTimeStamp() > latestNoteTime)
                    latestNoteTime = chordNotes.at(i)->getTimeStamp();
                if (chordNotes.at(i)->targetNote)
                {
                    foundTargetNote = true;
                }
            }
            if (foundTargetNote) //There was a target note in this chord so chaining needs to be adjusted
            {
//                const double chordWidth = latestNoteTime - chordNotes.at(0)->getTimeStamp();
//                sequenceObject.chain(chordSteps, chordWidth);
            }
        }
        catchUp();
        buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
        pauseProcessing = false;
    } catch (const std::out_of_range& ex) {
        std::cout << " error in timeHumanizeChords " << "\n";
    }
    return prevNoteTimesList;
}

Array<Sequence::NoteVelocities> MIDIProcessor::velocityHumanizeChords (Array<int> steps, String velSpec)
{
    Array<Sequence::NoteVelocities> prevNoteVelList;
    try {
        pauseProcessing = true;
        if (!sequenceObject.getLoadingFile())
        {
            for (int i=0; i<steps.size(); i++)
            {
                const Sequence::NoteVelocities act = {sequenceObject.theSequence.at(steps[i]),
                    sequenceObject.theSequence.at(steps[i])->velocity};
                prevNoteVelList.add(act);
            }
        }
        
        Array<double> strengths;
        std::string numStr = velSpec.toStdString();
        std::string delimiter = ",";
        size_t pos = 0;
        std::string token = std::string();
        while ((pos = numStr.find(delimiter)) != std::string::npos) {
            token = numStr.substr(0, pos);
            strengths.add(String(token).getDoubleValue());
            //                            std::cout << "strength " << strengths.getLast() << std::endl;
            numStr.erase(0, pos + delimiter.length());
        }
        strengths.add(String(numStr).getDoubleValue());
        //                        std::cout << "strength " << strengths.getLast() << std::endl;
        
        Array<int> chordsToProcess;
        for (int step=0;step<steps.size();step++)
        {
            if (sequenceObject.theSequence.at(steps[step])->chordIndex != -1)
                chordsToProcess.addIfNotAlreadyThere(sequenceObject.theSequence.at(steps[step])->chordIndex);
        }

        for (int chNum=0;chNum<chordsToProcess.size();chNum++)
        {
            const int chIndex = chordsToProcess[chNum];
            if (sequenceObject.chords.at(chIndex).notePointers.size()==0)
                continue;
            std::vector<std::shared_ptr<NoteWithOffTime>> chordNotes = sequenceObject.chords.at(chIndex).notePointers;

            const float topNoteVel = chordNotes.at(0)->getVelocity();
            if (chordNotes.size()==2)
            {
                const float ckVel = strengths.getLast() * topNoteVel;
                chordNotes.at(1)->setVelocity(ckVel);
            }
            else // (chord.size()>2)
            {
                for (int j=1;j<chordNotes.size()-1;j++)
                {
                    const float ckVel = strengths.getFirst() * topNoteVel ;
                    chordNotes.at(j)->setVelocity(ckVel);
                }
                const float ckVel = strengths.getLast() * topNoteVel;
                chordNotes.at(chordNotes.size()-1)->setVelocity(ckVel);
            }
        }
        catchUp();
        buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
        pauseProcessing = false;
    } catch (const std::out_of_range& ex) {
        std::cout << " error in velocityHumanizeChords " << "\n";
    }
    return prevNoteVelList;
}

void MIDIProcessor::addPedalChange(PedalType pType)
{
    pauseProcessing = true;
    if (copyOfSelectedNotes.size()<=1)
        return;
    std::vector<Sequence::PedalMessage> *pedalChanges;
    if (pType==sustPedal)
        pedalChanges = &sequenceObject.sustainPedalChanges;
    else// if (pType==softPedal)
        pedalChanges = &sequenceObject.softPedalChanges;
    
    double newStart = sequenceObject.theSequence.at(copyOfSelectedNotes.getFirst())->getTimeStamp();
    double newEnd = sequenceObject.theSequence.at(copyOfSelectedNotes.getLast())->getTimeStamp();
    if (pedalChanges->size()==0)
    {
        Sequence::PedalMessage onMsg = Sequence::PedalMessage(newStart,true);
        Sequence::PedalMessage offMsg = Sequence::PedalMessage(newEnd+1, false);
        pedalChanges->push_back(onMsg);
        pedalChanges->push_back(offMsg);
        catchUp();
        buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
        return;
    }
    
    //        std::cout << " add sustain: newStart, newEnd " <<newStart<<" "<<newEnd<< "\n";
    //        MidiMessageSequence seq;
    for (int i=0; i<pedalChanges->size() ;i+=2)
    {
        //            if (i>10) break;
        double thisStart = pedalChanges->at(i).timeStamp;
        double thisEnd = pedalChanges->at(i+1).timeStamp;
        //            std::cout << i/2 <<" thisStart, thisEnd "<<thisStart<<" "<<thisEnd<< "\n";
        if (newEnd<=thisStart) //==new entirely before 'this'
        {
            //Do nothing
            //                 std::cout << i/2 <<" "<<" ==new entirely before 'this' " <<"\n";
        }
        else if (newStart<=thisStart && (thisStart<=newEnd && newEnd<=thisEnd) ) //new partly before 'this
        {
            //Chop off the start of 'this' after the end of 'new'
            //                std::cout << i/2 <<" "<< " ==new partly before 'this' " <<"\n";
            pedalChanges->at(i).timeStamp = newEnd+3.0;
        }
        else if ((thisStart<=newStart && newStart<=thisEnd) && thisEnd<=newEnd) //new partly after 'this'
        {
            //Chop off the end of 'this' before the start of 'new'
            //                std::cout << i/2 <<" "<< " ==new partly after 'this' " <<"\n";
            pedalChanges->at(i+1).timeStamp = newStart-3.0;
        }
        else if (thisEnd<=newStart) //new entirely after 'this'
        {
            //Do nothing
            //                std::cout << i/2 <<" "<< " ==new entirely after 'this' " <<"\n";
        }
        else if (newStart<=thisStart && thisEnd<=newEnd) //new surrounds 'this'
        {
            //Delete all of 'this'
            //                std::cout << i/2 <<" "<< " ==new surrounds 'this' " << "\n";
            pedalChanges->at(i).timeStamp = DBL_MAX; //To be flushed out later
            pedalChanges->at(i+1).timeStamp = DBL_MAX; //To be flushed out later
        }
        else if (thisStart<=newStart && newEnd<=thisEnd) //'this' surrounds new
        {
            //Treat same as "new partly after 'this'" i.e. Chop off the part of 'this' after the end of 'new'
            //                std::cout << i/2 <<" "<< " =='this' surrounds new " << "\n";
            pedalChanges->at(i+1).timeStamp = newStart-3.0;
        }
    }
    for (int i=(int) pedalChanges->size()-1; 0<=i ;i--) //Delete those marked for deletion above
    {
        if (pedalChanges->at(i).timeStamp == DBL_MAX)
            pedalChanges->erase(pedalChanges->begin() + i);
    }
    if (pedalChanges->back().timeStamp<newStart) //Add at end of list
    {
        Sequence::PedalMessage onMsg = Sequence::PedalMessage(newStart,true);
        Sequence::PedalMessage offMsg = Sequence::PedalMessage(newEnd+1, false);
        if (offMsg.timeStamp>sequenceObject.seqDurationInTicks)
            offMsg.timeStamp = sequenceObject.seqDurationInTicks;
        pedalChanges->push_back(onMsg);
        pedalChanges->push_back(offMsg);
    }
    else //Find place in list
    {
        for (int i=0; i<pedalChanges->size() ;i++)
        {
            if (pedalChanges->at(i).timeStamp>newStart)
            {
                std::vector< Sequence::PedalMessage>::iterator iter;
                Sequence::PedalMessage onMsg = Sequence::PedalMessage(newStart,true);
                Sequence::PedalMessage offMsg = Sequence::PedalMessage(newEnd+1, false);
                iter = pedalChanges->begin() + i;
                pedalChanges->insert(iter, offMsg);
                iter = pedalChanges->begin() + i;
                pedalChanges->insert(iter, onMsg);
                break;
            }
        }
    }
    //        std::cout << " nSusts "<<pedalChanges->size()/2<< "\n";
    //        for (int i=0; i<pedalChanges->size() ;i+=2)
    //        {
    //            std::cout << " Before: onTS "<<pedalChanges->at(i).timeStamp
    //            << " offTS "<<pedalChanges->at(i+1).timeStamp
    //            << "\n";
    //        }
    for (int i=(int) pedalChanges->size()-1; 0<=i ;i-=2) //Delete very short sustains
    {
        const double sustDuration = (pedalChanges->at(i).timeStamp - /*The sust-off*/
                                     pedalChanges->at(i-1).timeStamp) /*The sust-on*/;
        if (sustDuration <= 10)
        {
            const std::vector< Sequence::PedalMessage>::iterator iter1 = pedalChanges->begin() + i - 2;
            const std::vector< Sequence::PedalMessage>::iterator iter2 = pedalChanges->begin() + i;
            pedalChanges->erase(iter1, iter2);
        }
    }
    //        for (int i=0; i<pedalChanges->size() ;i+=2)
    //        {
    //            std::cout << " After: onTS "<<pedalChanges->at(i).timeStamp
    //            << " offTS "<<pedalChanges->at(i+1).timeStamp
    //            << "\n";
    //        }
    catchUp();
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
    pauseProcessing = false;
}

void MIDIProcessor::deletePedalChange(PedalType pType)
{
    pauseProcessing = true;
    //        if (copyOfSelectedNotes.size()<=1)
    //            return;
    std::vector<Sequence::PedalMessage> *pedalChanges;
    if (pType==sustPedal)
        pedalChanges = &sequenceObject.sustainPedalChanges;
    else// if (pType==softPedal)
        pedalChanges = &sequenceObject.softPedalChanges;
    
    double currentZtlTime = timeInTicks-xInTicksFromViewer;
    int deleteBar = -1;
    for (int i=0;i<pedalChanges->size();i+=2)
    {
        if (pedalChanges->at(i).timeStamp < currentZtlTime && currentZtlTime <= pedalChanges->at(i+1).timeStamp)
            deleteBar = i;
    }
    if (deleteBar!=-1)
    {
        const std::vector< Sequence::PedalMessage>::iterator iter1 = pedalChanges->begin() + deleteBar;
        const std::vector< Sequence::PedalMessage>::iterator iter2 = pedalChanges->begin() + deleteBar+2;
        pedalChanges->erase(iter1, iter2);
        catchUp();
        buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
    }
    pauseProcessing = false;
}

  bool MIDIProcessor::atPedalChange(PedalType pType)
  {
      const double currentZtlTime = timeInTicks-xInTicksFromViewer;
      std::vector<Sequence::PedalMessage> *pedalChanges;
      if (pType==sustPedal)
      {
          for (int i=0;i<sequenceObject.sustainPedalChanges.size();i+=2)
          {
              if (sequenceObject.sustainPedalChanges.at(i).timeStamp < currentZtlTime &&
                      currentZtlTime <= sequenceObject.sustainPedalChanges.at(i+1).timeStamp)
                  return true;
          }
      }
      else
      {
          for (int i=0;i<sequenceObject.softPedalChanges.size();i+=2)
          {
              if (sequenceObject.softPedalChanges.at(i).timeStamp < currentZtlTime &&
                  currentZtlTime <= sequenceObject.softPedalChanges.at(i+1).timeStamp)
                  return true;
          }
      }
      return false;
  }

  void MIDIProcessor::createChord()
{
//    std::cout << "MidiProcessor create_chord: 1st selected note"<<sequenceObject.theSequence.at(copyOfSelectedNotes[0])->currentStep <<"\n";
    if (copyOfSelectedNotes.size()<=1)
        return;
    pauseProcessing = true;

    deleteChords(false);
    
    std::vector<std::shared_ptr<NoteWithOffTime>> chordNotes;
    //Put notes in selected range into the chord vector
    for(int i=copyOfSelectedNotes.getFirst() ; i<=copyOfSelectedNotes.getLast(); i++)
    {
        chordNotes.push_back(sequenceObject.theSequence[i]);
    }
    struct {
        bool operator()(std::shared_ptr<NoteWithOffTime> a, std::shared_ptr<NoteWithOffTime> b) const
        {
            if (a->noteNumber == b->noteNumber)
                return a->channel < b->channel;
            else
                return a->noteNumber > b->noteNumber;
        }
    } customCompare2;
    std::sort(chordNotes.begin(), chordNotes.end(),customCompare2);
    int chordIndex = sequenceObject.newChordFromSteps(chordNotes);
    chordNotes.at(0)->chordTopStep=-1;
    chordNotes.at(0)->noteIndexInChord=-1;
    for (int i=1; i<chordNotes.size(); i++)
    {
        chordNotes.at(i)->chordTopStep = chordNotes.at(0)->currentStep;
        chordNotes.at(i)->chordIndex = chordIndex;
        chordNotes.at(i)->noteIndexInChord = i;
    }
//    std::cout << "added chord at "<< chordIndex <<"\n";
//    for (int i=0;i<sequenceObject.chords.at(chordIndex).notePointers.size();i++)
//    {
//        if (
//        std::cout << "chord note: step "<< sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep
//        <<"  timestamp " << sequenceObject.chords.at(chordIndex).notePointers.at(i)->timeStamp
//        << "\n";
//    }
    catchUp();
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
    pauseProcessing = false;
}

void MIDIProcessor::deleteChords(bool rebuild)
{
    if (copyOfSelectedNotes.size()==0)
        return;
    pauseProcessing = true;
    int adjustedFirstStep = copyOfSelectedNotes.getFirst();;
    
    if (sequenceObject.theSequence.at(copyOfSelectedNotes.getFirst())->inChord)
    {
        const int chordIndex = sequenceObject.theSequence.at(copyOfSelectedNotes.getFirst())->chordIndex;
        adjustedFirstStep=INT_MAX;
        for (int i=0;i<sequenceObject.chords.at(chordIndex).notePointers.size();i++) //Find lowest and highest note step in chord
        {
            if (sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep < adjustedFirstStep)
                adjustedFirstStep = sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep;
        }
    }
    else
    {
        adjustedFirstStep = copyOfSelectedNotes.getFirst();;
    }
    
    int adjustedLastStep = adjustedLastStep = copyOfSelectedNotes.getLast();
    if (sequenceObject.theSequence.at(copyOfSelectedNotes.getLast())->inChord)
    {
        const int chordIndex = sequenceObject.theSequence.at(copyOfSelectedNotes.getLast())->chordIndex;
        adjustedLastStep=INT_MIN;
        for (int i=0;i<sequenceObject.chords.at(chordIndex).notePointers.size();i++) //Find lowest and highest note step in chord
        {
            if (sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep > adjustedLastStep)
                adjustedLastStep = sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep;
        }
    }
    else
    {
        adjustedLastStep = adjustedLastStep = copyOfSelectedNotes.getLast();
    }
    
    Array<int> chordsToRemove;
    for(int i=adjustedFirstStep ; i<=adjustedLastStep; i++)
    {
//        std::cout << "MidiProcessor chord "<< i << "\n";
        if (sequenceObject.theSequence.at(i)->inChord)
        {
//            std::cout << "Request removal of chord for step "<< i << " index " << sequenceObject.theSequence.at(i)->chordIndex << "\n";
            chordsToRemove.addIfNotAlreadyThere(sequenceObject.theSequence.at(i)->chordIndex);
            sequenceObject.theSequence.at(i)->chordIndex = -1;
            sequenceObject.theSequence.at(i)->chordTopStep = -1;
            sequenceObject.theSequence.at(i)->noteIndexInChord = -1;
            sequenceObject.theSequence.at(i)->inChord = false;
        }
    }
    int chordAfterDeletions = chordsToRemove.getLast()+1;
    if (chordsToRemove.size()>0)
    {
        int nChordsDeleted = chordsToRemove.size();
//        std::cout << "chordsToRemove: first, last " << chordsToRemove.getFirst()<<" "<<chordsToRemove.getLast()<<"\n";
        sequenceObject.chords.erase(sequenceObject.chords.begin()+chordsToRemove.getFirst(),
                                    sequenceObject.chords.begin()+chordsToRemove.getLast()+1);
//        if (chordsToRemove.size()<sequenceObject.chords.size() && adjustedLastStep<sequenceObject.theSequence.size())
//        {
            int tempCount = 0;
            for (auto step=sequenceObject.theSequence.begin();step!=sequenceObject.theSequence.end();step++)
            {
                if ((*step)->chordIndex >= chordAfterDeletions)
                    (*step)->chordIndex -= nChordsDeleted;

                if (adjustedFirstStep<=(*step)->currentStep && (*step)->currentStep<=adjustedLastStep)
                {
                    (*step)->chordTopStep = -1;
                    (*step)->chordIndex = -1;
                    (*step)->inChord = false;
                }
                tempCount++;
            }
//        }
        if (rebuild)
        {
            catchUp();
            buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
        }
    }
    pauseProcessing = false;
//    std::cout << "MidiProcessor END delete_chord\n";
}

void MIDIProcessor::autoCreateChords(double maxLength) //Based on notes chained to target notes in selection , limited to maxLength
{
    if (copyOfSelectedNotes.size()==0)
        return;
    for (int step=copyOfSelectedNotes.getFirst();step<=copyOfSelectedNotes.getLast();step++)
    {
        if (sequenceObject.theSequence.at(step)->targetNote)
        {
            
        }
    }
}

Array<Sequence::StepActivity> MIDIProcessor::chainCommand (Array<int> selection, double inverval)
{
    //        std::cout << "chainCommand: interval = " <<inverval<<"\n";
    Array<Sequence::StepActivity> stepActivity = sequenceObject.chain(selection, inverval);
    if (undoMgr->inUndo || undoMgr->inRedo)
    {
        double ztlTime;
        if (xInTicksFromViewer==0)
            ztlTime = getTimeInTicks();
        else
            ztlTime = getTimeInTicks()-xInTicksFromViewer;
        setTimeInTicks(ztlTime);
    }
    else
    {
        sequenceObject.setChangedFlag(true);
        catchUp();
    }
    changeMessageType = CHANGE_MESSAGE_UNDO;
    //sequenceObject.selectionToRestoreForUndoRedo = selection;
    sendSynchronousChangeMessage(); //To midiProcessor
    return stepActivity;
}

//This sets the humanize properties of chords.  Actual time adjustment is done in buildSequenceAsOf
void MIDIProcessor::humanizeChordNoteTimes ()
{
    std::cout << "humanizeChordNoteTimes: params = " <<sequenceObject.chordTimeHumanize<<"\n";
    if (copyOfSelectedNotes.size()==0)
        return;
    pauseProcessing = true;
    int adjustedFirstStep = copyOfSelectedNotes.getFirst();
    
    if (sequenceObject.theSequence.at(copyOfSelectedNotes.getFirst())->inChord)
    {
        const int chordIndex = sequenceObject.theSequence.at(copyOfSelectedNotes.getFirst())->chordIndex;
        adjustedFirstStep=INT_MAX;
        for (int i=0;i<sequenceObject.chords.at(chordIndex).notePointers.size();i++) //Find lowest and highest note step in chord
        {
            if (sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep < adjustedFirstStep)
                adjustedFirstStep = sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep;
        }
    }
    else
    {
        adjustedFirstStep = copyOfSelectedNotes.getFirst();;
    }
    
    int adjustedLastStep = adjustedLastStep = copyOfSelectedNotes.getLast();
    if (sequenceObject.theSequence.at(copyOfSelectedNotes.getLast())->inChord)
    {
        const int chordIndex = sequenceObject.theSequence.at(copyOfSelectedNotes.getLast())->chordIndex;
        adjustedLastStep=INT_MIN;
        for (int i=0;i<sequenceObject.chords.at(chordIndex).notePointers.size();i++) //Find lowest and highest note step in chord
        {
            if (sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep > adjustedLastStep)
                adjustedLastStep = sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep;
        }
    }
    else
    {
        adjustedLastStep = adjustedLastStep = copyOfSelectedNotes.getLast();
    }
    
    Array<int> chordsToHumanize;
    for(int i=adjustedFirstStep ; i<=adjustedLastStep; i++)
    {
        //        std::cout << "MidiProcessor chord "<< i << "\n";
        if (sequenceObject.theSequence.at(i)->inChord)
        {
            chordsToHumanize.addIfNotAlreadyThere(sequenceObject.theSequence.at(i)->chordIndex);
        }
    }
    for (int i=0;i<chordsToHumanize.size();i++)
    {
        sequenceObject.chords[chordsToHumanize[i]].timeSpec = "h:"+sequenceObject.chordTimeHumanize;
//        std::cout << "Time Humanize chord step "<<chordsToHumanize[i]<<" "<< sequenceObject.chords[chordsToHumanize[i]].timeSpec << "\n";
    }
    catchUp();
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
    if (copyOfSelectedNotes.size()==0)
        return;
    pauseProcessing = true;
    pauseProcessing = false;
}

//This sets the velocity properties of chords.  Actual time adjustment is done in buildSequenceAsOf
void MIDIProcessor::humanizeChordNoteVelocities ()
{
//    std::cout << "humanizeChordNoteVelocities: params = " <<sequenceObject.chordVelocityHumanize<<"\n";
    if (copyOfSelectedNotes.size()==0)
        return;
    pauseProcessing = true;
    int adjustedFirstStep = copyOfSelectedNotes.getFirst();
    
    if (sequenceObject.theSequence.at(copyOfSelectedNotes.getFirst())->inChord)
    {
        const int chordIndex = sequenceObject.theSequence.at(copyOfSelectedNotes.getFirst())->chordIndex;
        adjustedFirstStep=INT_MAX;
        for (int i=0;i<sequenceObject.chords.at(chordIndex).notePointers.size();i++) //Find lowest and highest note step in chord
        {
            if (sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep < adjustedFirstStep)
                adjustedFirstStep = sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep;
        }
    }
    else
    {
        adjustedFirstStep = copyOfSelectedNotes.getFirst();;
    }
    
    int adjustedLastStep = adjustedLastStep = copyOfSelectedNotes.getLast();
    if (sequenceObject.theSequence.at(copyOfSelectedNotes.getLast())->inChord)
    {
        const int chordIndex = sequenceObject.theSequence.at(copyOfSelectedNotes.getLast())->chordIndex;
        adjustedLastStep=INT_MIN;
        for (int i=0;i<sequenceObject.chords.at(chordIndex).notePointers.size();i++) //Find lowest and highest note step in chord
        {
            if (sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep > adjustedLastStep)
                adjustedLastStep = sequenceObject.chords.at(chordIndex).notePointers.at(i)->currentStep;
        }
    }
    else
    {
        adjustedLastStep = adjustedLastStep = copyOfSelectedNotes.getLast();
    }
    
    Array<int> chordsToHumanize;
    for(int i=adjustedFirstStep ; i<=adjustedLastStep; i++)
    {
        //        std::cout << "MidiProcessor chord "<< i << "\n";
        if (sequenceObject.theSequence.at(i)->inChord)
        {
            chordsToHumanize.addIfNotAlreadyThere(sequenceObject.theSequence.at(i)->chordIndex);
        }
    }
    for (int i=0;i<chordsToHumanize.size();i++)
    {
        sequenceObject.chords[chordsToHumanize[i]].velSpec = "h:"+sequenceObject.chordVelocityHumanize;
//        std::cout << "Vel Humanize chord step "<<chordsToHumanize[i]<<" "<< sequenceObject.chords[chordsToHumanize[i]].timeSpec << "\n";
    }
    catchUp();
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, getSequenceReadHead());
    
    pauseProcessing = false;
}

void MIDIProcessor::setIndividualNotesActivity (Array<Sequence::StepActivity> act) //Used only to restore activity after undo
{
    for (int i=0;i<act.size();i++)
        sequenceObject.theSequence.at(act[i].step)->targetNote = act[i].active;
    if(undoMgr->inUndo)
    {
        Array<int> steps;
        for (int i=0;i<act.size();i++)
            steps.add(act[i].step);
        changeMessageType = CHANGE_MESSAGE_UNDO;
        //sequenceObject.selectionToRestoreForUndoRedo = steps;
        //setTimeInTicks(sequenceObject.theSequence.at(sequenceObject.selectionToRestoreForUndoRedo[0])->getTimeStamp());
        //            inUndoRedo = true;
        double ztlTime;
        if (xInTicksFromViewer==0)
            ztlTime = getTimeInTicks();
        else
            ztlTime = getTimeInTicks()-xInTicksFromViewer;
        setTimeInTicks(ztlTime);
        
        sendSynchronousChangeMessage();
        changeMessageType = CHANGE_MESSAGE_NONE;
    }
    else
        sequenceObject.selectionToRestoreForUndoRedo.clear();
}

void MIDIProcessor::setIndividualNoteTimes (Array<Sequence::PrevNoteTimes> prevTimes) //For use in undo
{
    for (int i=0;i<prevTimes.size();i++)
    {
        double delta = prevTimes[i].time - prevTimes[i].note->timeStamp;
        prevTimes[i].note->timeStamp = prevTimes[i].time;
        prevTimes[i].note->offTime += delta;
    }
//    if(undoMgr->inUndo)
//    {
//        std::vector<std::shared_ptr<NoteWithOffTime>> notes;
//        for (int i=0;i<prevTimes.size();i++)
//            notes.push_back(prevTimes[i].note);
//        
    double ztlTime;
    if (xInTicksFromViewer==0)
        ztlTime = getTimeInTicks();
    else
        ztlTime = getTimeInTicks()-xInTicksFromViewer;
    setTimeInTicks(ztlTime);
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, timeInTicks);
//    }
//    else
//        sequenceObject.selectionToRestoreForUndoRedo.clear();
}

void MIDIProcessor::setIndividualNoteOffTimes (Array<Sequence::PrevNoteTimes> prevOffTimes) //For use in undo
{
    for (int i=0;i<prevOffTimes.size();i++)
    {
        prevOffTimes[i].note->offTime = prevOffTimes[i].time;
    }
//    if(undoMgr->inUndo)
//    {
//        std::vector<std::shared_ptr<NoteWithOffTime>> notes;
//        for (int i=0;i<prevOffTimes.size();i++)
//            notes.push_back(prevOffTimes[i].note);
//        
    double ztlTime;
    if (xInTicksFromViewer==0)
        ztlTime = getTimeInTicks();
    else
        ztlTime = getTimeInTicks()-xInTicksFromViewer;
    setTimeInTicks(ztlTime);
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, timeInTicks);
//    }
    //    else
    //        sequenceObject.selectionToRestoreForUndoRedo.clear();
}

bool MIDIProcessor::getNoteActivity(int step)
{
    return sequenceObject.theSequence.at(step)->targetNote;
}

Array<Sequence::NoteVelocities> MIDIProcessor::changeNoteVelocities(Array<Sequence::NoteVelocities> changeList)
{
    Array<Sequence::NoteVelocities> preVels;
    for (int i=0;i<changeList.size();i++)
    {
        Sequence::NoteVelocities velRecord;
        velRecord.note = changeList[i].note;
        velRecord.velocity = changeList[i].note->velocity;
        preVels.add(velRecord);
        changeList[i].note->velocity = changeList[i].velocity;
    }
    return preVels;
}

void MIDIProcessor::restoreNoteVelocities(Array<Sequence::NoteVelocities> changeList)
{
    for (int i=0;i<changeList.size();i++)
    {
        changeList[i].note->velocity = changeList[i].velocity;
    }
}


void MIDIProcessor::setTempoMultiplier(double value, double currentTime, bool documentReallyChanged)
{
    std::cout << "setTempoMultiplier "<<value<<"\r";
    if (value<0.1)
        value = 0.1;
    int indexBeforeCurrentTime = -1;
    int index;
    for (index=sequenceObject.bookmarkTimes.size()-1 ;0<=index ;index--)
    {
        //        std::cout
        //        << " currentTime " << currentTime
        //        << " bookmark time " << bookmarkTimes[index].time
        //        << " bookmark ScaleFactor " << bookmarkTimes[index].tempoScaleFactor
        //        << "\n";
        if (sequenceObject.bookmarkTimes[index].tempoChange && ((int)sequenceObject.bookmarkTimes[index].time)<=currentTime)
        {
            indexBeforeCurrentTime = index;
            break;
        }
    }
    
    if (indexBeforeCurrentTime == -1)
    {
        //        if (bookmarkTimes.size()>0 && bookmarkTimes[0].time==0 && bookmarkTimes[0].tempoChange)
        //        {
        //            Bookmark bm = bookmarkTimes[0];
        //            bm.tempoScaleFactor = value;
        //            bookmarkTimes.set(0, bm);
        //        }
        //        else
        //        {
        Sequence::Bookmark bm;
        bm.time = 0;
        bm.tempoChange = true;
        bm.tempoScaleFactor = value;
        sequenceObject.bookmarkTimes.insert(0,bm);
        //        }
    }
    else
    {
        Sequence::Bookmark bm;
        bm.time = sequenceObject.bookmarkTimes[indexBeforeCurrentTime].time;
        bm.tempoChange = true;
        bm.tempoScaleFactor = value;
        sequenceObject.bookmarkTimes.set(indexBeforeCurrentTime,bm);
    }
    sequenceObject.setChangedFlag (documentReallyChanged);
    double ztlTime;
    if (xInTicksFromViewer==0)
        ztlTime = getTimeInTicks();
    else
        ztlTime = getTimeInTicks()-xInTicksFromViewer;
    rewind(ztlTime);
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, ztlTime);
}

Array<Sequence::PrevNoteTimes> MIDIProcessor::changeNoteTimes(std::vector<std::shared_ptr<NoteWithOffTime>> notes, double delta)
{
    //For steps in selection, construct prevNoteTimesList
    //Entries in stepActivityList are {int step; bool active}
    std::vector<std::shared_ptr<NoteWithOffTime>> notesToChange = notes;
    for (int i=0; i<notes.size(); i++)
    {
        if (notes.at(i)->inChord)
        {
            const int chordIndex = notes.at(i)->chordIndex;
            int iHighest = 0;
            for (int i=1;i<sequenceObject.chords[chordIndex].notePointers.size();i++)
                if (sequenceObject.chords[chordIndex].notePointers.at(i)->noteNumber >
                            sequenceObject.chords[chordIndex].notePointers.at(iHighest)->noteNumber)
                    iHighest = i;
            if (notes.at(i)->currentStep == sequenceObject.chords[chordIndex].notePointers[iHighest]->currentStep)
            {
                //If it's the chord's top step add all the chord's notes to steps to be changed
                for (int i=0;i<sequenceObject.chords[chordIndex].notePointers.size();i++)
                {
                    if (std::find(notes.begin(), notes.end(), sequenceObject.chords[chordIndex].notePointers.at(i)) == notes.end())
                        notesToChange.push_back(sequenceObject.chords[chordIndex].notePointers.at(i));
                }
                sequenceObject.chords[chordIndex].chordTimeStamp += delta;
            }
        }
    }
    Array<Sequence::PrevNoteTimes> prevNoteTimesList;
    if (!sequenceObject.getLoadingFile())
    {
        for (int i=0; i<notesToChange.size(); i++)
        {
            const Sequence::PrevNoteTimes act = {notesToChange.at(i), notesToChange.at(i)->getTimeStamp()};
            prevNoteTimesList.add(act);
        }
    }
    for (int i=0; i<notesToChange.size(); i++)
    {
        double timeStamp = notesToChange.at(i)->getTimeStamp();
        double offTime = notesToChange.at(i)->getOffTime();
        timeStamp += delta;
        offTime += delta;
        notesToChange.at(i)->setTimeStamp(timeStamp);
        notesToChange.at(i)->setOfftime(offTime);
    }
    sequenceObject.setChangedFlag(true);
    double ztlTime;
    if (xInTicksFromViewer==0)
        ztlTime = getTimeInTicks();
    else
        ztlTime = getTimeInTicks()-xInTicksFromViewer;
    rewind(ztlTime);
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, ztlTime);
    return prevNoteTimesList;
}
Array<Sequence::PrevNoteTimes>  MIDIProcessor::changeNoteOffTimes(std::vector<std::shared_ptr<NoteWithOffTime>> notes, double delta)
{
//    std::cout << "changeNoteOffTimes "<<steps.size()<<" "<< delta<<"\n";
    Array<Sequence::PrevNoteTimes> prevNoteTimesList;
    if (!sequenceObject.getLoadingFile())
    {
        for (int i=0; i<notes.size(); i++)
        {
            const Sequence::PrevNoteTimes act = {notes.at(i), notes.at(i)->getOffTime()};
            prevNoteTimesList.add(act);
        }
    }
    for (int i=0;i<notes.size();i++)
    {
        const double proposedOffTime = notes.at(i)->offTime+delta;
        if (proposedOffTime>(notes.at(i)->getTimeStamp()+10))
            notes.at(i)->setOfftime(proposedOffTime);
        else
            notes.at(i)->setOfftime(notes.at(i)->getTimeStamp()+10);
    }
    sequenceObject.setChangedFlag(true);
//    catchUp();
    double ztlTime;
    if (xInTicksFromViewer==0)
        ztlTime = getTimeInTicks();
    else
        ztlTime = getTimeInTicks()-xInTicksFromViewer;
    buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doRetainEdits, ztlTime);
    return prevNoteTimesList;
}
void MIDIProcessor::setCopyOfSelectedNotes(Array<int> sel)
{
    copyOfSelectedNotes = sel;
}

void MIDIProcessor::setListenSequence(double startTime, double endTime, Array<int> tracks)
{
    listenSequence.clear();
    for (int step=0; step<sequenceObject.theSequence.size(); step++)
    {
        if (startTime <= sequenceObject.theSequence.at(step)->getTimeStamp() && sequenceObject.theSequence.at(step)->getTimeStamp()<=endTime
            && (tracks.size()==0||tracks.contains(sequenceObject.theSequence.at(step)->getTrack())))
        {
            NoteWithOffTime onMsg = *(sequenceObject.theSequence.at(step));
            NoteWithOffTime offMsg = onMsg;
            offMsg.timeStamp = (onMsg.offTime);
            offMsg.velocity = (0);
            listenSequence.push_back(onMsg);
            listenSequence.push_back(offMsg);
        }
    }
    std::sort(listenSequence.begin(), listenSequence.end());
    //        for (int i=0;i<20;i++)
    //            std::cout << i
    //            << " note "<<  listenSequence[i].getNoteNumber()
    //            <<" channel "  <<  listenSequence[i].getChannel()
    //            << " velocity " << (int) listenSequence[i].getVelocity()
    //            << "\n";
}

void MIDIProcessor::hiResTimerCallback()
{
    processBlock();
}
