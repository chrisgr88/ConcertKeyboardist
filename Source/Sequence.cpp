/*
  ==============================================================================

    Sequence.cpp
    Created: 31 Jan 2017 5:56:13am
    Author:  ChrisGr

  ==============================================================================
*/

#include "Sequence.h"
Sequence::Sequence()
: FileBasedDocument (filenameSuffix,
                     filenameWildcard,
                     "Load a Concert Keyboardist or MIDI File",
                     "Save a Concert Keyboardist File")
{
    triggeredNoteLimit = 4;
    tempoControl = TempoControl::autoTempo;
    waitForFirstNote = true;
    autoPlaySustains = true;
    autoPlaySofts = true;
    reVoiceChords = true;
    setNotePlayWindowAutoplaying(30);
    setNotePlayWindow(200);
    setLatePlayAdjustmentWindow(100);
    setLeadLagAdjustmentFactor(1.0);
    setKX(0.0000001);
    setKV(0.00010);
    setLowerTempoLimit(0.6);
    setUpperTempoLimit(1.4);
    majorVersionSavingFile = 0;
    minorVersionSavingFile = 2;
    buildNumberSavingFile = 9999;
    setSoundfontFile("//root/soundfront.sfz");
    setPluginFile("Massive");
}
Sequence::~Sequence()
{
//    undoMgr->clearUndoHistory();
//    delete undoMgr;
}

/*=============================================================
 <#saveSequence#>
 */
//TODO: Add calls to the new setter functions to set these properties
void Sequence::saveSequence(File fileToSave)// String  name = "")
{
    //TODO Save these:
    //- leadTimeProportionOfWidth
    setScoreFile(fileToSave); //Used for file name display it must be changed on save (updated in by ViewerFrame change notifier)
//Score properties - saved in each score file - Those with defaults can be omitted
    //Other properties:
    // - "timeInTicks" updated by MidiProcessor::prepareToSave()
    // - "majorVersion" updated by MidiProcessor::prepareToSave()
    // - "minorVersion" updated by MidiProcessor::prepareToSave()
    // - "buildNumber" updated by MidiProcessor::prepareToSave()
    // - "tn" - Forced target note - see below
    // - "ntn" - Forced non target note - see below
    // - "chainSeg" see below
    // - "tempoMultiplier" kept updated by setTempoMultiplier()
    // - "waitForFirstNote"
    // - "horizontalScale" is kept updated by the ScrollingNoteViewer

    //Not currently implemented:
    //    props.setValue("notePlayWindow", 100); //float How far early a note can be played in ticks
    //    props.setValue("notePlayWindowAutoplaying", 10); //float How far early a note can be played in ticks while autoplaying
    //    props.setValue("latePlayAdjustmentWindow", 100);
    //    props.setValue("leadLagAdjustmentFactor", 0.67);
    //    props.setValue("soundfontFile", "//root/soundfront.sfz");
    //    props.setValue("pluginFile", "Massive");
    
    StringPairArray props = sequenceProps.getAllProperties();
    StringArray keys = props.getAllKeys();
    
    //Create the sysexData track marker message
    int len = sizeof sysexTrackMarker;
    MidiMessage sysex = MidiMessage::createSysExMessage(sysexTrackMarker, len);
//    std::cout << " Writing sysexTrackMarker - length = "<< len << "\n";
    
    //std::unique_ptr<MidiMessageSequence>  seq(new MidiMessageSequence);
    MidiMessageSequence  seq;
    seq.addEvent(sysex);
    
    for (int i=0;i<props.size();i++)
    {
        if (keys[i] != "chainSeg")
        {
            String propertyStr = keys[i]+":"+props[keys[i]];
            int len = propertyStr.length();
            char buffer[128];
            propertyStr.copyToUTF8(buffer,128);
            MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
            std::cout << " Write sysex property - "<< propertyStr <<" "<<propertyStr.length() << "\n";
            seq.addEvent(sysex);
        }
        else
            std::cout << " Don't Write sysex property - "<< keys[i] << "\n";
            
    }
    
    for (int i=0;i<targetNoteTimes.size();i++)
    {
        //Property "tnt" - Write one record for each of the target note time
        String propertyStr = String("tnt:")+String(targetNoteTimes[i]);
        int len = propertyStr.length();
        char buffer[128];
        propertyStr.copyToUTF8(buffer,128);
        MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
//        td::cout << " Write sysex property - "<< propertyStr <<" "<<propertyStr.length() << "\n";
        seq.addEvent(sysex);
    }
    
    for (int i=0;i<bookmarkTimes.size();i++)
    {
        //Property "bookmark"
        double bm = bookmarkTimes[i];
        String propertyStr = String("bookmark:")+String(bm);
        int len = propertyStr.length();
        char buffer[128];
        propertyStr.copyToUTF8(buffer,128);
        MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
//        std::cout << " Write sysex property - "<< propertyStr <<" "<<propertyStr.length() << "\n";
        seq.addEvent(sysex);
    }
    for (int track=0;track<trackDetails.size();track++)
    {
        //Property "trackDetails"
        String propertyStr = String("trackDetails:")+String(track)
                                            +" "+String(trackDetails[track].playability)
                                            +" "+String(trackDetails[track].assignedChannel);
        int len = propertyStr.length();
        char buffer[128];
        propertyStr.copyToUTF8(buffer,128);
        MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
//        std::cout << " Write sysex property - "<< propertyStr <<" "<<propertyStr.length() << "\n";
        seq.addEvent(sysex);
    }
    //    std::cout << " Writing this many sysex records - "<< seq->getNumEvents()<< "\n";

    int tracksToCopy = midiFile.getNumTracks();
    if (loadedCkfFile)
        tracksToCopy--;
    
    MidiFile outputFile;
    short timeFormat = midiFile.getTimeFormat();
    for (int trkNumber=0;trkNumber<tracksToCopy;trkNumber++)
    {
        MidiMessageSequence seq;
        const MidiMessageSequence *theTrack = midiFile.getTrack(trkNumber);
        const int numEvents = theTrack->getNumEvents();
        for (int i=0;i<numEvents;i++)
        {
            MidiMessage msg = theTrack->getEventPointer(i)->message;
            seq.addEvent(msg);
        }
        outputFile.addTrack(seq);
    }
    outputFile.addTrack(seq);
    outputFile.setTicksPerQuarterNote(timeFormat);
    File tempFile = fileToSave.withFileExtension("ckf");
    fileToSave.deleteFile();
    FileOutputStream outputStream(fileToSave);
    outputFile.writeTo(outputStream);
    sendChangeMessage();
    setChangedFlag(false);
}

Array<Sequence::StepActivity> Sequence::chain (Array<int> selection, double interval)
{
    double startTime;
    double endTime;
    Array<Sequence::StepActivity> stepActivityList;
    if (selection.size()>0) //If not loading a file then we are in the chain command
    {
        startTime = theSequence.at(selection[0]).getTimeStamp();
        endTime = theSequence.at(selection.getLast()).getTimeStamp();
    }
    else
    {
        selection.ensureStorageAllocated(theSequence.size());
        startTime = 0;
        endTime = seqDurationInTicks;
        for (int step=0;step<theSequence.size();step++)
            selection.add(step);
    }
    
    theSequence[0].firstInChain = 0;
    theSequence[0].triggeredBy = -1;
    int firstInThisChain = 0;
    
    if (!getLoadingFile())
    {
        for (int step=0;step<theSequence.size();step++)
        {
            if (!selection.contains(step))
                continue;
            if ((startTime<=theSequence.at(step).getTimeStamp() && theSequence.at(step).getTimeStamp()<=endTime))
            {
                const int index = targetNoteTimes.indexOf(theSequence.at(step).getTimeStamp());
                if (index>=0)
                {
                    targetNoteTimes.remove(index);
                    const Sequence::StepActivity act = {step, true};
                    stepActivityList.add(act);
                }
                else
                {
                    const StepActivity act = {step, false};
                    stepActivityList.add(act);
                }
            }
        }
    }
    
//    targetNoteTimes.removeIf([startTime, endTime](double v) { return (startTime<=v && v<= endTime);} );
    int firstStep = -1;
    for (int step=0;step<theSequence.size();step++)
    {
        if (!selection.contains(step))
            continue;
        if ((startTime<=theSequence.at(step).getTimeStamp() && theSequence.at(step).getTimeStamp()<=endTime))
        {
            double startTimeDifference;
            if (firstStep==-1)
                firstStep = step;
            if (step==0)
                startTimeDifference = 99999;
            else
                startTimeDifference = theSequence[step].getTimeStamp()-theSequence[step-1].getTimeStamp();
            if (theSequence[step].chordTopStep!=-1 || startTimeDifference<=interval)
            {
                if (step>0)
                    theSequence[step-1].triggers = step;
                theSequence[step].triggeredBy = step-1;
                theSequence[step].firstInChain = firstInThisChain;
            }
            else
            {
                if (step>0)
                    theSequence[step-1].triggers = -1;//The last note in a chain triggers nothing
                theSequence[step].triggeredBy = -1; //This will be set based on the shortest note near the start of this group, done below
                firstInThisChain = step;
                theSequence[step].firstInChain = step;
                targetNoteTimes.add(theSequence[step].getTimeStamp());
    //            std::cout << "firstInChain " << step << " time " << targetNoteTimes.getLast() << "\n";
            }
        }
    }
    if (firstStep!=-1)
        targetNoteTimes.addIfNotAlreadyThere(theSequence[firstStep].getTimeStamp()); //First must always be a target note
    return stepActivityList;
}

/*=============================================================
 <#loadSequence#>
 */
//Loads the file in fileToLoad which must be set before calling if LoadType is load
void Sequence::loadSequence(LoadType type, Retain retainEdits)
{
    if (retainEdits == doNotRetainEdits)
    {
        targetNoteTimes.clear();
        chainingInterval = 12.0;
        bookmarkTimes.clear();
        setTempoMultiplier(1.0, false);
    }
    setLoadingFile(true);
    
    if (fileToLoad.getFileName().length() > 0 && type == Sequence::loadFile)
    {
        if (!fileToLoad.exists()) {
            std::cout << "File " << fileToLoad.getFileName() << " does not exist.\n";
            return;
        }
        String fileName = fileToLoad.getFileName();
        setScoreFile(fileToLoad); //This is just used by the file name display
        setFile (fileToLoad.withFileExtension(".ckf"));
        setLastDocumentOpened(fileToLoad);
        short fileType;
        FileInputStream inputStream(fileToLoad);
        if(!midiFile.readFrom(inputStream))
            jassert(false);
        //    std::cout << "numTracks " << midiFile.getNumTracks() << "\n";
        //    std::cout << "timeFormat " << midiFile.getTimeFormat() << "\n";
        
        fileType = 0;//midiFile.getFileType();
        //    std::cout << "fileType " << fileType << "\n";
        //    std::cout << "lastTimeStamp " << midiFile.getLastTimestamp() << "\n";
        
        
        
        MidiMessageSequence tempoChangeEvents;
        
        ppq = midiFile.getTimeFormat(); //This is only used in converting the midi file to a standard of 96 ppq.
        if (ppq<0)
            ppq = 96;
        
        midiFile.findAllTempoEvents(tempoChangeEvents);
        tempoChanges.clear();
        double prevTempo = -1;
        for (int i=0;i<tempoChangeEvents.getNumEvents();i++)
        {
            MidiMessage msg = tempoChangeEvents.getEventPointer(i)->message;
            msg.setTimeStamp(96.0*msg.getTimeStamp()/ppq);
            tempoChanges.push_back(msg);
            const double secPerQtr = msg.getTempoSecondsPerQuarterNote();
            const double tempo = (1.0/secPerQtr)*(60.0);
            if (tempo!=prevTempo)
//                std::cout << "tempoChange at " << msg.getTimeStamp() << " " << tempo << "\n";
            prevTempo = tempo;
        }
//        std::cout << "# tempoChangeEvents " << tempoChangeEvents.getNumEvents() << "\n";
        if (tempoChanges.size()==0)
        {
            MidiMessage msg = MidiMessage::tempoMetaEvent(60000000 / 120);
            tempoChanges.push_back(msg);
        }
        MidiMessage lastTempoChg = tempoChanges[tempoChanges.size()-1];
        lastTempoChg.setTimeStamp(9999999999);
        tempoChanges.push_back(lastTempoChg);
        
        tempoChangeEvents.clear();
        
        MidiMessageSequence timesigChangeEvents;
        midiFile.findAllTimeSigEvents(timesigChangeEvents);
        timeSigChanges.clear();
        for (int i=0;i<timesigChangeEvents.getNumEvents();i++)
        {
            MidiMessage msg = timesigChangeEvents.getEventPointer(i)->message;
            msg.setTimeStamp(96.0*msg.getTimeStamp()/ppq);
            msg.getTimeSignatureInfo(numerator, denominator);
            timeSigChanges.add(msg);
//            std::cout << "Time sig change " << msg.getTimeStamp() << " " << numerator << " " << denominator << "\n";
        }
        if (timeSigChanges.size() > 0)
            timeSigChanges[0].getTimeSignatureInfo(numerator, denominator);
        else
            timeSigChanges.add(MidiMessage::timeSignatureMetaEvent(4, 4));

//        timeIncrement = 96.0*tempo*(numerator/denominator)/60000.0;//Per tick.See spreadsheet in source directory. 96 is ppq, always the same
        
        numTracks = midiFile.getNumTracks();
        
        //Track overview
        trackDetails.clear();
//        nNoteOnsInTrack.clear();
//        nSustainMessagesInTrack.clear();
//        nSoftMessagesInTrack.clear();
        programChanges.clear();
        for (int i=0;i<16;i++)
            programChanges.push_back(-1);
        areThereProgramChanges = false;
        
        seqDurationInTicks = 0;
        for (int trk=0;trk<numTracks;trk++)
        {
            String txt = "";
            const MidiMessageSequence *theTrack = midiFile.getTrack(trk);
            const int numEvents = theTrack->getNumEvents();
    //        std::cout << "Track " << trk << " events " << numEvents << "\n";
            int nNoteOns = 0;
            int nSysex = 0;
            int nMeta = 0;
            int nTrackMeta = 0;
            int nChannelMeta = 0;
            int nKeySigMeta = 0;
            int nTimeSigMeta = 0;
            int nSustains = 0;
            int nSofts = 0;
            int nSostenutos = 0;
            int nOtherContr = 0;
            int channel = -1;
            TrackDetail trkDetail;
            trkDetail.startMeasure = -1;
            trkDetail.endMeasure = -1;
            for (int i=0;i<numEvents;i++) 
            {
                MidiMessage msg = theTrack->getEventPointer(i)->message;
//                std::cout << "trk " << trk << " channel " << msg.getChannel() << "\n";
                
                if (msg.isTextMetaEvent())
                {
                    txt += msg.getTextFromTextMetaEvent();
                }

                if (msg.isProgramChange())
                {
                    programChanges[msg.getChannel()-1] = msg.getProgramChangeNumber();
                    areThereProgramChanges = true;
                    trkDetail.instrument = String(MidiMessage::getGMInstrumentName(msg.getProgramChangeNumber()));
                }
                if (msg.isController())
                {
                    if (msg.isSustainPedalOn())
                        nSustains++;
                    if (msg.isSostenutoPedalOn()) nSostenutos++;
                    if (msg.isSoftPedalOn())
                        nSofts++;
                    if (!msg.isSustainPedalOn() && !msg.isSostenutoPedalOn() && !msg.isSoftPedalOn())
                        nOtherContr++;
                }
                if (msg.isNoteOn())
                {
                    if (theTrack->getTimeOfMatchingKeyUp(i) > seqDurationInTicks)
                        seqDurationInTicks = theTrack->getTimeOfMatchingKeyUp(i);
                    nNoteOns++;
                    const double ts = msg.getTimeStamp();
                    if (trkDetail.startMeasure == -1)
                        trkDetail.startMeasure = ts; //Will convert to measure later.
                    trkDetail.endMeasure = ts; //Will convert to measure later
                    channel = msg.getChannel();
                }
                if (msg.isSysEx()) nSysex++;
                if (msg.isMetaEvent()) nMeta++;
                if (msg.isTrackMetaEvent()) nTrackMeta++;
                if (msg.isMidiChannelMetaEvent()) nChannelMeta++;
                if (msg.isKeySignatureMetaEvent()) nKeySigMeta++;
                if (msg.isTimeSignatureMetaEvent()) nTimeSigMeta++;
                if (msg.isTrackNameEvent())
                    trkDetail.description = msg.getTextFromTextMetaEvent();
                else
                    trkDetail.description = "";
            }
            trkDetail.description = txt;
            trkDetail.nNotes = nNoteOns;
            trkDetail.nSustains = nSustains;
            trkDetail.nSofts = nSofts;
            trkDetail.originalChannel = channel;
//            if (trkDetail.description.length()!=0 || trkDetail.instrument.length()!=0 ||
//                        trkDetail.nNotes!=0 || trkDetail.nSustains!=0 || trkDetail.nSofts !=0)
//                trkDetail.showInList = true;
//            else
//                trkDetail.showInList = false;
            
            if (trkDetail.nNotes>0)
                trkDetail.playability = TrackTypes::Track_Play;
            else if (trkDetail.nSustains>0 || trkDetail.nSofts>0)
                trkDetail.playability = TrackTypes::Track_Controllers;
            else
                trkDetail.playability = TrackTypes::Track_Other;
            trackDetails.add(trkDetail);
        }
        seqDurationInTicks = 96.0*seqDurationInTicks/ppq;
//        std::cout << "seqDurationInTicks " << seqDurationInTicks << "\n";
        
        //Beat & measure lines
        beatTimes.clear();
        measureTimes.clear();
        int tickPosition = 0;
        for (int timeSigIndex=0;timeSigChanges.size()>timeSigIndex;timeSigIndex++)
        {
            timeSigChanges[timeSigIndex].getTimeSignatureInfo(numerator, denominator);
            const float ticksPerBeat = getPPQ() * 4.0/(float)denominator;
            
            int startTick;
            if (timeSigIndex==0)
                startTick = 0;
            else
                startTick = timeSigChanges[timeSigIndex].getTimeStamp();
            int endTick;
            if (timeSigChanges.size()>timeSigIndex+1)
                endTick = timeSigChanges[timeSigIndex+1].getTimeStamp();
            else
                endTick = seqDurationInTicks;
            
            //        bool up = false;
            const int sectionDurationInBeats = (endTick - startTick) / ticksPerBeat;
            for (int beat=0;beat<sectionDurationInBeats;beat++)
            {
                if (beat%numerator == 0) //Measure
                {
                    measureTimes.push_back(tickPosition);
//                    if (measureTimes.size()<30)
//                        std::cout << measureTimes.size() <<  " measuretime "  << tickPosition << "\n";
                    beatTimes.push_back(tickPosition);
                }
                else //Beat
                    beatTimes.push_back(tickPosition);
                tickPosition += ticksPerBeat;
            }
        }
        measureTimes.push_back(seqDurationInTicks);
        beatTimes.push_back(seqDurationInTicks);
        
        //Compute start and end measure in each playable track
        for (int trk=0;trk<trackDetails.size();trk++)
        {
            bool foundStart = false;
            if (trackDetails[trk].startMeasure != -1) //If we found any note
            {
                TrackDetail trkDetail = trackDetails[trk];
                //The following were determined during the scan of all events (including noteOns) in each track
                const int startInTicks = 96.0*trackDetails[trk].startMeasure/ppq;
                const int endInTicks = 96.0*trackDetails[trk].endMeasure/ppq;
//                std::cout << "start ticks, end ticks " << trk <<" "<<startInTicks << " " << endInTicks << "\n";
                int m;
                for (m=0;m<measureTimes.size()-1;m++)
                {
//                    std::cout << "measure, time " << m <<" "<<measureTimes[m] << "\n";
                    if (!foundStart && measureTimes[m]<=startInTicks && startInTicks<measureTimes[m+1])
                    {
                        trkDetail = trackDetails[trk];
                        trkDetail.startMeasure = m+1;
                        trackDetails.set(trk, trkDetail);
                        foundStart = true;
                    }
                    if (foundStart && measureTimes[m]<=endInTicks && endInTicks<measureTimes[m+1])
                    {
                        trkDetail = trackDetails[trk];
                        trkDetail.endMeasure = m+1;
                        trackDetails.set(trk, trkDetail);
                        break;
                    }
                }
            }
        }
        
        MidiMessageSequence ckfSysex; //All sysex records from our app's "properties" track
    //    std::cout << "nTracks " << numTracks << "\n";
        loadedCkfFile = false;
        if (midiFile.getNumTracks()>1)
        {
            midiFile.getTrack(numTracks-1)->extractSysExMessages(ckfSysex);
    //        std::cout << "nSysexRecords " << ckfSysex.getNumEvents() << "\n";
            if (ckfSysex.getNumEvents()>0)
            {
                int sysexSize = ckfSysex.getEventPointer(0)->message.getSysExDataSize();
                HeapBlock<char> allSysex;
                allSysex.allocate(sysexSize, true);
                
                char *pSysex;
                pSysex = (char*) ckfSysex.getEventPointer(0)->message.getSysExData();
                String sysexStr;
                for (int i=0; i<sysexSize; i++)
                {
                    char ch = *(pSysex+i);
                    String st = String::charToString(ch);
                    sysexStr.append(st, 1);
                }
//                std::cout << "read sysexStr - length = " << sysexStr.length() << " " << "\n";
                if (sysexSize>0)
                {
                    if (0==memcmp(sysexTrackMarker, pSysex, sizeof(sysexTrackMarker)))
                    {
//                        std::cout <<"This is a ckf file."<<"\n";
                        loadedCkfFile = true;
                        numTracks -= 1; //Don't read sysex track into sequence
                    }
//                    else
//                        std::cout <<"This is not a ckf file."<<"\n";
                }
            }
        }
        if (loadedCkfFile)
        {
            sequenceProps.clear();
            for (int sysexBlockNum=1; sysexBlockNum<ckfSysex.getNumEvents(); sysexBlockNum++)
            {
                int sysexSize = ckfSysex.getEventPointer(sysexBlockNum)->message.getSysExDataSize();
                char *pSysex;
                pSysex = (char*) ckfSysex.getEventPointer(sysexBlockNum)->message.getSysExData();
                String sysexStr;
                for (int i=0; i<sysexSize; i++)
                {
                    char ch = *(pSysex+i);
                    String st = String::charToString(ch);
                    sysexStr.append(st, 1);
                }
                String key = sysexStr.upToFirstOccurrenceOf(":", false, true);
                String value = sysexStr.fromFirstOccurrenceOf(":", false, true);
//                std::cout <<"sysexStr - key, value "<<key << " *** " << value <<"\n";
                if (key == "bookmark")
                {
                    double bm = value.getDoubleValue();
//                    std::cout <<"read bookmark "<< bm <<"\n";
                    bookmarkTimes.add(bm);
                }
                else if (key == "trackDetails")
                {
                    StringArray values;
                    values.addTokens(value, " ", "\"");
                    const int track = values[0].getIntValue();
                    const int playability = values[1].getIntValue();
                    const int assignedChannel = values[2].getIntValue();
                    TrackDetail trkDet = trackDetails[track];
                    trkDet.playability = playability;
                    trkDet.assignedChannel = assignedChannel;
                    trackDetails.set(track, trkDet);
//                    std::cout << "loadedTrack " <<track <<" playability "<<playability <<" assignedChannel "<<assignedChannel <<"\n";
                }
                else if (key == "tnt") //targetNoteTimes
                {
                    //                    std::cout <<"read forced nontarget note "<< value <<"\n";
                    targetNoteTimes.add(value.getDoubleValue());
                }
                else
                    sequenceProps.setValue(key, value);
            }
        }
        else //midi file
        {
            sequenceProps.setValue("waitForFirstNote", var(true));
            sequenceProps.setValue("tempoMultiplier", var(1.0));
            sequenceProps.setValue("chordTimeHumanize", var(0.0));
            sequenceProps.setValue("chordVelocityHumanize", var(1.0));
            sequenceProps.setValue("horizontalScale", var(1.0));
            sequenceProps.setValue("autoPlaySustains", var(true));
            sequenceProps.setValue("autoPlaySofts", var(true));
            sequenceProps.setValue("reVoiceChords", var(true));
            sequenceProps.setValue("exprVelToOriginalValRatio", var(0.5));
        }
        autoPlaySustains = sequenceProps.getBoolValue("autoPlaySustains", var(true));
        autoPlaySofts = sequenceProps.getBoolValue("autoPlaySofts", var(true));
        reVoiceChords = sequenceProps.getBoolValue("reVoiceChords", var(true));
        waitForFirstNote = sequenceProps.getBoolValue("waitForFirstNote", var(true));
        setTempoMultiplier(sequenceProps.getDoubleValue("tempoMultiplier"), false);
        chordTimeHumanize = sequenceProps.getDoubleValue("chordTimeHumanize", var(2.0));
        chordVelocityHumanize = sequenceProps.getDoubleValue("chordVelocityHumanize", var(1.0));
        exprVelToScoreVelRatio = sequenceProps.getDoubleValue("exprVelToScoreVelRatio", var(0.5));
        
        StringPairArray props = sequenceProps.getAllProperties();
        StringArray keys = props.getAllKeys();
        StringArray vals = props.getAllValues();
//        std::cout <<"Properties loaded: " <<"\n";
//        for (int i=0;i<props.size();i++)
//            std::cout <<" property: " << keys[i] << " / " << vals[i] <<"\n";
        ckfSysex.clear();
        sendChangeMessage(); //Is this needed?
        setChangedFlag (false);
        recordsWithEdits.clear();
        
        for (int trkNumber=0;trkNumber<numTracks;trkNumber++)
        {
            const MidiMessageSequence *theTrack = midiFile.getTrack(trkNumber);
            const int numEvents = theTrack->getNumEvents();
            recordsWithEdits.add(Array<NoteWithOffTime>());
            recordsWithEdits.getLast().ensureStorageAllocated(numEvents);
        }
        for (int trkNumber=0;trkNumber<numTracks;trkNumber++)
        {
            const MidiMessageSequence *theTrack = midiFile.getTrack(trkNumber);
            const int numEvents = theTrack->getNumEvents();
            for (int i=0;i<numEvents;i++)
            {
                if (theTrack->getEventPointer(i)->message.isNoteOn())
                {
                    NoteWithOffTime msg(trkNumber, theTrack->getEventPointer(i)->message, theTrack->getTimeOfMatchingKeyUp(i));
                    msg.track = trkNumber;
                    if (msg.offTime <= msg.getTimeStamp()) //In a correct sequence this should not happen
                        msg.offTime = msg.getTimeStamp()+50; //But if it does, turn it into a short note  with non neg duration
                    msg.setTimeStamp(96.0*msg.getTimeStamp()/ppq);
                    msg.offTime = 96.0*msg.offTime/ppq;
                    recordsWithEdits[trkNumber].add(msg);
                }
            }
        }
    }
    //End of reloading file ####################################################################################
    theSequence.clear();
    
//      Transfer tracks to "theSequence"
    theControllers.clear();
    for (int trkNumber=0;trkNumber<numTracks;trkNumber++)
    {
        const MidiMessageSequence *theTrack = midiFile.getTrack(trkNumber);
        const int numEvents = theTrack->getNumEvents();
        //Notes
        if (isActiveTrack(trkNumber))
        {
//            std::cout
//            << "Loading track "<< trkNumber
//            << ", noteOns "<< nNoteOnsInTrack[trkNumber]
//            << "\n";
            int recordNumber = 0;
            for (int i=0;i<numEvents;i++)
            {
                if (theTrack->getEventPointer(i)->message.isNoteOn())
                {
                    NoteWithOffTime msg(trkNumber, theTrack->getEventPointer(i)->message, theTrack->getTimeOfMatchingKeyUp(i));
                    msg.track = trkNumber;
                    if (msg.offTime <= msg.getTimeStamp()) //In a correct sequence this should not happen
                        msg.offTime = msg.getTimeStamp()+50; //But if it does, turn it into a short note  with non neg duration
                    msg.setTimeStamp(96.0*msg.getTimeStamp()/ppq);
                    msg.offTime = 96.0*msg.offTime/ppq;
                    msg.editedMessageIndex = recordNumber;
                    theSequence.push_back(msg);
                    recordNumber++;
//                    std::cout <<theSequence.size()<< " Add note: Track, TimeStamp, NN " << msg.track  << ", "
//                    << msg.getTimeStamp()<< " " << msg.getNoteNumber() <<"\n";
                }
            }
        }
        //Controllers
        for (int i=0;i<numEvents;i++)
        {
            if (theTrack->getEventPointer(i)->message.isController())
            {
                ControllerMessage ctrMsg(trkNumber, theTrack->getEventPointer(i)->message);
                theControllers.push_back(ctrMsg);
//                                    std::cout << "Add Controller: Step, Time " << i << ", " << ctrMsg.getTimeStamp()
//                                    << " Track " << trkNumber
//                                    << " Channel " << ctrMsg.getChannel()
//                                    << " cc " << ctrMsg.getControllerNumber()
//                                    << " Value " << ctrMsg.getControllerValue()
//                                    <<"\n";
            }
        }
    }
    
    if (theSequence.size()>0)
    {
        std::sort(theSequence.begin(), theSequence.end());
        std::sort(theControllers.begin(), theControllers.end());
        
        //Extract pedal changes
        String sustainPedalDirection = "";
        String softPedalDirection = "";
//        double prevTimeStamp = -1;
        sustainPedalChanges.clear();
        softPedalChanges.clear();
        for (int i=0;i<theControllers.size();i++)
        {
            ControllerMessage ctrMsg = theControllers[i];
            ctrMsg.setTimeStamp(96.0*ctrMsg.getTimeStamp()/ppq);
            if (ctrMsg.isSustainPedalOn()||ctrMsg.isSustainPedalOff())   //Sustain pedal
            {
                String prevDirection = sustainPedalDirection;
                if (ctrMsg.isSustainPedalOn())
                {
                    if (sustainPedalDirection!="down")
                        sustainPedalDirection ="down";
                }
                else if (ctrMsg.isSustainPedalOff())
                {
                    if (sustainPedalDirection!="up")
                        sustainPedalDirection = "up";
                }
                if (prevDirection != sustainPedalDirection && !(prevDirection=="" && sustainPedalDirection=="up"))
                {
//                    std::cout << ctrMsg.getTimeStamp() << " sustainPedalChange "<<ctrMsg.getControllerValue()<<"\n";
                    sustainPedalChanges.push_back(ctrMsg);
//                    prevTimeStamp = ctrMsg.getTimeStamp();
//                    std::cout << "i, Time " << i << ", " << 96.0*ctrMsg.getTimeStamp()/ppq
//                    << " Channel " << ctrMsg.getChannel()
//                    << " cc " << ctrMsg.getControllerNumber()
//                    << " Value " << ctrMsg.getControllerValue()
//                    << " " << sustainPedalDirection
//                    << " Type:" << ctrMsg.getDescription()
//                    <<"\n";
                }
            }
            else if (ctrMsg.isSoftPedalOn() || ctrMsg.isSoftPedalOff())  //Soft pedal
            {
                String prevDirection = softPedalDirection;
                if (ctrMsg.getControllerValue()>=63)
                {
                    if (softPedalDirection!="down")
                        softPedalDirection ="down";
                }
                else if (ctrMsg.getControllerValue()<63)
                {
                    if (softPedalDirection!="up")
                        softPedalDirection = "up";
                }
                if (prevDirection != softPedalDirection && !(prevDirection=="" && softPedalDirection=="up"))
                {
                    softPedalChanges.push_back(ctrMsg);
                    
//                     std::cout << "i, Time " << i << ", " << ctrMsg.getTimeStamp()
//                     //            << " Channel " << ctrMsg.getChannel()
//                     //            << " cc " << ctrMsg.getControllerNumber()
//                     //            << " Value " << ctrMsg.getControllerValue()
//                     << " " << softPedalDirection
//                     << " Type:" << ctrMsg.getDescription()
//                     <<"\n";
                }
            }
        }
        
        //Humanize chord note start times and velocities
        srand(0);
        double thisChordTimeStamp;
        for (int step=0; step<theSequence.size();step++)
        {
//            bool print = false;
            Array<int> chord;
            int chordTopStep = step;
            
            if (theSequence[step].chordTopStep==-2) // -2 means not determined yet
            {
                thisChordTimeStamp = theSequence[step].getTimeStamp();
                while (theSequence[step].getTimeStamp() == thisChordTimeStamp)
                {
                    chord.add(step++);
//                    if (step <20)
//                        std::cout<< "first pass chord add " << step << "\n";

                }
                step--;
            }
            else
            {
                if (theSequence[step].chordTopStep == -1)
                {
                    int chordFirstStep = step;
                    thisChordTimeStamp = theSequence[step].getTimeStamp();
                    chord.add(step++);
                    while (theSequence[step].chordTopStep == chordFirstStep)
                    {
                        chord.add(step++);
                    }
                    step--;
                }
            }
            if (chord.size()>1)
            {
                double timeToNextNote;
                if (step<theSequence.size()-1)
                    timeToNextNote = theSequence[step+1].getTimeStamp()-thisChordTimeStamp;
                else
                    timeToNextNote = DBL_MAX;
                double localTimeFuzz = std::min(timeToNextNote*0.33,chordTimeHumanize);
                
//                if (print)
//                    std::cout<<step<< " localFuzz "<< localFuzz<< "\n";
                bool allSameVelocities = true;
                const int firstVelocity =  theSequence[chord[0]].getVelocity();
                for (int i=1; i<chord.size(); i++)
                {
                    theSequence[chord[i]].chordTopStep = chord[0];
                    if (theSequence[chord[i]].getVelocity() != firstVelocity)
                        allSameVelocities = false;
                    
                    const int temp = localTimeFuzz*100;
                    double randAdd;
                    if (temp==0)
                        randAdd = 0;
                    else
                        randAdd = rand()%temp/100.0;
                    theSequence[chord[i]].setTimeStamp(thisChordTimeStamp+randAdd);
//                    theSequence[chord[i]].offTime = theSequence[chord[i]].offTime + randAdd;
//                    if (print)
//                        std::cout<<i<< " thisChordTS, newTS "<<thisChordTimeStamp<< " " << theSequence[chord[i]].getTimeStamp()<< "\n";
                }
                if (/*allSameVelocities ||*/ reVoiceChords)
                {
//                    std::cout<<"Found chord at " << step<< " notes "<< chord.size()  << "\n";
                    if (chord.size()==2)
                    {
                        const float topNoteVel = theSequence[chord[0]].getFloatVelocity();
                        theSequence[chord[1]].setVelocity(topNoteVel * (1.0 - 0.3*chordVelocityHumanize));
//                        std::cout<< 1 << " " << chord[1]  <<" "<< (int) theSequence[chord[1]].getVelocity() << "\n";
                    }
                    else // (chord.size()>2)
                    {
                        const float topNoteVel = theSequence[chord[0]].getFloatVelocity();
//                        std::cout<< 0 << " " << chord[0]  << " " << (int) theSequence[chord[0]].getVelocity() << "\n";
                        for (int j=1;j<chord.size()-1;j++)
                        {
                            const float newVel = topNoteVel * (1.0 - 0.4*chordVelocityHumanize);
//                            const int thisStep = chord[j];
                            theSequence[chord[j]].setVelocity(newVel);
//                            std::cout<< j << " " << chord[j]  << " " << (int) theSequence[chord[j]].getVelocity() << "\n";
                        }
                        const float newVel = topNoteVel * (1.0 - 0.2*chordVelocityHumanize);
                        theSequence[chord.getLast()].setVelocity(newVel);
//                        std::cout<< chord.size()-1
//                        << " " << chord.getLast()
//                        << " " << (int) theSequence[chord.getLast()].getVelocity() << "\n";
                    }
                }
            }
            theSequence[chordTopStep].chordTopStep=-1;
        }
        
        std::sort(theSequence.begin(), theSequence.end());
//        
//        //Determine which notes are triggeredBy previous notes
        if(targetNoteTimes.size()>0) //Non zero if loaded from ckf file or previously created for midi file by chain ()
        {
            int firstInThisChain = 0;
            double prevTS = -1;
            for (int step=0; step<theSequence.size();step++)
            {
//                std::cout<< step
//                << " ts " << theSequence[step].getTimeStamp()
//                << " firstInChain" << targetNoteTimes.contains(theSequence[step].getTimeStamp()) << "\n";
                if (prevTS != theSequence[step].getTimeStamp() && targetNoteTimes.contains(theSequence[step].getTimeStamp()))
                {
                    if (step>0)
                        theSequence[step-1].triggers = -1;//Last note in chain triggers nothing
                    theSequence[step].triggeredBy = -1; //This will be set based on the shortest note near the start of this group, done below
                    firstInThisChain = step;
                    theSequence[step].firstInChain = step;
                }
                else
                {
                    if (step>0)
                    {
                        theSequence[step-1].triggers = step;
                        theSequence[step].triggeredBy = step-1;
                    }
                    else
                        theSequence[step].triggeredBy = step-1;
                    theSequence[step].firstInChain = firstInThisChain;
                }
                prevTS = theSequence[step].getTimeStamp();
            }
        }
        else
        {
            chain(Array<int>(),chainingInterval);
        }
    
        
        //Find chainTriggers and set this property for all steps in a given chain
        int step = 0;
        int chainTrigger;
        while (step < theSequence.size())
        {
            //If this step is a firstInChain scan all notes with time stamp equal to that of the firstInChain for the shortest note
            if (theSequence[step].triggeredBy==-1 && theSequence[step].chordTopStep==-1)
            {
                chainTrigger = step;
//                int highestNote = -1;
                int stepOfhighestNote = step;
                int subStep = step;
                while (subStep<theSequence.size()-1 && theSequence[subStep].getTimeStamp()==theSequence[theSequence[step].firstInChain].getTimeStamp())
                {
                    if (theSequence[subStep].getNoteNumber() >= theSequence[stepOfhighestNote].getNoteNumber())
                    {
                        stepOfhighestNote = subStep;
                    }
                    subStep++;
                }
                chainTrigger = stepOfhighestNote;
            }
            theSequence[step].chainTrigger = chainTrigger;
            step++;
        }

        //Store highest velocity in each chain in every step of the chain
        for (int step=0;step<theSequence.size();step++)
        {
            int firstInChain = step;
            int highestVelocity = -1;
            int firstStep = step;
            bool enteredLoop = false;
            while (step<theSequence.size() && theSequence[step].firstInChain==firstInChain)
            {
                enteredLoop = true;
                if (theSequence[step].getVelocity() > highestVelocity)
                    highestVelocity = theSequence[step].getVelocity();
                step++;
            }
            if (enteredLoop)
                step--;
            
            //Store highest velocity in this chain in every step of this chain
            for (int i=firstStep; i<=step; i++ )
            {
                theSequence[i].highestVelocityInChain = highestVelocity;
            }
        }
        
        //Determine which steps are triggeredNotes and triggeredOffNotes
        //triggeredNotes are steps that start no later than the triggeredNoteLimit from the chainTrigger.
        //triggeredOffNotes are triggeredNotes that end before the END of the next chainTrigger note.
        for (int step=0; step<theSequence.size();step++)
        {
            if ( theSequence[step].getTimeStamp() <= theSequence[theSequence[step].chainTrigger].getTimeStamp()+triggeredNoteLimit)
            {
                theSequence[step].triggeredNote=true;
                //Scan for nextFirstInChain
                int nextFirstInChain = theSequence[step].firstInChain+1;
                while (nextFirstInChain<theSequence.size()-1 && theSequence[nextFirstInChain].triggeredBy != -1)
                    nextFirstInChain++;
                
                //Two cases for a step to be a triggeredOffNote:
                //1) Where a the step ends before the next chain trigger
                //2) Where the step's offtime is no more than the start of any triggered note of the chain that starts after it)
                if (nextFirstInChain<theSequence.size()-1
                    && theSequence[step].triggeredNote && theSequence[step].offTime < theSequence[nextFirstInChain].getTimeStamp())
                    theSequence[step].triggeredOffNote = true;
                else
                    theSequence[step].triggeredOffNote = false;
                
                //Scan for case (2) above
                int i = nextFirstInChain;
                bool shouldSustain = false;
                while (theSequence[i].getTimeStamp() <= theSequence[nextFirstInChain].getTimeStamp()+triggeredNoteLimit) //All triggered notes in next chain
                {
                    if (theSequence[step].offTime>theSequence[i].getTimeStamp())
                    {
                        shouldSustain = true;
                        break;
                    }
                    i++;
                }
                theSequence[step].triggeredOffNote = !shouldSustain;
            }
            else
                theSequence[step].autoplayedNote = true;
        }
    }
    setLoadingFile(false);
    dumpData(0, 20, -1);
    //We assume that rewind will always be called after loadSequence, and that rewind calls sendChangeMessage
} //End of loadSequence

SortedSet<int> Sequence::getNotesUsed(int &minNote, int &maxNote)
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

uint Sequence::varintLength(char* input, uint inputSize) {
    uint ret = 0;
    uint i;
    for (i = 0; i < inputSize; i++) {
        ret |= (input[i] & 127) << (7 * i);
        //If the next-byte flag is set
        if(!(input[i] & 128)) {
            break;
        }
    }
    return i+1;
}

int Sequence::myConvertFromBase64 (OutputStream& binaryOutput, StringRef base64TextInput)
{
    
    std::cout << "stRef length in convertFromBase64 " << base64TextInput.length() <<"\n";
    int length = 0;
    for (String::CharPointerType s = base64TextInput.text; ! s.isEmpty();)
    {
        uint8 data[4];
        
        for (int i = 0; i < 4; ++i)
        {
            uint32 c = (uint32) s.getAndAdvance();
            
            if (c >= 'A' && c <= 'Z')         c -= 'A';
            else if (c >= 'a' && c <= 'z')    c -= 'a' - 26;
            else if (c >= '0' && c <= '9')    c += 52 - '0';
            else if (c == '+')                c = 62;
            else if (c == '/')                c = 63;
            else if (c == '=')                { c = 64; if (i <= 1) return false; }
            else                              return -1;
            
            data[i] = (uint8) c;
        }
        
        binaryOutput.writeByte ((char) ((data[0] << 2) | (data[1] >> 4)));
        length++;
        if (data[2] < 64)
        {
            binaryOutput.writeByte ((char) ((data[1] << 4) | (data[2] >> 2)));
            length++;
            if (data[3] < 64)
            {
                binaryOutput.writeByte ((char) ((data[2] << 6) | data[3]));
                length++;
            }
        }
    }
    
    return length;
}

void Sequence::dumpData(int start, int end, int nn)
{
    if (start==-1)
    {
        start = 0;
        end = (int)theSequence.size();
        nn = -1;
    }
    std::cout
    << " step "
    << " track "
    << " channel "
    << " nn "
    << " note "
    << " timeStamp "
    << " offTime "
    << " firstInChain "
    << " triggers "
    << " triggeredBy "
    << " chainTrigger "
    << " triggered "
    << " triggeredOff "
    << " chordTop "
    << " edMsgNdx"
    << "\n";
    if (end>theSequence.size())
        end = theSequence.size();
    for (int i=start; i<theSequence.size() && i<end;i++)
    {
        if (nn==-1 || theSequence[i].getNoteNumber()==nn )
        {
            String note = MidiMessage::getMidiNoteName (theSequence[i].getNoteNumber(), true, true, 3);
            std::cout
            << i <<" "
            << (int)theSequence[i].track <<" "
            << theSequence[i].getChannel() <<" "
            << theSequence[i].getNoteNumber() <<" "
            << note<<" "
            << theSequence[i].getTimeStamp()<<" "
            << theSequence[i].offTime<<" "
            << theSequence[i].firstInChain<<" "
            << theSequence[i].triggers<<" "
            << theSequence[i].triggeredBy<<" "
            << theSequence[i].chainTrigger<<" "
            << theSequence[i].triggeredNote<<" "
            << theSequence[i].triggeredOffNote<<" "
            << theSequence[i].chordTopStep<<" "
            << theSequence[i].editedMessageIndex<<" "
            <<"\n";
        }
    }
}

std::vector<NoteWithOffTime> Sequence::getNotesInTimeRange(double minTimeStamp,
                                                                double maxTimeStamp)
{
    std::vector<NoteWithOffTime> notes;
    notes.clear();
    for (int noteIndex=0;noteIndex<theSequence.size();noteIndex++)
    {
        NoteWithOffTime msg = theSequence.at(noteIndex);
        if (msg.offTime>=minTimeStamp && msg.getTimeStamp()<=maxTimeStamp)
            notes.push_back(msg);
    }
    return notes;
}

Array<MidiMessage> Sequence::getCurrentTimeSig(double timeStamp)
{
    return Array<MidiMessage>();
}
