/*
  ==============================================================================

    Sequence.h
    Created: 31 Jan 2017 5:56:13am
    Author:  ChrisGr

  ==============================================================================
*/

#ifndef SEQUENCE_H_INCLUDED
#define SEQUENCE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "NoteWithOffTime.h"
#include <iostream>
#include <map>
#include <random>

const char* const filenameSuffix = ".ckf";
const char* const filenameWildcard = "*.ckf";
ApplicationCommandManager& getCommandManager();
ApplicationProperties& getAppProperties();

class Sequence  : public FileBasedDocument, public ActionBroadcaster
{
public:
    Sequence();
    ~Sequence();
    
    typedef struct StepActivity {
        int step;
        bool active;
    } StepActivity;
    
    typedef struct PrevNoteTimes {
        std::shared_ptr<NoteWithOffTime> note;
        double time;
    } PrevNoteTimes;
    
    typedef struct NoteVelocities {
        std::shared_ptr<NoteWithOffTime> note;
        float velocity;
    } ChangeVelocities;
    
    std::vector<std::shared_ptr<NoteWithOffTime>> theSequence;
    
    bool loadedCkfFile;
    bool alreadyChained; //Either loaded from ckf file or first chaining of midi file has been completed
    bool propertiesChanged = false;
    int minNote = 127;
    int maxNote = 0;
    
    String getDocumentTitle() override
    {
        return String(getLastDocumentOpened().getFileNameWithoutExtension());
    }
    
    /** This method should try to load your document from the given file.
     @returns a Result object to indicate the whether there was an error.
     */
    /*
     Opening a document:
     - triggered by the CommandIDs::fileOpen by the menu
     - the menu triggers calling midiProcessor.loadFromUserSpecifiedFile() in MidiProcessor
     - this brings up the file dialog
     - if the user chooses a file, loadDocument in Sequence is called. Passed file name is ignored as loadFromUserSpecifiedFile saves it
     - loadDocument() sends a synchronous change message back to MidiProcessor to actually load the file with buildSequenceAsOf ( )
     - buildSequenceAsOf does some settings and calls back to LoadSequence in Sequence object.
     */
    bool loadDoc = false; //Tells midiProcessor to do a load document when it gets a change message
//    bool inUndoOrRedo = false;
    std::vector<std::shared_ptr<NoteWithOffTime>> selectionToRestoreForUndoRedo; //Used to pass changed steps to MidiProcessor so it can restore the selection
    File fileToLoad;
    Result loadDocument (const File& file) override
    {
        fileToLoad = file;
        loadDoc = true;
        //The actual loading is done in midiProcessor
        sendSynchronousChangeMessage();
        propertiesChanged = true;
        return Result::ok();
    }
    
    /** This method should try to write your document to the given file.
     @returns a Result object to indicate the whether there was an error.
     */
    Result saveDocument (const File& file)  override
    {
        saveSequence(file.getFullPathName());
        return Result::ok();
    }    
    
    File getSuggestedSaveAsFile	(	const File & 	defaultFile	) override
    {
        File newFile;
        if (defaultFile.getFullPathName().endsWith(".ckf"))
            newFile = defaultFile;
        else
            newFile = defaultFile.withFileExtension(".ckf");
        return newFile;
    }
    
    /** This is used for dialog boxes to make them open at the last folder you
     were using.
     
     getLastDocumentOpened() and setLastDocumentOpened() are used to store
     the last document that was used - you might want to store this val ue
     in a static variable, or even in your application's properties. It should
     be a global setting rather than a property of this object.
     
     This method works very well in conjunction with a RecentlyOpenedFilesList
     object to manage your recent-files list.
     
     As a default value, it's ok to return File(), and the document object will
     use a sensible one instead.
     
     @see RecentlyOpenedFilesList
     */
    File getLastDocumentOpened()  override
    {
//        return lastDocOpened.withFileExtension(".ckf");  //We never save .mid files
        
        RecentlyOpenedFilesList recentFiles;
        recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                       ->getValue ("recentConcertKeyboardistFiles"));
        
        return recentFiles.getFile (0);
    }
    
    /** This is used for dialog boxes to make them open at the last folder you
     were using.
     
     getLastDocumentOpened() and setLastDocumentOpened() are used to store
     the last document that was used - you might want to store this value
     in a static variable, or even in your application's properties. It should
     be a global setting rather than a property of this object.
     
     This method works very well in conjunction with a RecentlyOpenedFilesList
     object to manage your recent-files list.
     
     @see RecentlyOpenedFilesList
     */
    void setLastDocumentOpened (const File& file)  override
    {
//        lastDocOpened = file;
        RecentlyOpenedFilesList recentFiles;
        recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                       ->getValue ("recentConcertKeyboardistFiles"));
        
        recentFiles.addFile (file);
//        std::cout << "setLastDocumentOpened " << recentFiles.getNumFiles() << " " <<  file.getFullPathName() << " " << recentFiles.toString() << "\n";

//        std::cout << "setLastDocumentOpened "<<recentFiles.toString()<<"\n";
        getAppProperties().getUserSettings()
        ->setValue ("recentConcertKeyboardistFiles", recentFiles.toString());
    }
    File lastDocOpened;
    PropertySet sequenceProps;
    
    void setNoteActive(int nt, int channel, bool setting)
    {
        //If there are no program changes to be sent then we assume there is just one output device, so even if we are
        //sending on multiple channels we treat is as one channel.  This is to prevent (if we are playing on a single device)
        //a note from being played on one channel, then immediately repeated on a different one, therefore not forcing off
        //the previous note, resulting in the previous note from being terminated abruptly.
        if (!areThereProgramChanges)
            channel = 0;
        const int ndx = 16*(channel-1)+nt;
//        String note = MidiMessage::getMidiNoteName (nt, true, true, 3);
        noteIsOn[ndx]=setting;
//        std::cout << "setNoteActive " << note<<" "<< channel<<" "<<noteActive[ndx]<<" Ndx "<<ndx<<"\n";
    }
    bool isNoteActive(int nt, int channel)
    {
        if (!areThereProgramChanges)
            channel = 0;
        const int ndx = 16*(channel-1)+nt;
//        String note = MidiMessage::getMidiNoteName (nt, true, true, 3);
//        std::cout << "isNoteActive " << note<<" "<< channel<<" "<<noteActive[ndx]<<" Ndx "<<ndx<<"\n";
        return noteIsOn[ndx];
    }

    MidiFile midiFile; //File holding score
    inline double getPPQ()
    {
        return 960.0;
    }
    Array<MidiMessage>  getTimeSigInfo()
    {
        return timeSigChanges;
    }
    std::vector<std::shared_ptr<NoteWithOffTime>> &getSequence()
    {
        return theSequence;
    }
    inline int getSize()
    {
        return (int) theSequence.size();
    }
	unsigned int encodeVarint(uint8* output, unsigned int value);
	unsigned int decodeVarint(uint8* input, unsigned int inputSize);
    int myConvertFromBase64 (OutputStream& binaryOutput, StringRef base64TextInput);
	unsigned int varintLength(char* input, unsigned int inputSize);
    int getSeqDurationInTicks() {
        return seqDurationInTicks;
    }
//    std::vector<NoteWithOffTime*> getNotesInTimeRange(double minTimeStamp, double maxTimeStamp);
    Array<MidiMessage> getCurrentTimeSig(double timeStamp);
    void getNotesUsed(int &minNote, int &maxNote);
    
    bool loadingFile;
    void setLoadingFile(bool loading)
    {
        loadingFile=loading;
    }
    bool getLoadingFile() {return loadingFile;}

    enum Retain {doRetainEdits, doNotRetainEdits};
    enum LoadType {loadFile, reAnalyzeOnly, updateChords};
    bool loadSequence (LoadType justRebuildSequence, Retain retainEdits);
    bool causeLoadFailure = false;
    Array<StepActivity> chain(Array<int> selection, double interval);
    double chainingInterval;
    
    void setScoreFile(File file)
    {
        scoreFile = file;
        propertiesChanged = true;
    }
    String getScoreFileName() {return scoreFile.getFileName();}
    
    inline bool isBlackNote(int note)
    {
        return (blackNote[note%12]);
    }
    
    void saveSequence(File fileToSave);
    void dumpData(Array<int>);
    void dumpData(int start, int end, int nn);
    
    void setChordTimeHumanize(String value, bool documentReallyChanged)
    {
        chordTimeHumanize = value;
        sequenceProps.setValue("chordTimeHumanize", chordTimeHumanize);//How much to randomize simultaneous notes
        setChangedFlag (documentReallyChanged);
    }
    
    void setChordVelocityHumanize(String value, bool documentReallyChanged)
    {
        chordVelocityHumanize = value;
//        sequenceProps.setValue("chordVelocityHumanize", chordVelocityHumanize);//How much to randomize note velocities
        setChangedFlag (documentReallyChanged);
    }
    
    void setExprVelToOriginalValRatio(double value, bool documentReallyChanged)
    {
        exprVelToScoreVelRatio = value;
        sequenceProps.setValue("exprVelToScoreVelRatio", exprVelToScoreVelRatio);
        setChangedFlag (documentReallyChanged);
    }
    inline double getExprVelToOriginalValRatio()
    {
        return exprVelToScoreVelRatio;
    }
    void setChordVoicing(Array<double> values, bool documentReallyChanged)
    {
        chordVoicing = values;
        sequenceProps.setValue("chordVoicing", chordVoicing[1]); 
        setChangedFlag (documentReallyChanged);
    }
    
    enum TempoControl {autoTempo, fixedTempo, proportionalToOriginalTempo};
    TempoControl tempoControl;
    void setTempoControl(TempoControl value)
    {
        tempoControl = value;
        sequenceProps.setValue("tempoControl", "auto"); //enum: autoTempo, fixedTempo, tempoFromFile
    }
    std::vector<MidiMessage> tempoChanges;
    std::vector<MidiMessage> scaledTempoChanges;
    //Actual tempo is the midi file multiplied by tempoMultiplier
    double tempoMultiplier;
    void setTempoMultiplier(double value, double currentTime, bool documentReallyChanged);
    void setTempo(double tem)
    {
        tempo = tem;
    }
    
    void increaseTempo(double factor)
    {
        tempoMultiplier *= factor;
    }
    void decreaseTempo(double factor)
    {
        tempoMultiplier *= factor;
    }
    double getTempo (double currentTime, std::vector<MidiMessage> &tempos);
    double getTempoMultipier (double currentTime);
    
    //Realtime playing parameters
    double latePlayAdjustmentWindow;
    void setLatePlayAdjustmentWindow(double value)
    {
        latePlayAdjustmentWindow = value;
        sequenceProps.setValue("latePlayAdjustmentWindow", value);
    }
    double leadLagAdjustmentFactor;
    void setLeadLagAdjustmentFactor(double value)
    {
        leadLagAdjustmentFactor = value;
        sequenceProps.setValue("leadLagAdjustmentFactor", value);
    }
    double kV;
    void setKV(double value)
    {
        kV = value;
        sequenceProps.setValue("kV", value); //float - Tempo adjustment due to deltaNoteOnLag
    }
    double kX;
    void setKX(double value)
    {
        kX = value;
        sequenceProps.setValue("kX", value); //float - Tempo adjustment due to noteOnLag
    }
    
    double upperTempoLimit; //As ratio of score tempo
    void setUpperTempoLimit(double value)
    {
        upperTempoLimit = value;
        sequenceProps.setValue("upperTempoLimit", value); //float
    }

    double lowerTempoLimit; //As ratio of score tempo //float
    void setLowerTempoLimit(double value)
    {
        lowerTempoLimit = value;
        sequenceProps.setValue("lowerTempoLimit", value); //float
    }
    
    int notePlayWindow;
    void setNotePlayWindow(int value)
    {
        notePlayWindow = value;
        sequenceProps.setValue("notePlayWindow", value); //float How far early a note can be played in ticks
    }
    int notePlayWindowAutoplaying;
    void setNotePlayWindowAutoplaying(int value)
    {
        notePlayWindowAutoplaying = value;
        sequenceProps.setValue("notePlayWindowAutoplaying", value); //float How far early a note can be played in ticks while autoplaying
    }
    
    String soundfontFile;
    void setSoundfontFile(String value)
    {
        soundfontFile = value;
        sequenceProps.setValue("soundfontFile", value);
    }

    String pluginFile;
    void setPluginFile(String value)
    {
        pluginFile = value;
        sequenceProps.setValue("pluginFile", value);
    }
    
//    bool inline isPrimaryTrack (int trk)
//    {
////        std::cout << "trk, primaryTracks, isPrimary " << trk<<" "<<primaryTracks << " "<<(primaryTracks & (1 << trk)) << "\n";
////        return primaryTracks & (1 << trk);
//        return (trackDetails[trk].playability == Track_Play);
//    }
    bool inline isActiveTrack (int trk)
    {
        return (trackDetails[trk].playability == Track_Play);
    }
    
    std::vector<double> beatTimes;
    std::vector<double> measureTimes;
    
    class Bookmark
    {
    public:
        Bookmark()
        {
            time = -1;
        }
        double time;
        bool tempoChange;
        double tempoScaleFactor;
        
        bool operator< (const Bookmark& bm2) const
        {
            return time<bm2.time;
        }
    };
    Array<Bookmark> bookmarkTimes;
    std::vector<int> programChanges;
    bool areThereProgramChanges;

//    std::vector<std::vector<NoteWithOffTime>> allNotes; //Indexed by track.  Tracks sorted by timeStamp, then descending noteNum
    std::vector<std::vector<std::shared_ptr<NoteWithOffTime>>> allNotes; //Indexed by track.  Tracks sorted by timeStamp, then descending noteNum
    
    class PedalMessage
    {
    public:
        PedalMessage (double time, bool on)
        {
//            std::cout<< "Make pedal message "<<time <<" "<< on <<"\n";
            timeStamp = time;
            pedalOn = on;
        }
        double timeStamp;
        bool pedalOn;
    };
    std::vector<ControllerMessage> theControllers;
    std::vector<PedalMessage> sustainPedalChanges;
    std::vector<PedalMessage> softPedalChanges;
    File scoreFile;  //This is just used by the file name display in the UI because it must be changed on save
    int numerator, denominator;
//    double timeIncrement; //Amount to increase time at each tick, based on ppq of 960.  Adjusted based on actual ppq of this midi file.
    int rightHandStartsHere = 1;
    
    String chordTimeHumanize;
    String chordVelocityHumanize;
    double exprVelToScoreVelRatio;
    Array<double> chordVoicing;
    
    double triggeredNoteLimit; //Maximum duration from a chainTrigger's start that a triggeredNote can start
    
    double seqDurationInTicks;
    int numTracks;
    Array<MidiMessage> timeSigChanges;
    
    //ChordVelTypes: Custom - "cust"; From Preset - "pres"; From Algorithm - "alg"
    //ChordTimeTypes Custom - "cust"; From Algorithm - "alg"
    
    class ChordDetail {
        friend Sequence;
    public:
        ChordDetail( )
        {
            chordTimeStamp=-1;
            scaleFactor=1.0f;
            timeSpec = "Manual";
            timeRandScale = 1.0f;
            timeRandSeed = 1;
            velSpec = "Manual"; //"None","Random',"Manual"
            velRandScale = 1.0f;
            notePointers.clear();
            noteIds.clear();
            offsets.clear();
            velRandSeed = 1;
            chordRect = Rectangle<float>();
            chordSelected = false;
        }
        ChordDetail(ChordDetail const &ch) :
        chordTimeStamp(ch.chordTimeStamp),
        scaleFactor(ch.scaleFactor),
        timeSpec(ch.timeSpec),
        timeRandScale(ch.timeRandScale),
        timeRandSeed(ch.timeRandSeed),
        velSpec(ch.velSpec),
        velRandScale(ch.velRandScale),
        velRandSeed(ch.velRandSeed),
        notePointers(ch.notePointers),
        offsets(ch.offsets),
        noteIds(ch.noteIds),
        chordRect(ch.chordRect),
        chordSelected(ch.chordSelected)
        {
        }
    public:
        int chordTimeStamp; //20
        float scaleFactor; //10
        String timeSpec; //20
        float  timeRandScale; //10
        int timeRandSeed; //10
        String velSpec; //20
        float  velRandScale; //10
        int velRandSeed; //10
        std::vector<std::shared_ptr<NoteWithOffTime>> notePointers; //Pointers to chord's notes
        std::vector<int> offsets; //Offsets from timeStamp of chord top note (or from chord time stamp????)
        std::vector<String> noteIds; //String(track)+"_"+String(channel)+"_"+String(noteNumber)
        Rectangle<float> chordRect; //Rectangle surrounding chord for display and hit testing. Value defined in makeNoteBars.
        bool chordSelected;
    }; //110
    
    Array<double> targetNoteTimes;
    std::vector<ChordDetail> chords;
    
    //Given a vector of pointers to contiguous notes sorted from highest to lowest guaranteed not to overlap any other chord,
    // create a new chord in chords[ ] and update the notes to know about the their chord.
    int newChordFromSteps(std::vector<std::shared_ptr<NoteWithOffTime>> chordNotes)
    {
        //Create a new ChordDetail record, chDet
        //Fill in its notePointers and noteIds (chordRect is defined in makeNoteBars)
        //Determine location of new chord in chords[] based on time stamp
        //Insert chDet into the chords array at the correct location
        //Determine the Return the chordIndex of the new chord in chords[ ]
        int firstStep = INT_MAX;
        int lastStep = INT_MIN;
        int highestNoteNumber = -1;
        int highestNoteIndex = -1;
        for (int i=0; i<chordNotes.size();i++)
        {
            if (chordNotes.at(i)->currentStep < firstStep)
                firstStep = chordNotes.at(i)->currentStep;
            if (chordNotes.at(i)->currentStep > lastStep)
                lastStep = chordNotes.at(i)->currentStep;
            if (chordNotes.at(i)->noteNumber > highestNoteNumber)
            {
                highestNoteIndex = i;
                highestNoteNumber = chordNotes.at(i)->noteNumber;
            }
        }
        
        ChordDetail chDet;
        StringArray values;
        chDet.chordTimeStamp = chordNotes.at(highestNoteIndex)->getTimeStamp();
        chDet.notePointers = chordNotes;
        for (int i=0;i<chordNotes.size();i++)
        {
            String noteId = String(chordNotes.at(i)->track)+"_"+String(chordNotes.at(i)->channel)+"_"+String(chordNotes.at(i)->noteNumber);
            
//            std::cout << i<< " chord notePointer: step " << chDet.notePointers.at(i)->currentStep
//            <<" ts " << chDet.notePointers.at(i)->timeStamp << "\n";
            
            const int offset = chordNotes.at(i)->timeStamp - chordNotes.front()->timeStamp;
            chDet.offsets.push_back(offset);
            chDet.noteIds.push_back(noteId);
        }
        //Find where to insert chord into chords array and insert it
        int chordIndex;
        for (chordIndex=0;chordIndex<chords.size();chordIndex++)
        {
            if (chords.at(chordIndex).chordTimeStamp > chordNotes.at(0)->timeStamp)
                break;
        }
        if (chords.size()==0)
            chords.push_back(chDet);
        else
            chords.insert(chords.begin()+chordIndex,chDet);
        
        //Also update each chord note's information about its membership in a chord:
        chordNotes.at(0)->chordTopStep=-1;
        chordNotes.at(0)->noteIndexInChord=-1;
//        chordIndex -= 1;
        chordNotes.at(0)->chordIndex = chordIndex;
        chordNotes.at(0)->inChord = true;
        for (int i=1; i<chordNotes.size(); i++)
        {
            chordNotes.at(i)->chordTopStep = chordNotes.at(0)->currentStep;
            chordNotes.at(i)->chordIndex = chordIndex;
            chordNotes.at(i)->noteIndexInChord = i;
            chordNotes.at(i)->inChord = true;
        }
        
        //Adjust the chord index of each note after the notes of the inserted chord
        for (int step=lastStep+1; step<theSequence.size();step++)
        {
            if (theSequence.at(step)->inChord)
                theSequence.at(step)->chordIndex += 1;
        }
        return chordIndex;
    }
    
    Array<Array<double>> undoStack;
    std::vector<bool> noteIsOn;
    
//    bool suppressSpeedAdjustment;
    double tempo;
    bool waitForFirstNote; //If true when play is started, Processor::waitingForFirstNote is set true & nxt unplayed note is moved to ztl.
    bool autoPlaySustains;
    bool autoPlaySofts;
    bool allVelocitiesFromScore;
    bool deriveSecVelocityFromPrimary;
    int originalPpq;
    int ppq;
    
//    String headerStringAsB64;
    char sysexTrackMarker[3] = {'p','p','f'};
//    typedef struct SysexHeader {
//        char tag[6];
//    } SysexHeaderType;
    
    enum TrackTypes {Track_Play = 1, Track_Mute, Track_Autoplay, Track_Controllers, Track_Other};
        
    const bool blackNote[11] = {false,true,false,true,false,false,true,false,true,false,true};
    typedef struct TrackDetail {
        String description;
        String instrument;
        int nNotes;
        int nSustains;
        int nSofts;
        int startMeasure;
        int endMeasure;
        int playability;
        int originalChannel;
        int assignedChannel;
    } trackDetail;

    int nPlayableTracks()
    {
        auto nActive = 0;
        for (auto i = 0; i < trackDetails.size(); ++i) {
            if (trackDetails[i].playability == Track_Play)
                nActive++;
        }
        return nActive;
    }
    
    Array<TrackDetail> trackDetails;
    
    AudioPluginInstance *pThePlugin;
//    MemoryBlock pluginState; //Used to hold latest loaded plugin's state so it can be accessed by ContentComponent.
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sequence)
};
#endif  // SEQUENCE_H_INCLUDED
