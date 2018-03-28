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
    public ChangeListener,
    public AudioPlayHead
{
public:
    //==============================================================================
    MIDIProcessor();
    ~MIDIProcessor();
    CriticalSection audioProcessing;

    //==============================================================================
    void hiResTimerCallback() override;
    
    Sequence sequenceObject;
    
    void changeListenerCallback (ChangeBroadcaster* broadcaster) override;
    void timerCallback (int timerID) override;
    
    bool getCurrentPosition (CurrentPositionInfo& result)  override;
    
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
        sendChangeMessage();
//        accompTimeInTicks = time;
    }
    void setLeadTimeInTicks(int ticks)
    {
        leadTimeInTicks = ticks;
    }
    
    void loadFromUserSpecifiedFile ()
    {
#if JUCE_IOS
        auto ckDocs = File::getSpecialLocation(File::currentApplicationFile);
        FileChooser chooser ("Open Sequence", ckDocs, "*.ckf;*.mid", true);
#else
        FileChooser chooser ("Open Sequence",File(), "*.ckf;*.mid", true);
#endif
        if (chooser.browseForFileToOpen())
        {
            File file = chooser.getResult();
            if (!file.isDirectory())
            {
                String fn = file.getFullPathName();
//                sequenceObject.setScoreFile(file);
                
                sequenceObject.loadDocument(File(fn));//This is not "loadSequence" but triggers loading it with loadSequence
            }
        }
        //return Result::fail (TRANS("User cancelled"));
    }
    void loadSpecifiedFile (File file)//const bool showMessageOnFailure)
    {
        if (sequenceObject.saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
        {
            undoMgr->clearUndoHistory();
            sequenceObject.loadDocument(file);
        }
    }
    
    void buildSequenceAsOf (Sequence::LoadType type, Sequence::Retain retainEdits, double time)
    {
//        std::cout << "entering buildSequenceAsOf \n";
        HighResolutionTimer::stopTimer();
        
        if (sequenceObject.loadSequence(type, retainEdits))
        {
            //Clear the  "hungLoadingPreviousFile" flag.
            getAppProperties().getUserSettings()->setValue ("deadMansPedal", "");
            rewind(time);
        }
        else
        {
            AlertWindow::showNativeDialogBox("Warning", "Failed to Load File", false);
        }
        
        HighResolutionTimer::startTimer(timerIntervalInMS);
    }

    bool isPlaying;
    bool isListening; //True means  we are listening to a selection
    double startListenTime;
    double endListenTime;
    double lastStartTime;
    bool pauseClock; //Used to halt increasing timeInTicks if next noteOn time lags too much behind timeInTick
    bool waitingForFirstNote; //If set, time does not increment.  This is set false when the first expr note is played.
    void rewind (double time, bool sendChangeMessages=true);
    void listenToSelection();
    void endListen();
#define CHANGE_MESSAGE_NONE -1
#define CHANGE_MESSAGE_REWIND 0
#define CHANGE_MESSAGE_TWEEN 1
#define CHANGE_MESSAGE_START_PLAYING 2
#define CHANGE_MESSAGE_STOP_PLAYING 3
#define CHANGE_MESSAGE_BEAT_CHANGED 4
#define CHANGE_MESSAGE_MEASURE_CHANGED 5
#define CHANGE_MESSAGE_REPAINT_VIEWER 6
#define CHANGE_MESSAGE_TEMPO_CHANGE 7
#define CHANGE_MESSAGE_RETURN_BASELINE 8
#define CHANGE_MESSAGE_UNDO 9
    int changeMessageType; //Set before sending a change message - used by viewer to choose desired action
    //Set before sending a CHANGE_MESSAGE_TWEEN
    double tweenTo;
    double transitionTime;
    
    void tweenMove (double targetTime, double transitionTime);
    
    void playableStepForwardBack (bool direction);
    int getMeasure(double horizontalShift);
    int getZTLTime(double horizontalShift);
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
    Sequence::Bookmark atBookmark (); //Returns exact bookmark time ZTL is close to bookmark, else -1
    void addRemoveBookmark (int action, bool tempoChange=false, double tempoScale = 0); //At current ZTL position
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

    void play (bool ply, String fromWhere);  //Set to >= 0 reset viewer. All notes up to this step are marked as played.

    double getSequenceReadHead()
    {
        return sequenceReadHead;
    }

    int startStep, endStep;
    double resetToTime;
    
    bool noteIsOn(int seqStep)
    {
        return onNotes.contains(seqStep);
    }
    
    void catchUp(bool sendChangeMessages = false);
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
    void messageCollectorReset(const double rate)
    {
        synthMessageCollector.reset(rate);
        messageCollectorsAreReset = true;
        pluginMessageCollector->reset(rate);
    }
    
#define FIFO_SIZE 50
    int noteOnOffFifoBuffer [FIFO_SIZE];
    AbstractFifo noteOnOffFifo;
    bool messageCollectorsAreReset;
    MidiMessageCollector *pluginMessageCollector;
    MidiOutput *defaultMidiOutput;   //The midi output  chosen, if any,  in the midi setup dialog
    
    void prepareToSave() { }
    
    bool resetViewer; //Used as flag to tell viewer in a change notification to reset itself
    
    double mostRecentNoteTime;
    double earliness;
    double leadLag;
    double leadLagForPlayedTargetNote;
    double prevLeadLag;
    double noteOnLag;
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
    }
    inline bool getNotesEditable()
    {
        return notesEditable;
    }
    void increaseTempo()
    {
        variableTempoRatio *= 1.1;
        std::cout << "increase variableTimeIncrement  "<<variableTimeIncrement<<"\n" ;
        changeMessageType = CHANGE_MESSAGE_TEMPO_CHANGE;
        sendSynchronousChangeMessage();
    }
    void decreaseTempo()
    {
        variableTempoRatio *= 1.0/1.1;
        std::cout << "decrease variableTimeIncrement  "<<variableTimeIncrement<<"\n"; 
        changeMessageType = CHANGE_MESSAGE_TEMPO_CHANGE;
        sendSynchronousChangeMessage();
    }
    void returnToBaseline()
    {
        std::cout << "returnToBaseline  "<<variableTimeIncrement<<"\n";
        changeMessageType = CHANGE_MESSAGE_RETURN_BASELINE;
        sendSynchronousChangeMessage();
    }
    void setTempoMultiplier(double value, double currentTime, bool documentReallyChanged);
    int lastPlayedSeqStep = -1; //Equal to the step of the target just before the next to be played
    int lastUserPlayedSeqStep = -1; //Previous value of lastPlayedSeqStep
    int lastPlayedNoteStep = -1; //The step of the last played note, even if its note a target note (for tracking tempo and measures)
    int lastPlayedTargetNoteTime= -1;
    int nextDueTargetNoteTime = -1;
    double getLastUserPlayedStepTime();
    Array<Sequence::StepActivity> setNoteListActivity(bool setNotesActive, Array<int> steps);
    Array<Sequence::PrevNoteTimes> timeHumanizeChords (Array<int> steps, String timeSpec);
    Array<Sequence::NoteVelocities> velocityHumanizeChords (Array<int> steps, String velSpec);
    enum PedalType {sustPedal, softPedal};
    void addPedalChange(PedalType pType);
    void deletePedalChange(PedalType pType);
    bool atPedalChange(PedalType pType);
    void createChord();
    void deleteChords(bool rebuild);
    void autoCreateChords(double maxLength); //Based on notes chained to target notes in selection, limited to maxLength
    Array<Sequence::StepActivity> chainCommand (Array<int> selection, double inverval);
    void humanizeChordNoteTimes ();
    void humanizeChordNoteVelocities ();
    void setIndividualNotesActivity (Array<Sequence::StepActivity> act); //Used only to restore activity after undo
    void setIndividualNoteTimes (Array<Sequence::PrevNoteTimes> prevTimes);
    void setIndividualNoteOffTimes (Array<Sequence::PrevNoteTimes> prevTimes);
    bool getNoteActivity(int step);
    Array<Sequence::NoteVelocities> changeNoteVelocities(Array<Sequence::NoteVelocities>);
    void restoreNoteVelocities(Array<Sequence::NoteVelocities>);
    Array<Sequence::PrevNoteTimes> changeNoteTimes(std::vector<std::shared_ptr<NoteWithOffTime>> notes, double time);
    Array<Sequence::PrevNoteTimes>  changeNoteOffTimes(std::vector<std::shared_ptr<NoteWithOffTime>> notes, double delta);
    Array<int> copyOfSelectedNotes;
    void setCopyOfSelectedNotes(Array<int> sel);
    void setListenSequence(double startTime, double endTime, Array<int> tracks);
    double variableTempoRatio; // variableTempoRatio = variableTimeIncrement/curTimeIncrement
    double prevTempo;
    bool fullPowerMode = true;
    int sampleRate;
    std::atomic_bool pauseProcessing;
    bool pluginEnabled;
    bool midiOutEnabled;
    MidiMessageCollector synthMessageCollector;
    int velocityFromBreath;
private:
    //==============================================================================

    std::vector<NoteWithOffTime> listenSequence;
    double listenStep;
#if JUCE_MAC || JUCE_IOS
    MidiOutput *ckMidiOutput;
#endif
    bool notesEditable; //Whether notes can be edited in the viewer
    
    void sendMidiMessage(MidiMessage msg)
    {
        double t = Time::getMillisecondCounterHiRes()*0.001;
        msg.setTimeStamp(t);
//        std::cout
//        << " note " << msg.getNoteNumber()
//        << " velocity " << (int) msg.getVelocity()
//        << "\n";
        if (midiOutEnabled)
        {
#if JUCE_MAC || JUCE_IOS
            ckMidiOutput->sendMessageNow(msg);
#endif
            if (defaultMidiOutput!=nullptr)
            {
                defaultMidiOutput->sendMessageNow(msg);
            }
        }
        if (pluginEnabled && messageCollectorsAreReset)
        {
            if (pluginMessageCollector)
                pluginMessageCollector->addMessageToQueue (msg);
        }
        if (messageCollectorsAreReset)
            synthMessageCollector.addMessageToQueue(msg);
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
    
    double prevExprNoteTick;
    double prevPrevExprNoteTick;
    void resetPrevNoteTimes()
    {
        prevExprNoteTick = -1000000;
        prevPrevExprNoteTick = -1000000;
    }
    
    double prevTimeInTicks;
    int currentNote, lastNoteValue;
    unsigned timeInSamples;
    int latestPlayedSequenceStep;
    bool prevTis;  //Used to detect rewind
    int duplicateNote;
    AudioPlayHead::CurrentPositionInfo playHeadInfo;
    Array<MidiMessage> exprEvents;//SC
    Array<int, CriticalSection> onNotes; //SC//Seq numbers of on notes
    double loopStartTick;
    int loopEndStep;
    double loopEndTickOffset;  //Number of ticks past loopEndStep that the time should be returned to loopStartTick.
    double loopEndTick; //When loopEndStep is played, loopEndTick is set to (timeStamp of loopEndStep) + loopEndTickOffset
    bool loopPending = false;
    std::deque<int> scheduledNotes;
    
    double timerIntervalInMS; //SC//Length of ticks in ms to provide a given tempo assuming a ppq of 960
    double timeIncrement;
    double variableTimeIncrement;//Amount to increase time at each tick, based on ppq of 960.Adjusted based on actual ppq of this midi file.
    
    class MyUndoManager : public UndoManager //MyUndoManager ============================================================
    {
    public:
        explicit MyUndoManager (int maxNumberOfUnitsToKeep = 30000,
                        int minimumTransactionsToKeep = 30)
        {
            UndoManager::UndoManager(maxNumberOfUnitsToKeep, minimumTransactionsToKeep);
        }
        ~MyUndoManager ()
        {
            
        }
        bool undo ()
        {
            inUndo = true;
            bool result = UndoManager::undo();
//            inUndo = false;
            return result;
        }
        
        bool redo ()
        {
            inRedo = true;
            bool result = UndoManager::redo();
//            inRedo = false;
            return result;
        }
        
        bool inUndo;
        bool inRedo;
    };
    
public:
    MyUndoManager *undoMgr;
    
    class ActionSetNoteActivity : public UndoableAction //ActionSetNoteActivity =============================================
    {
        MIDIProcessor& proc;
        
    public:
        ActionSetNoteActivity(MIDIProcessor& _proc, bool setNotesActive, Array<int> stps) : proc(_proc)
        {
            setActive = setNotesActive;
            steps = stps;
            
            selection.clear();
            for (int i=0; i<stps.size();i++)
                selection.push_back(proc.sequenceObject.theSequence.at(stps[i]));
        }
        ~ActionSetNoteActivity()
        {
        }
        bool perform()
        {
            prevValues = proc.setNoteListActivity(setActive, steps);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            return true;
        }
        bool undo()
        {
            proc.setIndividualNotesActivity(prevValues);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            proc.changeMessageType = CHANGE_MESSAGE_UNDO;
            proc.sendSynchronousChangeMessage();
            proc.changeMessageType = CHANGE_MESSAGE_NONE;
            return true;
        }
    private:
        bool setActive;
        Array<int> steps;
        std::vector<std::shared_ptr<NoteWithOffTime>> selection;
        Array<Sequence::StepActivity> prevValues;
    };
    
    class ActionChain : public UndoableAction      //ActionChain =============================================
    {
        MIDIProcessor& proc;
        
    public:
        ActionChain(MIDIProcessor& _proc, double _interval, Array<int> _selection) : proc(_proc)
        {
            selectedSteps = _selection;
            if (_interval>0)
            {
                inverval = _interval;
                proc.sequenceObject.chainingInterval = _interval;
            }
            else
                inverval = proc.sequenceObject.chainingInterval;
            
            selection.clear();
            for (int i=0; i<_selection.size();i++)
                selection.push_back(proc.sequenceObject.theSequence.at(_selection[i]));
        }
        ~ActionChain()
        {
        }
        bool perform()
        {
            prevValues = proc.chainCommand(selectedSteps, inverval);
            return true;
        }
        bool undo()
        {
            proc.setIndividualNotesActivity(prevValues);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            proc.changeMessageType = CHANGE_MESSAGE_UNDO;
            proc.sendSynchronousChangeMessage();
            proc.changeMessageType = CHANGE_MESSAGE_NONE;
            return true;
        }
    private:
        double inverval;
        std::vector<std::shared_ptr<NoteWithOffTime>> selection;
        Array<int> selectedSteps;
        Array<Sequence::StepActivity> prevValues;
    };

    class ActionChangeNoteTimes : public UndoableAction  //ActionChangeNoteTimes =============================================
    {
        MIDIProcessor& proc;
        
    public:
        ActionChangeNoteTimes(MIDIProcessor& _proc, double _delta,
                              std::vector<std::shared_ptr<NoteWithOffTime>> _selection) : proc(_proc)
        {
            delta = _delta;
            selection = _selection;
        }
        ~ActionChangeNoteTimes()
        {
        }
        bool perform()
        {
            prevValues = proc.changeNoteTimes(selection, delta);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            return true;
        }
        bool undo()
        {
            proc.setIndividualNoteTimes(prevValues);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            proc.changeMessageType = CHANGE_MESSAGE_UNDO;
            proc.sendSynchronousChangeMessage();
            proc.changeMessageType = CHANGE_MESSAGE_NONE;
            return true;
        }
    private:
        double delta;
        std::vector<std::shared_ptr<NoteWithOffTime>> selection;
        Array<Sequence::PrevNoteTimes> prevValues;
    };
    
    
    class ActionChangeNoteOffTimes : public UndoableAction  //ActionChangeNoteOffTimes =========================================
    {
        MIDIProcessor& proc;
        
    public:
        ActionChangeNoteOffTimes(MIDIProcessor& _proc, double _delta,
                              std::vector<std::shared_ptr<NoteWithOffTime>> _selection) : proc(_proc)
        {
            delta = _delta;
            selection = _selection;
        }
        ~ActionChangeNoteOffTimes()
        {
        }
        bool perform()
        {
            prevValues = proc.changeNoteOffTimes(selection, delta);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            return true;
        }
        bool undo()
        {
            proc.setIndividualNoteOffTimes(prevValues);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            proc.changeMessageType = CHANGE_MESSAGE_UNDO;
            proc.sendSynchronousChangeMessage();
            proc.changeMessageType = CHANGE_MESSAGE_NONE;
            return true;
        }
    private:
        double delta;
        std::vector<std::shared_ptr<NoteWithOffTime>> selection;
        Array<Sequence::PrevNoteTimes> prevValues;
    };
    
    class ActionTimeHumanizeChords : public UndoableAction   //ActionTimeHumanizeChords =============================================
    {
        MIDIProcessor& proc;
        
    public:
        ActionTimeHumanizeChords(MIDIProcessor& _proc, String _timeSpec , Array<int> _selection) : proc(_proc)
        {
            selectedSteps = _selection;
            timeSpec = _timeSpec;
            
            selection.clear();
            for (int i=0; i<_selection.size();i++)
                selection.push_back(proc.sequenceObject.theSequence.at(_selection[i]));
        }
        ~ActionTimeHumanizeChords()
        {
        }
        bool perform()
        {
            prevValues = proc.timeHumanizeChords(selectedSteps, timeSpec);
            return true;
        }
        bool undo()
        {
            proc.setIndividualNoteTimes(prevValues);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            proc.changeMessageType = CHANGE_MESSAGE_UNDO;
            proc.sendSynchronousChangeMessage();
            proc.changeMessageType = CHANGE_MESSAGE_NONE;
            return true;
        }
    private:
        String timeSpec = String();
        std::vector<std::shared_ptr<NoteWithOffTime>> selection;
        Array<int> selectedSteps;
        Array<Sequence::PrevNoteTimes> prevValues;
    };
    
    class ActionVelocityHumanizeChords : public UndoableAction   //ActionVelocityHumanizeChords ====================================
    {
        MIDIProcessor& proc;
        
    public:
        ActionVelocityHumanizeChords(MIDIProcessor& _proc, String _velSpec , Array<int> _selection) : proc(_proc)
        {
            selectedSteps = _selection;
            velSpec = _velSpec;
            
            selection.clear();
            for (int i=0; i<_selection.size();i++)
                selection.push_back(proc.sequenceObject.theSequence.at(_selection[i]));
        }
        ~ActionVelocityHumanizeChords()
        {
        }
        bool perform()
        {
            prevValues = proc.velocityHumanizeChords(selectedSteps, velSpec);
            return true;
        }
        bool undo()
        {
            proc.restoreNoteVelocities(prevValues);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            proc.changeMessageType = CHANGE_MESSAGE_UNDO;
            proc.sendSynchronousChangeMessage();
            proc.changeMessageType = CHANGE_MESSAGE_NONE;
            return true;
        }
    private:
        String velSpec  = String();
        std::vector<std::shared_ptr<NoteWithOffTime>> selection;
        Array<int> selectedSteps;
        Array<Sequence::NoteVelocities> prevValues;
    };
    
    class ActionChangeVelocities : public UndoableAction   //ActionChangeVelocities =============================================
    {
        MIDIProcessor& proc;
        
    public:
        ActionChangeVelocities(MIDIProcessor& _proc, Array<Sequence::NoteVelocities> _newVelocities) : proc(_proc)
        {
            newVelocities = _newVelocities;
            selection.clear();
            for (int i=0; i<_newVelocities.size();i++)
                selection.push_back(_newVelocities[i].note);
        }
        ~ActionChangeVelocities()
        {
        }
        bool perform()
        {
            prevVelocities = proc.changeNoteVelocities(newVelocities);
            return true;
        }
        bool undo()
        {
            proc.restoreNoteVelocities(prevVelocities);
            proc.sequenceObject.selectionToRestoreForUndoRedo = selection;
            proc.changeMessageType = CHANGE_MESSAGE_UNDO;
            proc.sendSynchronousChangeMessage();
            proc.changeMessageType = CHANGE_MESSAGE_NONE;
            return true;
        }
    private:
        Array<Sequence::NoteVelocities> newVelocities;
        std::vector<std::shared_ptr<NoteWithOffTime>> selection;
        Array<Sequence::NoteVelocities> prevVelocities;
    };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIProcessor)
};
#endif  // PLUGINPROCESSOR_H_INCLUDED
