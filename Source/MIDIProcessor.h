/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#define CLOCK_TIMER 0

#include "../JuceLibraryCode/JuceHeader.h"
#include "NoteWithOffTime.h"
#include "Sequence.h"
#include <iostream>
#include <deque>

#define TIMER_STOPLISTEN 0
#define TIMER_APP_ACTIVE 1

//==============================================================================
/**
*/
class MIDIProcessor  :
    public MidiMessageCollector,
    public ChangeBroadcaster,
    private HighResolutionTimer,
    private MultiTimer,
    public ChangeListener
{
public:
    //==============================================================================
    MIDIProcessor();
    ~MIDIProcessor();
    CriticalSection audioProcessing;

    //==============================================================================
    void hiResTimerCallback() override;
    
    void changeListenerCallback (ChangeBroadcaster* broadcaster) override;
    void timerCallback (int timerID) override;
    double getTempo ()
    {
//        return 625*timeIncrement; //625 = 60000.0/96.0
//        else
        if (timeInTicks<=0)
            return 60;
        else
            return sequenceObject.tempoMultiplier * sequenceObject.getTempo(timeInTicks);
    }
    double getRealTimeTempo ()
    {
        if (timeInTicks<=0)
            return sequenceObject.tempoMultiplier * sequenceObject.getTempo(0) * variableTempoRatio;
        else
        {
//            std::cout<< "getRealTimeTempo "<< timeInTicks << " "<<sequenceObject.getTempo(timeInTicks) <<"\n";
            return sequenceObject.tempoMultiplier * sequenceObject.getTempo(timeInTicks) * variableTempoRatio;
        }
    }
    
    void processBlock ();
    inline double getTimeInTicks()
    {
//        std::cout<< "getTimeInTicks "<< timeInTicks << "\n";
        return timeInTicks;
    }
    void setTimeInTicks(double time)
    {
//        std::cout<< "setTimeInTicks "<< time << "\n";
        timeInTicks = time;
//        accompTimeInTicks = time;
    }
    void setLeadTimeInTicks(int ticks) {leadTimeInTicks = ticks;}
    
    void loadFromUserSpecifiedFile ()//const bool showMessageOnFailure)
    {
        //    WildcardFileFilter filt = WildcardFileFilter("*.mid;*.ckf;*.ppf", "*", "Midi or ConcertKeyboardist files");
        FileChooser fc ("Open Sequence",File(),"*.ckf;*.mid;*.ppf", true);
        //                    getLastDocumentOpened(),
        //                    "*.mid;*.ckf;*.ppf");
        //
        if (fc.browseForFileToOpen())
        {
            File file = fc.getResult();
            if (!file.isDirectory())
            {
                String fn = file.getFullPathName();
                sequenceObject.setScoreFile(file);
                
                sequenceObject.loadDocument(File(fn));//This is not "loadSequence" but triggers loading it with loadSequence
            }
        }
        //return Result::fail (TRANS("User cancelled"));
    }
    void loadSpecifiedFile (File file)//const bool showMessageOnFailure)
    {
        sequenceObject.setScoreFile(file);
        sequenceObject.loadDocument(file);
        //return Result::fail (TRANS("User cancelled"));
    }
    
    //enum LoadType {loadFile, reAnalyzeOnly};
    void buildSequenceAsOf(Sequence::LoadType type, Sequence::Retain retainEdits, double time)
    {
        pauseGLRendering = true;
        HighResolutionTimer::stopTimer();
        sequenceObject.loadSequence(type, retainEdits);
        HighResolutionTimer::startTimer(timerIntervalInMS);
        rewind(time);  //Note that rewind now does a sendChangeMessage
    }

    std::vector<NoteWithOffTime> upcomingNoteGroup; //Group of notes just past the "nextAvailable" group. Used for highlighting them.
    bool isPlaying;
    bool isListening; //True is we are listening to a selection
    double startListenTime;
    double endListenTime;
    double lastStartTime;
    bool pauseClock; //Used to halt increasing timeInTicks if next noteOn time lags too much behind timeInTick
//    bool waitForFirstNote; //If true when play started waitingForFirstNote is set to true and next unplayed note moved to ztl.
    bool waitingForFirstNote; //If set, time does not increment.  This is set false when the first expr note is played.
    void rewind (double time);
    void listenToSelection();
    void endListen();
#define CHANGE_MESSAGE_NONE -1
#define CHANGE_MESSAGE_REWIND 0
#define CHANGE_MESSAGE_TWEEN 1
#define CHANGE_MESSAGE_START_PLAYING 2
#define CHANGE_MESSAGE_STOP_PLAYING 3
#define CHANGE_MESSAGE_BEAT_CHANGED 4
#define CHANGE_MESSAGE_MEASURE_CHANGED 5
#define CHANGE_MESSAGE_NOTE_PLAYED 6
#define CHANGE_MESSAGE_UNDO 7
    int changeMessageType; //Set before sending a change message - used by viewer to choose desired action
//    bool inUndoRedo = false;
    //Set before sending a CHANGE_MESSAGE_TWEEN
    double tweenTo;
    double transitionTime;
    
    void tweenMove (double targetTime, double transitionTime);
    
    void playableStepForwardBack (bool direction);
    int getMeasure(double horizontalShift);
    bool getMetronomeIlluminated()
    {
        return metronomeLighted;
    }
    void measureForwardBack (bool direction);
    void bookmarkForwardBack (bool direction);
    bool atZTL();
#define BOOKMARK_ADD 0
#define BOOKMARK_REMOVE 1
#define BOOKMARK_TOGGLE 2
    double atBookmark (); //Returns exact bookmark time ZTL is close to bookmark, else -1
    void addRemoveBookmark (int action); //At current ZTL position
    double xInTicksFromViewer;
    void setXInTicks(double x)
    {
        xInTicksFromViewer = x;
//        std::cout
//        << "set xInTicksFromViewer " << xInTicksFromViewer
//        << " timeInTicks " << timeInTicks
//        << "\n";
    }
    
    bool playing() {return isPlaying;}

    void play(bool ply, String fromWhere);  //Set to >= 0 reset viewer. All notes up to this step are marked as played.

    double getSequenceReadHead()
    {
        return sequenceReadHead;
    }

    int startStep, endStep;
    double resetToTime;
    
    bool pauseGLRendering;
    inline bool getPauseGLRendering() //Called by renderOpenGL to determine if pause should be started or stopped
    {
        return true;
//        return pauseGLRendering;
    }
    
    void updateActiveTracks()
    {
        buildSequenceAsOf(Sequence::reAnalyzeOnly, Sequence::doNotRetainEdits, 0.0);
    }
    
    bool noteIsOn(int seqStep)
    {
        return onNotes.contains(seqStep);
    }
    
    void catchUp();
//    void dumpData(int nSteps);
    
    class Listener
    {
    public:
        virtual ~Listener()  {}
        
//        /** Called after a sequence is finished loading. */
//        virtual void loadedSequence (MIDIProcessor*) = 0;
        
        /** Called when there is a change in the set of currently on notes. */
        virtual void activeNotesChanged (MIDIProcessor*)  {}
    };
    
    /** Registers a listener to receive events when this button's state changes.
     If the listener is already registered, this will not register it again.
     @see removeListener
     */
    void addListener (Listener* newListener);

    /** Removes a previously-registered button listener
     @see addListener
     */
    void removeListener (Listener* listener);
    void synthMessageCollectorReset(const double rate)
    {
        synthMessageCollector.reset(rate);
        if (rate!=44100)
            synthMessageCollectorIsReset = true;
    }
    enum class MidiDestination {internalSynth = 0, output = 1, pluginSynth = 2};
    void setMidiDestination ( MidiDestination dest )
    {
        midiDestination = dest;
    }
    
#define FIFO_SIZE 50
    int noteOnOffFifoBuffer [FIFO_SIZE];
    AbstractFifo noteOnOffFifo;
    bool synthMessageCollectorIsReset;
    MidiMessageCollector synthMessageCollector;
    
    void prepareToSave()
    {
        sequenceObject.sequenceProps.setValue("majorVersion",  var(99)); //That created this file
        sequenceObject.sequenceProps.setValue("minorVersion", var(99)); //That created this file
        sequenceObject.sequenceProps.setValue("buildNumber", var(99)); //That created this file
    }
    
    Sequence sequenceObject;
    
    bool resetViewer; //Used as flag to tell viewer in a change notification to reset itself
    
    double mostRecentNoteTime;
    double earliness;
    double leadLag;
    int initialWindowHeight;
    
    void setNotesEditable(bool edit)
    {
        if (edit)
        {
            notesEditable=true;
        }
        else
        {
            notesEditable=false;
        }
        changeMessageType = CHANGE_MESSAGE_REWIND; //To cause makeNoteBars to redraw note heads
        sendSynchronousChangeMessage();
//        repaint();
    }
    inline bool getNotesEditable()
    {
        return notesEditable;
    }
    int lastPlayedSeqStep;
    void setLastPlayedSeqStep()  //Set step to next active step after this time
    {
        int i;
        const double time = timeInTicks-xInTicksFromViewer;
        for (i=0;i<sequenceObject.theSequence.size();i++)
        {
//            std::cout << "step, ts, targetTime " << i<<" "<<sequenceObject.theSequence.at(i).getTimeStamp()<<" "<<time<<"\n";
            if (sequenceObject.theSequence.at(i).triggeredBy==-1 && sequenceObject.theSequence.at(i).getTimeStamp()>=time)
                break;
        }
        lastPlayedSeqStep = i-1;
    }
    
    

    Array<Sequence::StepActivity> setNoteListActivity(bool setNotesActive, Array<int> steps) //Used only by Perform in undo
    {
        Array<Sequence::StepActivity> stepActivityList;
        if (setNotesActive==true)
        {
            for (int i=0;i<steps.size();i++)
            {
                const double ts = sequenceObject.theSequence.at(steps[i]).getTimeStamp();
                const int index = sequenceObject.targetNoteTimes.indexOf(ts);
                if (index>=0)
                {
                    const Sequence::StepActivity act = {steps[i], true};
                    stepActivityList.add(act);
                }
                else
                {
                    if (sequenceObject.theSequence.at(steps[i]).chordTopStep==-1)
                        sequenceObject.targetNoteTimes.add(ts);
                    const Sequence::StepActivity act = {steps[i], false};
                    stepActivityList.add(act);
                }
            }
        }
        else //set Notes inActive
        {
            for (int i=0;i<steps.size();i++)
            {
                const double ts = sequenceObject.theSequence.at(steps[i]).getTimeStamp();
                const int index = sequenceObject.targetNoteTimes.indexOf (ts);
                if (index>=0)
                {
                    if (sequenceObject.theSequence.at(steps[i]).chordTopStep==-1)
                        sequenceObject.targetNoteTimes.remove(index);
                    const Sequence::StepActivity act = {steps[i], true};
                    stepActivityList.add(act);
                }
                else
                {
                    const Sequence::StepActivity act = {steps[i], false};
                    stepActivityList.add(act);
                }
            }
        }

        if (undoMgr->inUndo || undoMgr->inRedo)
            setTimeInTicks(sequenceObject.theSequence.at(sequenceObject.undoneOrRedoneSteps[0]).getTimeStamp());
        else
        {
            sequenceObject.setChangedFlag(true);
            catchUp();
        }
//        inUndoRedo = true;
        changeMessageType = CHANGE_MESSAGE_UNDO;
        sequenceObject.undoneOrRedoneSteps = steps;
        sendSynchronousChangeMessage(); //To viewer
        return stepActivityList;
    }
    
    Array<Sequence::StepActivity> chainCommand (Array<int> selection, double inverval)
    {
        Array<Sequence::StepActivity> stepActivity = sequenceObject.chain(selection, inverval);
        if (undoMgr->inUndo || undoMgr->inRedo)
            setTimeInTicks(sequenceObject.theSequence.at(sequenceObject.undoneOrRedoneSteps[0]).getTimeStamp());
        else
        {
            sequenceObject.setChangedFlag(true);
            catchUp();
        }
//        inUndoRedo = true;
        changeMessageType = CHANGE_MESSAGE_UNDO;
        sequenceObject.undoneOrRedoneSteps = selection;
        sendSynchronousChangeMessage(); //To midiProcessor
        return stepActivity;
    }

    void setIndividualNotesActivity (Array<Sequence::StepActivity> act) //Used only to restore activity after undo
    {
        for (int i=0;i<act.size();i++)
        {
            if (act[i].active)
                setAsTargetNote(act[i].step);
            else
                setAsNonTargetNote(act[i].step);
        }
        if(undoMgr->inUndo)
        {
            Array<int> steps;
            for (int i=0;i<act.size();i++)
                steps.add(act[i].step);
            changeMessageType = CHANGE_MESSAGE_UNDO;
            sequenceObject.undoneOrRedoneSteps = steps;
            setTimeInTicks(sequenceObject.theSequence.at(sequenceObject.undoneOrRedoneSteps[0]).getTimeStamp());
//            inUndoRedo = true;
            sendSynchronousChangeMessage();
            changeMessageType = CHANGE_MESSAGE_NONE;
        }
        else
            sequenceObject.undoneOrRedoneSteps.clear();
    }

    bool getNoteActivity(int step)
    {
        const double ts = sequenceObject.theSequence.at(step).getTimeStamp();
        const int index = sequenceObject.targetNoteTimes.indexOf(ts);
        if (index>=0)
            return true;
        else
            return false;
    }

    inline void setAsTargetNote(int step)
    {
        const double ts = sequenceObject.theSequence.at(step).getTimeStamp();
        const int index = sequenceObject.targetNoteTimes.indexOf(ts);
        if (index==-1)
            sequenceObject.targetNoteTimes.add(ts);
    }
    
    inline void setAsNonTargetNote(int step)
    {
        const double ts = sequenceObject.theSequence.at(step).getTimeStamp();
        const int index = sequenceObject.targetNoteTimes.indexOf(ts);
        if (index>-1)
            sequenceObject.targetNoteTimes.remove(index);
    }
    void setCopyOfSelectedNotes(Array<int> sel)
    {
        copyOfSelectedNotes = sel;
    }
    Array<int> copyOfSelectedNotes;
    
    void setListenSequence(double startTime, double endTime, Array<int> tracks)
    {
        listenSequence.clear();
        std::vector<NoteWithOffTime> *theSequence = sequenceObject.getSequence();
        for (int step=0; step<theSequence->size(); step++)
        {
            if (startTime <= theSequence->at(step).getTimeStamp() && theSequence->at(step).getTimeStamp()<=endTime
                && (tracks.size()==0||tracks.contains(theSequence->at(step).getTrack())))
            {
                NoteWithOffTime onMsg = theSequence->at(step);
                NoteWithOffTime offMsg = onMsg;
                offMsg.setTimeStamp(onMsg.offTime);
                offMsg.setVelocity(0);
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
    double variableTempoRatio; // variableTempoRatio = variableTimeIncrement/curTimeIncrement
    
    bool appIsActive = true;
    double getStartTimeOfNextStep()
    {
        int step;
        for (step=currentSeqStep;step<sequenceObject.theSequence.size();step++)
        {
            if (sequenceObject.theSequence[step].triggeredBy==-1)
                break;
        }
        return sequenceObject.theSequence[step].getTimeStamp();
    }
    
private:
    //==============================================================================
    std::vector<NoteWithOffTime> listenSequence;
    double listenStep;
    MidiOutput *midiOutput;
    MidiDestination midiDestination;
    bool notesEditable; //Whether notes can be edited in the viewer
    void sendMidiMessage(MidiMessage msg)
    {
        if (midiDestination==MidiDestination::internalSynth)
        {
            msg.setTimeStamp(99.0);
            synthMessageCollector.addMessageToQueue (msg); //<<<<<<<<<<<<<<< Add more
        }
        else if (midiDestination==MidiDestination::output)
            midiOutput->sendMessageNow(msg); //<<<<<< Use this to directly send midi
    }
    double timeInTicks = -1;
    int leadTimeInTicks; //How much space in ticks to allow to left of the ztl in viewer window

    double metTimeInTicks;  //Metronome timer - each time is counts to current ppq the metronome ticks and the met timer resets to 0.
    bool metronomeLighted;
//    double accompTimeInTicks;
    bool singleStep = false; //Indicates whether time advanced by timer or single step to next note
    int sequenceReadHead = 0;
    int currentSeqStep;
    
    int nextSustainStep;
    int nextSoftStep;
    int currentMeasure; //Used in keyboard navigation of measures and beats
    int meas; //Used in processBlock to track measure and pass current measure to Viewer paint() while playing
    int currentBeat; //Used in processBlock track beats for flashing beat indicator & metronome
//    double beatTickCounter; //Used to count ticks when transport is running even when notes are not being played
    bool autoPlaying;
    double autoPlayStartTime;
    bool panic; //If set true, sends all notes off in next call of processBlock
    double prevNoteOnLag;
    double prevTimeInTicks;
    int currentNote, lastNoteValue;
    unsigned timeInSamples;
    int latestPlayedSequenceStep;
    bool prevTis;  //Used to detect rewind
    int duplicateNote;
    AudioPlayHead::CurrentPositionInfo playHeadInfo;
    int sampleRate;
    Array<MidiMessage> exprEvents;//SC
    Array<int, CriticalSection> onNotes; //SC//Seq numbers of on notes
    double loopStartTick;
    int loopEndStep;
    double loopEndTickOffset;  //Number of ticks past loopEndStep that the time should be returned to loopStartTick.
    double loopEndTick; //When loopEndStep is played, loopEndTick is set to (timeStamp of loopEndStep) + loopEndTickOffset
    bool loopPending = false;
    std::deque<int> scheduledNotes;
    
    double timerIntervalInMS; //SC//Length of ticks in ms to provide a given tempo assuming a ppq of 96
    double timeIncrement;
    double variableTimeIncrement;//Amount to increase time at each tick, based on ppq of 96.Adjusted based on actual ppq of this midi file.
    class MyUndoManager : public UndoManager
    {
    public:
        MyUndoManager ()
        {
            
        }
        ~MyUndoManager ()
        {
            
        }
        bool undo ()
        {
            inUndo = true;
            bool result = UndoManager::undo();
            inUndo = false;
            return result;
        }
        
        bool redo ()
        {
            inRedo = true;
            bool result = UndoManager::redo();
            inRedo = false;
            return result;
        }
        
        bool inUndo;
        bool inRedo;
    };
    
public:
    MyUndoManager *undoMgr;
    
    class ActionSetNoteActivity : public UndoableAction
    {
        MIDIProcessor& proc;
        
    public:
        ActionSetNoteActivity(MIDIProcessor& _proc, bool setNotesActive, Array<int> stps) : proc(_proc)
        {
            setActive = setNotesActive;
            steps = stps;
        }
        ~ActionSetNoteActivity()
        {
        }
        bool perform()
        {
            prevValues = proc.setNoteListActivity(setActive, steps);
            return true;
        }
        bool undo()
        {
            proc.setIndividualNotesActivity(prevValues);
            return true;
        }
    private:
        bool setActive;
        Array<int> steps;
        Array<Sequence::StepActivity> prevValues;
    };
    
    class ActionChain : public UndoableAction
    {
        MIDIProcessor& proc;
        
    public:
        ActionChain(MIDIProcessor& _proc, double _interval, Array<int> _selection) : proc(_proc)
        {
            if (_interval>0)
            {
                inverval = _interval;
                proc.sequenceObject.currentChainingInterval = _interval;
            }
            else
                inverval = proc.sequenceObject.currentChainingInterval;
            selection = _selection;
        }
        ~ActionChain()
        {
        }
        bool perform()
        {
            prevValues = proc.chainCommand(selection, inverval);
            return true;
        }
        bool undo()
        {
            proc.setIndividualNotesActivity(prevValues);
            return true;
        }
    private:
        double inverval;
        Array<int> selection;
        Array<Sequence::StepActivity> prevValues;
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIProcessor)
};
#endif  // PLUGINPROCESSOR_H_INCLUDED
