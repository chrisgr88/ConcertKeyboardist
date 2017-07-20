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

class NoteWithOffTime : public MidiMessage
{
public:
    NoteWithOffTime(int trk, int byte1, int byte2, int byte3, double onTime, double offT) :
    MidiMessage(byte1,byte2,byte3,onTime),
//    pRecordsWithEdits(NULL),
    track(trk),
    offTime(offT),
    scheduledOnTime(0),
    scheduledOffTime(0),
    adjustedVelocity(0),
    firstInChain(-1),
    triggers(-1),
    triggeredBy(-1),
    chainTrigger(-1),
    highestVelocityInChain(0),
    triggeredNote(false),
    triggeredOffNote(false),
    autoplayedNote(false),
    noteOffNow(false),
    sustaining(false),
    rectBar(-1),
    rectHead(-1),
    triggeringExprNote(-1),
    selected(false),
    chordTopStep(-2),
    muted(false),
    head(Rectangle<float>()),
    timetoNextNote(-1)
    {
    }
    
    NoteWithOffTime(int trk, MidiMessage msg, double offT) :
    MidiMessage(msg),
//    pRecordsWithEdits(NULL),
    track(trk),
    offTime(offT),
    scheduledOnTime(0),
    scheduledOffTime(0),
    adjustedVelocity(0),
    firstInChain(-1),
    triggers(-1),
    triggeredBy(-1),
    chainTrigger(-1),
    highestVelocityInChain(-1),
    triggeredNote(false),
    triggeredOffNote(false),
    autoplayedNote(false),
    noteOffNow(false),
    sustaining(false),
    rectBar(-1),
    rectHead(-1),
    triggeringExprNote(-1),
    selected(false),
    chordTopStep(-2),
    muted(false),
    head(Rectangle<float>()),
    timetoNextNote(-1)
    {
    }
    
    NoteWithOffTime(NoteWithOffTime const &note) :
    MidiMessage(note),
//    pRecordsWithEdits(note.pRecordsWithEdits),
    track(note.track),
    offTime(note.offTime),
    scheduledOnTime(note.scheduledOnTime),
    scheduledOffTime(note.scheduledOffTime),
    adjustedVelocity(note.adjustedVelocity),
    firstInChain(note.firstInChain),
    triggers(note.triggers),
    triggeredBy(note.triggeredBy),
    chainTrigger(note.chainTrigger),
    highestVelocityInChain(note.highestVelocityInChain),
    triggeredNote(note.triggeredNote),
    triggeredOffNote(note.triggeredOffNote),
    autoplayedNote(note.autoplayedNote),
    noteOffNow(note.noteOffNow),
    sustaining(note.sustaining),
    rectBar(note.rectBar),
    rectHead(note.rectHead),
    triggeringExprNote(note.triggeringExprNote),
    selected(note.selected),
    chordTopStep(note.chordTopStep),
    muted(note.muted),
    head(note.head),
    timetoNextNote(note.timetoNextNote)
    {
    }
    
    NoteWithOffTime() :
    MidiMessage(),
//    pRecordsWithEdits(NULL),
    track(0),
    offTime(0),
    scheduledOnTime(0),
    scheduledOffTime(0),
    adjustedVelocity(0),
    firstInChain(-1),
    triggers(-1),
    triggeredBy(-1),
    chainTrigger(-1),
    highestVelocityInChain(-1),
    triggeredNote(false),
    triggeredOffNote(false),
    autoplayedNote(false),
    noteOffNow(false),
    sustaining(false),
    rectBar(-1),
    rectHead(-1),
    triggeringExprNote(-1),
    selected(false),
    chordTopStep(-2),
    muted(false),
    head(Rectangle<float>()),
    timetoNextNote(-1)
    {
    }
    
    ~NoteWithOffTime()
    {
    };
    
    void selectStep(bool sel)
    {
        selected = sel;
    }
    bool isSelected()
    {
        return selected;
    }
    bool invertSelected()
    {
        selected = !selected;
        return selected;
    }
    
    int getTrack() {return track;}
    
    bool operator< (const NoteWithOffTime& note2) const
    {
        if (getTimeStamp()==note2.getTimeStamp())
            return getNoteNumber()>note2.getNoteNumber();
        else
            return getTimeStamp()<note2.getTimeStamp();
    }
    
    
//    void changeTimeStamp(double newTimeStamp)
//    {
//        pRecordsWithEdits->at(track).at(editedMessageIndex).setTimeStamp(newTimeStamp);
//    }
//    double getTimeStamp() const
//    {
//        return pRecordsWithEdits->at(track).at(editedMessageIndex).getTimeStamp();
//    }
    inline double getOriginalTimeStamp() const
    {
        return getTimeStamp();
    }
    void changeTimeStamp(double newTS)
    {
        setTimeStamp(newTS);
    }
    
    void setOffTime(double newOffTime)
    {
        offTime = newOffTime;
    }
    double getOffTime()
    {
        return offTime;
    }

    void changeVelocity(float newVelocity)
    {
//        pRecordsWithEdits->at(track).at(editedMessageIndex).setVelocity(newVelocity);
        setVelocity(newVelocity);
    }
//    uint8 getVelocity() const
//    {
//        return pRecordsWithEdits->at(track).at(editedMessageIndex).getVelocity();
//    }
//    float getFloatVelocity() const
//    {
//        return pRecordsWithEdits->at(track).at(editedMessageIndex).getFloatVelocity();
//    }
    uint8 getOriginalVelocity() const
    {
        return getVelocity();
    }
    float getOriginalFloatVelocity() const
    {
        return getFloatVelocity();
    }
    
    void changeNoteNumber(int newNewNoteNumber)
    {
//        pRecordsWithEdits->at(track).at(editedMessageIndex).setNoteNumber(newNewNoteNumber);
        setNoteNumber(newNewNoteNumber);
    }
//    int getNoteNumber() const
//    {
//        return pRecordsWithEdits->at(track).at(editedMessageIndex).getNoteNumber();
//    }
    int getOriginalNoteNumber() const
    {
        return getNoteNumber();
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
//    std::vector<std::vector<MidiMessage>> *pRecordsWithEdits;
//    int editedMessageIndex; //Index into array of MidiMessages that hold values if edited from what was originally loaded
    MidiMessage *pMidiMessage;
    void setPMidiMessage(MidiMessage *pMsg)
    {
        pMidiMessage = pMsg;
    }
    MidiMessage *getPMidiMessage()
    {
        return pMidiMessage;
    }
    
    int track;
    double offTime;
    double scheduledOnTime; // Starting time assigned to steps when they are turned on by the scheduler based on the original note's time stamp and off time, adusted by the actual timeInTicks that the chainTrigger is triggered by an expr note.
    double scheduledOffTime; // Ending time assigned to steps when they are turned on by the scheduler based on the original note's time stamp and off time, adusted by the actual timeInTicks that the chainTrigger is triggered by an expr note.
    uint8 adjustedVelocity; //Computed at scheduling time.  May be based on the expr note velocity and secondary track vs primary track velocity.
    int firstInChain; // First step in a chain.  A chain ends at the first step whose start time is more than chainingInterval from previous step's start time.  Steps in the score are sorted by timeStamp (and then in descending order of note number).  This means that all steps in a chain are contiguous in the score.  Every step has a firstInChain property including the firstInChain itself.
    int triggers; //The step that this note triggers.  Set to -1 if last in group.
    int triggeredBy; //The that step that directly triggers this note.  Set to -1 if first in group.
    int chainTrigger; //The shortest note starting at the SAME time as the firstInChain. There may be other longer notes starting at the exact same time as the chainTrigger. The chainTrigger is not necessarily the firstInChain. Every step has a chainTrigger property including the chainTrigger itself.  A step numbered the same as its chainTrigger property is definitely a group trigger step.
    int highestVelocityInChain; //Used for display of velocity graph.
    bool triggeredNote; //triggeredNotes are played when the chain trigger is played.  They are steps that start no later than the triggeredNoteLimit from the chainTrigger.  Unless they are triggeredOffNotes they are held at least their full duration but are extended if the chainTrigger is held beyond their scheduled end time, in which case they are held until the chainTrigger is released.
    bool triggeredOffNote; //triggeredOffNotes are triggeredNotes that are forced off when their chainTrigger turns off.  triggeredOffNotes are defined as notes that end before the END of the next chainTrigger note.
    bool autoplayedNote; //Notes that start more than triggeredNoteLimit after the chainTrigger.
    bool noteOffNow; //Set true to request scheduler to end note immediately. Then reset to false.
    bool sustaining;  //Set to true for a note whose expr note has turned off before the note itself was scheduled to turn off.  Otherwise false.
    int rectBar; //GL Rect number of note bar
    int rectHead; //GL Rect number of note head
    uint8 triggeringExprNote; //The expr controller note that caused this note to be on
    bool selected;
    int chordTopStep; //Step of other than highest note in a group of simultaneous notes. For highest is -1, and if not in chord.
    bool muted;
    Rectangle<float> head;
    double timetoNextNote;
};
#endif  // NOTEWITHOFFTIME_H_INCLUDED
