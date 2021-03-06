/*
  ==============================================================================

    NoteWithOffTime.h
    Created: 18 Jan 2017 12:25:28pm
    Author:  ChrisGr

  ==============================================================================
*/
#ifndef NOTEWITHOFFTIME_H_INCLUDED
#define NOTEWITHOFFTIME_H_INCLUDED

class ControllerMessage : public MidiMessage
{
public:
    ControllerMessage(int trk, MidiMessage msg) :
    MidiMessage(msg),
    track(trk)
    {}
    ~ControllerMessage()
    {};
    bool operator< (const ControllerMessage& msg2) const
    {
        return getTimeStamp()<msg2.getTimeStamp();
    }
    int track;
};

class NoteWithOffTime// : public MidiMessage
{
public:
    
    NoteWithOffTime(int trk, double tStamp, int chan, int noteNum, float vel, double offT) :
    track(trk),
    indexInTrack(-1),
    timeStamp(tStamp),
//    originalIndexInTrack(-1),
    channel(chan),
    noteNumber(noteNum),
    velocity(vel),
    originalVelocity(vel),
    offTime(offT),
    scheduledOnTime(0),
    scheduledOffTime(0),
    adjustedVelocity(0.0f),
    currentStep(-1),
    firstInChain(-1),
    triggers(-1),
    triggeredBy(-1),
    chainTrigger(-1),
    targetNote(false),
    highestVelocityInChain(-1),
    inChord(false),
    triggeredNote(false),
    triggeredOffNote(false),
    autoplayedNote(false),
    noteOffNow(false),
    sustaining(false),
    rectBar(-1),
    rectHead(-1),
    triggeringExprNote(-1),
    isSelected(false),
    chordTopStep(-2),
    muted(false),
    head(Rectangle<float>()),
    bar(Rectangle<float>()),
    timetoNextNote(-1),
    chordIndex(-1),
    noteIndexInChord(-1)
    {
    }
    
    NoteWithOffTime(NoteWithOffTime const &note) :
    track(note.track),
    indexInTrack(note.indexInTrack),
    timeStamp(note.timeStamp),
//    originalIndexInTrack(note.originalIndexInTrack),
    channel(note.channel),
    noteNumber(note.noteNumber),
    velocity(note.velocity),
    originalVelocity(note.originalVelocity),
    offTime(note.offTime),
    scheduledOnTime(note.scheduledOnTime),
    scheduledOffTime(note.scheduledOffTime),
    adjustedVelocity(note.adjustedVelocity),
    currentStep(note.currentStep),
    firstInChain(note.firstInChain),
    triggers(note.triggers),
    triggeredBy(note.triggeredBy),
    chainTrigger(note.chainTrigger),
    targetNote(note.targetNote),
    highestVelocityInChain(note.highestVelocityInChain),
    inChord(note.inChord),
    triggeredNote(note.triggeredNote),
    triggeredOffNote(note.triggeredOffNote),
    autoplayedNote(note.autoplayedNote),
    noteOffNow(note.noteOffNow),
    sustaining(note.sustaining),
    rectBar(note.rectBar),
    rectHead(note.rectHead),
    triggeringExprNote(note.triggeringExprNote),
    isSelected(note.isSelected),
    chordTopStep(note.chordTopStep),
    muted(note.muted),
    head(note.head),
    bar(note.bar),
    timetoNextNote(note.timetoNextNote),
    chordIndex(note.chordIndex),
    noteIndexInChord(note.noteIndexInChord)
    {
    }
    
    NoteWithOffTime() :
    track(0),
    indexInTrack(-1),
    timeStamp(0.0),
//    originalIndexInTrack(-1),
    channel(1),
    noteNumber(0),
    velocity(0),
    originalVelocity(0),
    offTime(0),
    scheduledOnTime(0),
    scheduledOffTime(0),
    adjustedVelocity(0.0f),
    currentStep(-1),
    firstInChain(-1),
    triggers(-1),
    triggeredBy(-1),
    chainTrigger(-1),
    targetNote(false),
    highestVelocityInChain(-1),
    inChord(false),
    triggeredNote(false),
    triggeredOffNote(false),
    autoplayedNote(false),
    noteOffNow(false),
    sustaining(false),
    rectBar(-1),
    rectHead(-1),
    triggeringExprNote(-1),
    isSelected(false),
    chordTopStep(-2),
    muted(false),
    head(Rectangle<float>()),
    bar(Rectangle<float>()),
    timetoNextNote(-1),
    chordIndex(-1),
    noteIndexInChord(-1)
    {
    }
    
    ~NoteWithOffTime()
    {
    };
//    void selectStep(bool sel)
//    {
//        isSelected = sel;
//    }
//    bool isSelected()
//    {
//        return isSelected;
//    }
//    bool invertSelected()
//    {
//        isSelected = !isSelected;
//        return isSelected;
//    }
    
    inline void setTimeStamp(double ts)
    {
//        if (ts>200000 || ts<-200000)
//            std::cout << "setTimeStamp "<<currentStep<<" "<<ts<<"\n";
        timeStamp =  ts;
    }
    
    inline double getTimeStamp()
    {
        //        if (currentStep<15)
        //            std::cout << "setVelocty "<<currentStep<<" "<<vel*127<<"\n";
        return timeStamp;
    }
    
    inline void setOfftime(double time)
    {
//        if (time<0 || time>200000)
//            std::cout << "setOfftime "<<currentStep<<" "<<time<<"\n";
        offTime =  time;
    }
    
    inline double getOffTime()
    {
//        if (offTime<0 || offTime>200000)
//            std::cout << "getOffTime "<<currentStep<<" "<<offTime<<"\n";
        return offTime;
    }
    
    inline void setVelocity(float vel)
    {
//        if (currentStep<15)
//            std::cout << "setVelocty "<<currentStep<<" "<<vel*127<<"\n";
        velocity = vel;
    }
    inline float getVelocity()
    {
        //        if (currentStep<15)
        //            std::cout << "setVelocty "<<currentStep<<" "<<vel*127<<"\n";
        return velocity;
    }
    
    int getTrack() {return track;}
    
    bool operator< (const NoteWithOffTime& note2) const
    {
        if (timeStamp==note2.timeStamp)
            return noteNumber>note2.noteNumber;
        else
            return timeStamp<note2.timeStamp;
    }
    
    /*
     Overall behavior:
     - Chain is defined as a contiguous series of steps all with starting times separated by amount less than or equal to chainingInterval.
     - All notes in chain are added to scheduler when the chainTrigger is played.
     - All notes are started by the scheduler at their scheduledOnTime
     - triggeredNotes are turned off when chainTrigger is released
     - All other notes are ended by sheduler at their scheduledOffTime. (except possibly repeatedNotes)
     - If a step is marked as a repeatedNote the preceding step given by the repeatedNote property is tested for being on and that note is forced off before the new instance of that note is turned on.
     - The first note of a trill can only be a chainTrigger if there are no other notes in the chain.
     - In scanning a chain for analysis or scheduling we should skip all but the first note of a trill.
     
     Trills: Triggerable entities
     - A trill's notes are never triggeredNotes.
     - This means that a trill's notes can be scheduled to start and stop unaffected by the playing of simultaneous chainTriggers for normal note chains.
     - Trills are steps in the score file, and the steps are sorted by time tag, so we can no longer assume that a given chain's notes are contiguous. What are the implications of this?
     - Add a bool trill and int nextTrillStep properties to flag a trill step and indicate next step in this trill.  Use -1 to indicate last step.
     */
//   Index to this note's editable entry in track in allNotes[tracks[ ]].
    int track;
    int indexInTrack; //Index to this note's entry in its track in midiFile
    int timeStamp;
//    int originalIndexInTrack;
    int channel;
    int noteNumber;
    float velocity;
    float originalVelocity;  //As loaded from file
    double offTime;
    double scheduledOnTime; // Starting time assigned to steps when they are turned on by the scheduler based on the original note's time stamp and off time, adusted by the actual timeInTicks that the chainTrigger is triggered by an expr note.
    double scheduledOffTime; // Ending time assigned to steps when they are turned on by the scheduler based on the original note's time stamp and off time, adusted by the actual timeInTicks that the chainTrigger is triggered by an expr note.
    float adjustedVelocity; //Computed at scheduling time.  May be based on the expr note velocity and secondary track vs primary track velocity.
    int currentStep;
    int firstInChain; // First step in a chain.  A chain ends at the first step whose start time is more than chainingInterval from previous step's start time.  Steps in the score are sorted by timeStamp (and then in descending order of note number).  This means that all steps in a chain are contiguous in the score.  Every step has a firstInChain property including the firstInChain itself.
    int triggers; //The step that this note triggers.  Set to -1 if last in group.
    int triggeredBy; //The that step that directly triggers this note.  Set to -1 if first in group.
    int chainTrigger; //The shortest note starting at the SAME time as the firstInChain. There may be other longer notes starting at the exact same time as the chainTrigger. The chainTrigger is not necessarily the firstInChain. Every step has a chainTrigger property including the chainTrigger itself.  A step numbered the same as its chainTrigger property is definitely a group trigger step.
    bool targetNote; //Tag this step as a target note even if its time changes.  For use during sequence rebuilding.
    float highestVelocityInChain; //Used for display of velocity graph.
    bool inChord;
    bool triggeredNote; //triggeredNotes are played when the chain trigger is played.  They are steps that start no later than the triggeredNoteLimit from the chainTrigger.  Unless they are triggeredOffNotes they are held at least their full duration but are extended if the chainTrigger is held beyond their scheduled end time, in which case they are held until the chainTrigger is released.
    bool triggeredOffNote; //triggeredOffNotes are triggeredNotes that are forced off when their chainTrigger turns off.  triggeredOffNotes are defined as notes that end before the END of the next chainTrigger note.
    bool autoplayedNote; //Notes that start more than triggeredNoteLimit after the chainTrigger.
    bool noteOffNow; //Set true to request scheduler to end note immediately. Then reset to false.
    bool sustaining;  //Set to true for a note whose expr note has turned off before the note itself was scheduled to turn off.  Otherwise false.
    int rectBar; //GL Rect number of note bar
    int rectHead; //GL Rect number of note head
    uint8 triggeringExprNote; //The expr controller note that caused this note to be on
    bool isSelected;
    int chordTopStep; //Highest note in a group of notes grouped as a chord. For highest is -1, and if not in chord.
    bool muted;
    Rectangle<float> head;
    Rectangle<float> bar;
    double timetoNextNote;
    int chordIndex;  //Index in chords[ ] of this note's chord, if any. This may change depending on which tracks are active.
    int noteIndexInChord; //The current position of this note in the ChordDetail
};

#endif  // NOTEWITHOFFTIME_H_INCLUDED
