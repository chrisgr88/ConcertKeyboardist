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
#include <iostream> 
#include <map>

class NoteWithOffTime : public MidiMessage
{
public:
    NoteWithOffTime(int trk, int byte1, int byte2, int byte3, double onTime, double offTime) :
        MidiMessage(byte1,byte2,byte3,onTime),
        track(trk),
        offTimeStamp(offTime)
    {
    }
    
    NoteWithOffTime(int trk, MidiMessage msg, double offTime) :
        MidiMessage(msg),
        track(trk),
        offTimeStamp(offTime)
    {
    }
    
    ~NoteWithOffTime()
    {        
    };
    
    int getTrack() {return track;}
    
    bool operator< (const NoteWithOffTime& note2) const {return getOnTime()<note2.getTimeStamp();}
    double getOnTime() const {return getTimeStamp();}
    double getOffTime() const {return offTimeStamp;}
    
private:
    int track;
    double offTimeStamp;
};

//==============================================================================
/**
*/
class ReExpressorAudioProcessor  : public AudioProcessor, public ChangeBroadcaster, private HighResolutionTimer
{
public:
    //==============================================================================
    ReExpressorAudioProcessor();
    ~ReExpressorAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void updatePlayHeadFromHost();
    void printMessage(MidiMessage const& message);
    void releaseResources() override;
    void hiResTimerCallback() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool setPreferredBusArrangement(bool isInput, int bus, const AudioChannelSet& preferredSet) override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;
    
    double getTimeInTicks();
    int getLatestPlayedSequenceStep() {
        return lastPlayedSeqStep;
    }
    double getPPQ();
    Array<MidiMessage>  getTimeSigInfo();
    //Gets next available notes for user to play
    Array<MidiMessage> getAvailableNotes(int hostTimeInTicks);//Notes available to play at given time
    
    std::vector<NoteWithOffTime> *getSequence();
    std::vector<NoteWithOffTime> getNotesInTimeRange(double minTimeStamp, double maxTimeStamp);
    Array<MidiMessage> getCurrentTimeSig(double timeStamp);
    SortedSet<int> getNotesUsed(int &minNote, int &maxNote);

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    bool isPlaying;
//    void fileOpen(String fileName);
    void  loadSequence(String fileName);
    void rewind();
    bool sequenceChanged;
    inline bool getSequenceChanged() {return sequenceChanged;}
    void setSequenceChanged(bool changed) {
        sequenceChanged = changed;
    }
    void setTempo(double tem)
    {
        clockIntervalInMS = 1000.0*(60.0/tem)/96.0; //Milliseconds per tick at a ppq of 96
        tempo = tem;
        if (isTimerRunning())
            startTimer(clockIntervalInMS);
    }
    double getTempo()
    {
        return tempo;
    }


private:
    //==============================================================================
    double timeInTicks = 0;
    double timeIncrement; //Amount to increase time at each tick, based on ppq og 96.  Adjusted based on actual ppq of this midi file.
    double clockIntervalInMS; //Length of ticks in ms to provide a given tempo assuming a ppq of 96
    bool singleStep = false; //Indicates whether time advanced by timer or single step to next note
    //Notes outside this range are available as "user expression" notes
    int minSeqNote = 30;
    int maxSeqNote = 100;
    
    Array<MidiMessage> onNotes;
    std::vector<NoteWithOffTime> theSequence;
    std::vector<NoteWithOffTime> notesPendingOff;
    Array<MidiMessage> timeSigChanges;
    Array<MidiMessage> tempoChanges;
    
    int sequenceReadHead = 0;  //One tick past the most recently played sequence note.  Or zero at start.
    int lastPlayedSeqStep;
    int noteAvailablityWindow = 96;  //How far after their original time that notes are available (in ticks)
    double tempo;
    int quantizationInterval = 24;  //How much original note on-times can differ before the notes aren't triggered simultaneously
    double bpm;
    double ppq;
    
    AudioParameterFloat* speed;
    int currentNote, lastNoteValue;
    unsigned blockSize;
    unsigned timeInSamples;
    int latestPlayedSequenceStep;
    
    AudioPlayHead::CurrentPositionInfo playHeadInfo;
    
    int noteCounter = 60;
    
    typedef struct {
        uint8 chan;
        uint8 nn;
        uint8 vel; //Use 0 for note off
    } noteType;
    
    int sampleRate;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReExpressorAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
