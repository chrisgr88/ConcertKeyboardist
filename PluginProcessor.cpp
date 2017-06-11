/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
ReExpressorAudioProcessor::ReExpressorAudioProcessor()
{
//    createEditorIfNeeded();
//    addChangeListener((ChangeListener *)getActiveEditor());
}

ReExpressorAudioProcessor::~ReExpressorAudioProcessor()
{
}

//==============================================================================
const String ReExpressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReExpressorAudioProcessor::acceptsMidi() const
{
//   #if JucePlugin_WantsMidiInput
    return true;
//   #else
//    return false;
//   #endif
}

bool ReExpressorAudioProcessor::producesMidi() const
{
//   #if JucePlugin_ProducesMidiOutput
    return true;
//   #else
//    return false;
//   #endif
}

double ReExpressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReExpressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ReExpressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReExpressorAudioProcessor::setCurrentProgram (int index)
{
}

const String ReExpressorAudioProcessor::getProgramName (int index)
{
    return String();
}

void ReExpressorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ReExpressorAudioProcessor::prepareToPlay (double _sampleRate, int samplesPerBlock)
{
    std::cout << "Starting\n";
    blockSize = samplesPerBlock; 
    timeInSamples = 0;
    sampleRate = static_cast<float> (_sampleRate);
    onNotes.clear();
    sequenceReadHead = 0;
    lastPlayedSeqStep = -1;
    loadSequence("/Users/chrisgr/repos/reexpressor/Source/ThreeNotes34Time.mid");
    setSequenceChanged(true);
}

//void ReExpressorAudioProcessor::fileOpen(String fileName) {
//    loadSequence(fileName);
//    std::cout << "fileOpen\n";
//}

void ReExpressorAudioProcessor::loadSequence(String fileName)
{
    File file(fileName);
    lastPlayedSeqStep = -1;
    timeInTicks = 0;
    sequenceReadHead = 0;
    //File loading steps:
    
    //Case1:
    //User chooses file in editor (& sequence if more than one sequence?)
    //Editor passes path (& sequence?) to processor
    //User chooses
    //Case2:
    //On startup, processor fetches path of previous file from settings
    
    //Processor loads & parses file
    //Processor sends change notification to editor
    
    //Also, each time editor opens:
    
    //Editor fetches info on note range, etc & updates keyboard in ui & ui dimensioning details
    //Editor is calling repaint in a timer which is fetching current time and updating display
    //On zoom or window resize editor handles updating display by itself
    //getActiveEditor(); //This is how the processor can get the active editor if needed
    
    //Test reading midi file
    //    File file ("/Users/chrisgr/JuceProject/ReExpressor/Source/Bach.mid");
    //    File file ("/Users/chrisgr/JuceProject/ReExpressor/Source/OneNote-34Time.mid");
    //    File file ("/Users/chrisgr/JuceProject/ReExpressor/Source/IsntSheLovely.mid");
    //    File file ("/Users/chrisgr/Documents/midi/RavelPavanFromMusScore.mid");
    //    File file ("/Users/chrisgr/Documents/midi/ravel_little_pieces_2_(c)oguri.mid");
    
    //    File file ("/Users/chrisgr/Documents/midi/bachFromMusScore.mid");
    //    File file ("/Users/chrisgr/Downloads/chop23b.mid");
    //    File file ("/Users/chrisgr/Downloads/choval16.mid");
    
//    File file ("/Users/chrisgr/Downloads/mz_545_2.mid");
//        File file ("/Users/chrisgr/Downloads/grieg_walzer.mid");
    //    File file ("/Users/chrisgr/Downloads/glullaby.mid");
    
    //    File file ("/Users/chrisgr/Downloads/eyeofthetiger.mid");
    //    File file ("/Users/chrisgr/Downloads/77sunsetstrip.mid");
    //    File file ("/Applications/Kyma/MIDI/DESAFIN1.MID");
    
    if (!file.exists()) {
        std::cout << "File " << file.getFileName() << " does not exist.\n";
        return;
    }
    short fileType;
    MidiFile midiFile;
    FileInputStream inputStream(file);
    if(!midiFile.readFrom(inputStream))
        jassert(false);
    std::cout << "numTracks " << midiFile.getNumTracks() << "\n";
    std::cout << "timeFormat " << midiFile.getTimeFormat() << "\n";

    fileType = 0;//midiFile.getFileType();
    std::cout << "fileType " << fileType << "\n";
    std::cout << "lastTimeStamp " << midiFile.getLastTimestamp() << "\n";
  
    ppq = midiFile.getTimeFormat();
    timeIncrement = ppq/96.0f;
    MidiMessageSequence tempoChangeEvents;
    midiFile.findAllTempoEvents(tempoChangeEvents);
    tempoChanges.clear();
//    for (int i=0;i<tempoChangeEvents.getNumEvents();i++)
//    {
//        MidiMessage msg = tempoChangeEvents.getEventPointer(i)->message;
//    }
    
    if (tempoChangeEvents.getNumEvents()>0)
    {
        MidiMessage msg = tempoChangeEvents.getEventPointer(0)->message;
        double secPerQtr = msg.getTempoSecondsPerQuarterNote();
        tempo = (1.0/secPerQtr)*(60.0);
    }
    else
    {
        tempo = 60.0;
    }
    clockIntervalInMS = 1000.0*(60.0/tempo)/96.0; //Milliseconds per tick at a ppq of 96
    setSequenceChanged(true);
    
    
    MidiMessageSequence timesigChangeEvents;
    midiFile.findAllTimeSigEvents(timesigChangeEvents);
    timeSigChanges.clear();
    for (int i=0;i<timesigChangeEvents.getNumEvents();i++)
    {
        MidiMessage msg = timesigChangeEvents.getEventPointer(i)->message;
        timeSigChanges.add(msg);
        
        int numerator, denominator;
        msg.getTimeSignatureInfo(numerator, denominator);
        std::cout << "Time sig info " << msg.getTimeStamp() << " " << numerator << " " << denominator << "\n";
    }
    int numTracks = midiFile.getNumTracks();
    std::cout << "nTracks " << numTracks << "\n";
    
    int firstTrack;
    if (fileType==0 || numTracks==0)
    {
        firstTrack = 0;
    }
    else
    {
        firstTrack = 1;
    }
    
    theSequence.clear();
    for (int trkNumber=firstTrack;trkNumber<numTracks;trkNumber++)
    {
        if (trkNumber==10)
            continue;
        const MidiMessageSequence *theTrack;
        theTrack = midiFile.getTrack(trkNumber);
        for (int i=0;i<theTrack->getNumEvents();i++)
        {
            if (theTrack->getEventPointer(i)->message.isNoteOn())
            {
                NoteWithOffTime msg(trkNumber, theTrack->getEventPointer(i)->message,
                                    theTrack->getTimeOfMatchingKeyUp(i)); //of note bar
                theSequence.push_back(msg);
            }
            else if (theTrack->getEventPointer(i)->message.isTrackNameEvent())
            {
                String str = theTrack->getEventPointer(i)->message.getTextFromTextMetaEvent();
                std::cout << "TrackName: trkNumber, events, pos, str          " << trkNumber<<" "<<theTrack->getNumEvents()<<" "<<i<<" "<<str<<"\n";
            }
            else if (theTrack->getEventPointer(i)->message.isProgramChange())
            {
                int progCh = theTrack->getEventPointer(i)->message.getProgramChangeNumber();
//                if (progCh==10)
//                    continue;
                std::cout << "ProgChange: trkNumber, events, pos, prog#       " << trkNumber<<" "<<theTrack->getNumEvents()<<" "<<i<<" "<<progCh<<"\n";
            }
            else if (theTrack->getEventPointer(i)->message.isTempoMetaEvent())
            {
//                double len = theTrack->getEventPointer(i)->message.getTempoSecondsPerQuarterNote();
//                std::cout << "TempoChange: trkNumber, events, pos, secsPerQtr " << trkNumber<<" "<<theTrack->getNumEvents()<<" "<<i<<" "<<len<<"\n";
            }
        }
    }
    
    std::sort(theSequence.begin(), theSequence.end());
    
    //Quantize start times
    if (theSequence.size()>0)
    {
        int simultaneousDelta = ppq/8-1;
        int prevRetainedTimeStamp = theSequence[0].getTimeStamp();
        for (int note=0; note<theSequence.size();note++) {
            if ((theSequence[note].getTimeStamp()-prevRetainedTimeStamp)<=simultaneousDelta)
            {
                theSequence[note].setTimeStamp(prevRetainedTimeStamp);
            }
            else
                prevRetainedTimeStamp = theSequence[note].getTimeStamp();
        }
    }
    
    //    Example
    //    SampleRate = 44100
    
    //    sampleInterval = 1000/sampleRate in ms (e.g. 1000/44100 = 0.0226ms
    //    Tempo = 120 bpm = 2 beats per second = 500 ms per quarter note.  (120/bpm)*500 = msPerQuarterNote
    //    sampleBufferDuration = bufferSize/(sampleRate/1000) e.g. 512/44.1 ms = 11.61 ms
    //    tickInterval = msPerQuarterNote/ppq e.g. 500/96 = 5.20833333 ms per tick |||| samplesPerTick = 5.208/0.0226
    //    samplesPerTick = ((120/bpm)*500/ppq)/(1000/sampleRate) = ((120/120)*500/96)/(1000/44100)
    //    =(120*500*sampleRate)/(bpm*ppq*1000) = (60*sampleRate)/(bpm*ppq) = (60*44100)/(120*96)
    //    per sample buffer with 512 frames ca 2 ticks ( 2.22911565 )
    
    //    Need conversion of cumulative timetag (samples) to ticks
    
    //    Cumulative ms = 44.100 samples/ms ////  msPerSample = 1/44.1
    //    Cumulative time = (1/44.1)*cumulativeTimeTag
}

void ReExpressorAudioProcessor::rewind()
{
    lastPlayedSeqStep = -1;
    timeInTicks = 0;
    sequenceReadHead = 0;
}

void ReExpressorAudioProcessor::updatePlayHeadFromHost()
{
    if (AudioPlayHead* ph = getPlayHead())
    {
        AudioPlayHead::CurrentPositionInfo newPlayHead;
        
        if (ph->getCurrentPosition (newPlayHead))
        {
            playHeadInfo = newPlayHead;  //Successfully got the current playHead from the host..
            return;
        }
    }
    
    // If the host fails to provide the current time, we'll just reset our copy to a default..
    playHeadInfo.resetToDefault();
}

double ReExpressorAudioProcessor::getTimeInTicks()
{
    return timeInTicks;
}

double ReExpressorAudioProcessor::getPPQ()
{
    return ppq;
}
Array<MidiMessage> ReExpressorAudioProcessor::getTimeSigInfo()
{
    if (timeSigChanges.size()>0)
    {
        return   timeSigChanges;
        int num, denom;
        timeSigChanges[0].getTimeSignatureInfo(num, denom);
        return timeSigChanges;
    }
    else
    {
        MidiMessage msg = MidiMessage::timeSignatureMetaEvent(4,4);
        Array<MidiMessage> timeSigChanges;
        timeSigChanges.add(msg);
        return timeSigChanges;
    }
}

void ReExpressorAudioProcessor::printMessage(MidiMessage const& message)
{
    const int size = message.getRawDataSize();
    const uint8* data = message.getRawData();
    String logMessage = "message[time: ";
    logMessage << message.getTimeStamp();
    logMessage << "] ";
    for(int i = 0; i < size; i++)
    {
        logMessage << data[i] << " ";
    }
    Logger::outputDebugString(logMessage);
}

void ReExpressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReExpressorAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    // Reject any bus arrangements that are not compatible with your plugin

    const int numChannels = preferredSet.size();

   #if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
   #elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
   #else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
#endif
   
    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

void ReExpressorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    updatePlayHeadFromHost();
    isPlaying = playHeadInfo.isPlaying;
//    const double samplesPerTick = (60*sampleRate)/(playHeadInfo.bpm*ppq);
    
    if (isPlaying)
    {
        if (!isTimerRunning())
            startTimer(clockIntervalInMS);
    }
    else
    {
        if (isTimerRunning())
            stopTimer();
    }
    
    const int64 tis = playHeadInfo.timeInSamples;
    if (tis==0)
    {
        sequenceReadHead = 0;
        lastPlayedSeqStep = -1;
        stopTimer();
        timeInTicks = 0;
    }
//    timeInTicks = tis/samplesPerTick;
    //    Array<int> notes = availableNotes(tis/samplesPerTick);
    
//    if ((int)playHeadInfo.timeInSamples %(blockSize*10) == 0) {
//        std::cout << "samplesPerTick " << samplesPerTick << "\n";
//        std::cout << "timeInSamples " << (playHeadInfo.timeInSamples/sampleRate) <<"\n";
//        std::cout << "timeInTicks " << timeInTicks <<"\n";
//        std::cout << "timeInQuarterNotes " << ((timeInSamples/samplesPerTick)/96.0) <<"\n";
//        for (int i=0;i<notes.size();i++)
//            std::cout << notes[i] << " ";
//        std::cout << "\n";
//    }
    
    MidiMessage msg;
    int samplePosition;
    Array<MidiMessage> exprEvents;
    exprEvents.clear();
    for (MidiBuffer::Iterator it (midiMessages); it.getNextEvent (msg, samplePosition);)
    {
        if (minSeqNote <= msg.getNoteNumber() && msg.getNoteNumber() <= maxSeqNote)
        {
            ;
        }
        else //It was an expression note
        {
            exprEvents.add(msg);
        }
    }
    
    midiMessages.clear();

//    //Delete sequence events that are too old
//    for (int seqNotesIndex=0;seqNotesIndex<seqNotes.size();seqNotesIndex++) {
//        if (seqNotes[seqNotesIndex].getTimeStamp() < (timeInSamples - timeLag))
//            seqNotes.remove(seqNotesIndex);
//        else
//            break;
//    }

    Array<MidiMessage> availableNotes; //Only note-ons are stored here
    availableNotes.clear();
    //Do note-ons
    for (int exprEventIndex=0;exprEventIndex<exprEvents.size();exprEventIndex++)
    {
        if (exprEvents[exprEventIndex].isNoteOn())
        {
            availableNotes = getAvailableNotes(timeInTicks);
            lastPlayedSeqStep += availableNotes.size();
            if(availableNotes.size()>0) //Do note-ons of the first sequence note and all with the same time stamp
            {
                Array<MidiMessage> notesToStart;
                for (int seqNoteIndex=0;seqNoteIndex<availableNotes.size();seqNoteIndex++)
                {
                    notesToStart.add(availableNotes[seqNoteIndex]);
                }
                
                for (int noteToStart = 0;noteToStart<notesToStart.size();noteToStart++)
                {
                    //Do note-ons that came before note-offs of the same note on the same channel
                    for(int onNoteIndex=onNotes.size()-1;onNoteIndex>=0;onNoteIndex--)
                    {
                        if (onNotes[onNoteIndex].getChannel() == notesToStart[noteToStart].getChannel() &&
                            onNotes[onNoteIndex].getNoteNumber() == notesToStart[noteToStart].getNoteNumber())
                        {
//                            std::cout << "Force note off " << onNotes[onNoteIndex].getNoteNumber() <<"\n";
                            MidiMessage noteOn = MidiMessage::noteOn(onNotes[onNoteIndex].getChannel(),
                                                                       onNotes[onNoteIndex].getNoteNumber(),
                                                                       (uint8) 0);
                            midiMessages.addEvent(noteOn,samplePosition);
                            onNotes.remove(onNoteIndex);
                        }
                    }
                    MidiMessage noteOn = MidiMessage::noteOn(  notesToStart[noteToStart].getChannel(),
                                                               notesToStart[noteToStart].getNoteNumber(),
                                                               exprEvents[exprEventIndex].getVelocity());
                    noteOn.setTimeStamp(samplePosition);
//                    std::cout << "NoteOn " << noteOn.getNoteNumber() << "\n";
                    midiMessages.addEvent(noteOn,samplePosition);
                    //We store the note-number of the triggering expr note-on so we can turn off this sequence note when
                    //the this particular expression note's note-off happens.
                    noteOn.setTimeStamp(exprEvents[exprEventIndex].getNoteNumber());
                    onNotes.add(noteOn);
                }
//                std::cout << "\n";
            }
        }
        else if (exprEvents[exprEventIndex].isNoteOff()) //Do note-offs for all sequence notes that are active
        {
            for (int onNoteIndex=onNotes.size()-1;onNoteIndex>=0;onNoteIndex--)
            {
                //The onNotes time stamp actually contains the note number of the expr event that started this onNote
                //..so turn off all onNotes that this expr note-off is related to.
                if (onNotes[onNoteIndex].getTimeStamp() == exprEvents[exprEventIndex].getNoteNumber())
                {
//                    notesPendingOff.push_back(<#const_reference __x#>)
                    MidiMessage noteOff = MidiMessage::noteOn(onNotes[onNoteIndex].getChannel(),
                                                               onNotes[onNoteIndex].getNoteNumber(),
                                                               (uint8) 0);
                    noteOff.setTimeStamp(samplePosition);
                    onNotes[onNoteIndex].setVelocity(0);
//                    std::cout << "NoteOff" << noteOff.getNoteNumber() << " ";
                    midiMessages.addEvent(noteOff,samplePosition);
                    onNotes.remove(onNoteIndex);
                }
            }
//            std::cout << "\n";
        }
        else //Not a note note-on or or note-off so just transfer to out buffer
            midiMessages.addEvent(exprEvents[exprEventIndex], exprEvents[exprEventIndex].getTimeStamp());
    }
    exprEvents.clear();
    //Remove sequence notes that are on
    for (int seqNoteIndex=availableNotes.size()-1;seqNoteIndex>=0;seqNoteIndex--)
    {
        for (int onNoteIndex=0;onNoteIndex<onNotes.size();onNoteIndex++)
        {
            if (availableNotes[seqNoteIndex].getChannel() == onNotes[onNoteIndex].getChannel() &&
                availableNotes[seqNoteIndex].getNoteNumber() == onNotes[onNoteIndex].getNoteNumber())
                availableNotes.remove(seqNoteIndex);
        }
    }
    
    //------For testing   Create a dummy note-on, as if from sequence.
//    seqNotes.clear();
//    if (noteCounter++>110)
//        noteCounter=40;
//    MidiMessage foo1 = MidiMessage::noteOn(2, noteCounter, MidiMessage::floatValueToMidiByte (127.0));
//    foo1.setTimeStamp(time-100);
//    if (time>100) {
//        seqNotes.add(foo1);
//        //Simulate another note with different note number and/or time stamp
//        MidiMessage foo2 = MidiMessage::noteOn(2, noteCounter+4, MidiMessage::floatValueToMidiByte (127.0));
//        foo2.setTimeStamp(time - 100); //Enable this line out to create the test case for equal time stamps
////        foo2.setTimeStamp(time - 90); //Enable this line out to create the test case for different time stamps
//        seqNotes.add(foo2); //Comment this line out for the test case with a single note in the buffer
//    }
    //------For testing
    timeInSamples += blockSize;
}



//==============================================================================
// Return the note numbers of the next notes to play from "theSeq"
// Notes to play are the next notes with time <= (hostTimeInTicks-noteAvailablityWindow) ....
//...and note time >= sequenceReadHead
//Return all notes from first found up to the note with timeTag <= (sequenceReadHead+quantizationInterval)

Array<MidiMessage> ReExpressorAudioProcessor::getAvailableNotes(int hostTimeInTicks)
{
    Array<MidiMessage> notes;
    notes.clear();
    int noteIndex;
    for (noteIndex=0;noteIndex<theSequence.size();noteIndex++)
    {
        if (theSequence.at(noteIndex).isNoteOn() &&
            theSequence.at(noteIndex).getOnTime()>=sequenceReadHead)
        {
            notes.add(theSequence.at(noteIndex));
//            std::cout << "hostTimeInTicks,noteIndex " << hostTimeInTicks <<" "<<noteIndex<<"\n";
            break;
        }
    }
    if (notes.size()>0)  //Add all other notes with the same time stamp
        while (++noteIndex < theSequence.size() && theSequence.at(noteIndex).getTimeStamp() == notes.getFirst().getTimeStamp())
            notes.add(theSequence.at(noteIndex));
    
//    std::cout << "notes ";
    for (int i=0;i<notes.size();i++)
    {
        MidiMessage msg = notes[i];
//        std::cout <<  msg.getNoteNumber() << " ";
    }
//    std::cout << "\n";
    
    if (theSequence.size()==noteIndex) //Last note - reset to start
    {
        sequenceReadHead+=999999;
//        timeInTicks = 0;
//        sequenceReadHead = 0;
//        lastPlayedSeqStep = -1;
    }
    else
    if (theSequence.size()>noteIndex)
    {
        if (singleStep)
            timeInTicks = theSequence.at(noteIndex).getTimeStamp(); //Advance time to start of next note
        sequenceReadHead = notes.getLast().getTimeStamp()+1;
    }
    
//    int ts = theSequence[noteIndex].getTimeStamp();
//    if (notes.size()>0)
//    {
//            std::cout << " play note "<< notes.getLast()<<" " << theSequence[noteIndex].getTimeStamp()<<"\n";
////            const int lastSimultaneousTime = noteTime;
////            while (noteIndex<theSequence.size() && theSequence[noteIndex].getTimeStamp() <= lastSimultaneousTime)
////            {
////                std::cout << " play note "<< notes.getLast()<<" " << theSequence[noteIndex].getTimeStamp()<<"\n";
////                noteIndex++;
////            }
//            sequenceReadHead = theSequence[noteIndex].getTimeStamp()+1;
//    }
    
    return notes;
}

std::vector<NoteWithOffTime> *ReExpressorAudioProcessor::getSequence()
{
    return &theSequence;
}

std::vector<NoteWithOffTime> ReExpressorAudioProcessor::getNotesInTimeRange(double minTimeStamp,
                                                                            double maxTimeStamp)
{
    std::vector<NoteWithOffTime> notes;
    notes.clear();
    for (int noteIndex=0;noteIndex<theSequence.size();noteIndex++)
    {
        NoteWithOffTime msg = theSequence.at(noteIndex);
        if (msg.getOffTime()>=minTimeStamp && msg.getOnTime()<=maxTimeStamp)
            notes.push_back(msg);
    }
    return notes;
}
SortedSet<int> ReExpressorAudioProcessor::getNotesUsed(int &minNote, int &maxNote)
{
    SortedSet<int> used;
    minNote = 127;
    maxNote = 0;
    for (int noteIndex=0;noteIndex<theSequence.size();noteIndex++)
    {
        const int note = theSequence.at(noteIndex).getNoteNumber();
        used.add(note);
        if (note<minNote)
            minNote = note;
        if (note>maxNote)
            maxNote = note;
    }
    return used;
}

Array<MidiMessage> ReExpressorAudioProcessor::getCurrentTimeSig(double timeStamp)
{
    Array<MidiMessage> foo;
    foo.clear();
    return foo;
}

void ReExpressorAudioProcessor::hiResTimerCallback()
{
    timeInTicks+=timeIncrement;
}

//==============================================================================
bool ReExpressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ReExpressorAudioProcessor::createEditor()
{
    return new ReExpressorAudioProcessorEditor (this);
}

//==============================================================================
void ReExpressorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ReExpressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReExpressorAudioProcessor();
}
