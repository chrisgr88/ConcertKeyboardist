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
    MidiMessageSequence  sysexSeq;
    sysexSeq.addEvent(sysex);
    
    for (int i=0;i<props.size();i++) //Save properties actually stored in props
    {
        String propertyStr = keys[i]+":"+props[keys[i]];
        int len = propertyStr.length();
        char buffer[128];
        propertyStr.copyToUTF8(buffer,128);
        MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
        std::cout << " Write sysex property - "<< propertyStr <<" "<<propertyStr.length() << "\n";
        sysexSeq.addEvent(sysex);
    }
    
    for (int i=0;i<targetNoteTimes.size();i++)
    {
        //Property "tnt" - Write one record for each of the target note time
        String propertyStr = String("tnt:")+String(targetNoteTimes[i]);
        int len = propertyStr.length();
        char buffer[128];
        propertyStr.copyToUTF8(buffer,128);
        MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
//        td::cout << " Write sysex propertyStr - "<< propertyStr <<" "<<propertyStr.length() << "\n";
        sysexSeq.addEvent(sysex);
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
//        std::cout << " Write sysex bookmark - "<< propertyStr <<" "<<propertyStr.length() << "\n";
        sysexSeq.addEvent(sysex);
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
//        std::cout << " Write sysex trackDetails - "<< propertyStr <<" "<<propertyStr.length() << "\n";
        sysexSeq.addEvent(sysex);
    }

    for (int chIndex=0;chIndex<chords.size();chIndex++)
    {
//        scaleFactor=1.0f;
//        timeSpec="TimeSpec"; timeRandScale=1.0f; timeRandSeed=1;
//        velSpec="VelSpec"; velRandScale=1.0f; velRandSeed=1;
        //Property "chordDetails"
        String propertyStr = String("chordDetails:")+String(chords[chIndex].timeStamp) +" "+String(chords[chIndex].scaleFactor)+
        +" "+chords[chIndex].timeSpec+" "+String(chords[chIndex].timeRandScale)+" "+String(chords[chIndex].timeRandSeed)
        +" "+chords[chIndex].velSpec+" "+String(chords[chIndex].velRandScale)+" "+String(chords[chIndex].velRandSeed);
        int len = propertyStr.length();
        char buffer[128];
        propertyStr.copyToUTF8(buffer,128);
        MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
        if (chIndex<5)
            std::cout << " Write sysex chordDetails - "<<chIndex<<" "<< propertyStr <<" "<<propertyStr.length() << "\n";
        sysexSeq.addEvent(sysex);
        //Property "chNote" : The notes in this chord
        for (int note=0;note<chords[chIndex].notePointers.size();note++)
        {
            //A unique signature for the note at this exact timeStamp
            String noteId = String(chords[chIndex].notePointers[note]->track)
                                            +"_"+String(chords[chIndex].notePointers[note]->channel)
                                            +"_"+String(chords[chIndex].notePointers[note]->noteNumber);
            const int offset = chords[chIndex].notePointers[note]->getTimeStamp() - chords[chIndex].timeStamp;
            
            String propertyStr = String("chordNote:")+String(chIndex)+" "+String(offset)+" "+noteId;
            int len = propertyStr.length();
            char buffer[128];
            propertyStr.copyToUTF8(buffer,128);
            MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
            if (chIndex<5)
                std::cout << " Write chord note - "<<propertyStr <<" "<<propertyStr.length() << "\n";
            sysexSeq.addEvent(sysex);
        }
    }
    //    std::cout << " Number of sysex records written - "<< seq->getNumEvents()<< "\n";

    int tracksToCopy = midiFile.getNumTracks();
    if (loadedCkfFile)
        tracksToCopy--;
    
    MidiFile outputFile;
    short timeFormat = 96;//midiFile.getTimeFormat();
    for (int trk=0;trk<tracksToCopy;trk++)
    {
        MidiMessageSequence trackSeq;
        const MidiMessageSequence *theTrack = midiFile.getTrack(trk);
        const int numEvents = theTrack->getNumEvents();
        for (int i=0;i<numEvents;i++)
        {
            MidiMessage msg = theTrack->getEventPointer(i)->message;
            if (!msg.isNoteOn())
            {
                msg.setTimeStamp(96.0*msg.getTimeStamp()/ppq);
                trackSeq.addEvent(msg);
            }
        }
        for (int step=0;step<allNotes[trk].size();step++)
        {
            MidiMessage onMsg = MidiMessage::noteOn(allNotes[trk][step]->channel,
                                                  allNotes[trk][step]->noteNumber,
                                                  allNotes[trk][step]->velocity);
            if (allNotes[trk][step]->chordIndex>=0) //If in a chord
                onMsg.setTimeStamp(chords[allNotes[trk][step]->chordIndex].timeStamp); //Write the chords timestamp for all its notes
            else
                onMsg.setTimeStamp(allNotes[trk][step]->getTimeStamp()); //Otherwise write the actual note's timestamp
            trackSeq.addEvent(onMsg);
            MidiMessage offMsg = MidiMessage::noteOff(allNotes[trk][step]->channel,
                                                  allNotes[trk][step]->noteNumber,
                                                  0.0f);
            offMsg.setTimeStamp(allNotes[trk][step]->offTime);
            trackSeq.addEvent(offMsg);
        }
        trackSeq.sort();
        outputFile.addTrack(trackSeq);
    }
    outputFile.addTrack(sysexSeq);
    outputFile.setTicksPerQuarterNote(timeFormat);
    File tempFile = fileToSave.withFileExtension("ckf");
    fileToSave.deleteFile();
    FileOutputStream outputStream(fileToSave);
    outputFile.writeTo(outputStream);
    sendChangeMessage();
    setChangedFlag(false);
}

//This is for when a plain midi file is loaded or when called from chain command
Array<Sequence::StepActivity> Sequence::chain (Array<int> selection, double interval)
{
    //If there was no selection, construct a selection array with all steps
    double startTime;
    double endTime;
    if (selection.size()>0)
    {
        startTime = theSequence.at(selection[0])->getTimeStamp();
        endTime = theSequence.at(selection.getLast())->getTimeStamp();
    }
    else
    {
        selection.ensureStorageAllocated(theSequence.size());
        startTime = 0;
        endTime = seqDurationInTicks;
        for (int step=0;step<theSequence.size();step++)
            selection.add(step);
    }
    
    //For steps in selection, construct stepActivityList from the targetNoteTimes array.
    //Entries in stepActivityList are {int step; bool active}
    Array<Sequence::StepActivity> stepActivityList;
    if (!getLoadingFile())
    {
        for (int step=0;step<theSequence.size();step++)
        {
            if (!selection.contains(step))
                continue;
//            if ((startTime<=theSequence.at(step).timeStamp && theSequence.at(step).timeStamp<=endTime))
//            {
                const int index = targetNoteTimes.indexOf(theSequence.at(step)->getTimeStamp());
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
//            }
        }
    }

    theSequence[0]->firstInChain = 0;
    theSequence[0]->triggeredBy = -1;
    int firstInThisChain = 0;
    int firstStep = -1;
    for (int step=0;step<theSequence.size();step++)
    {
        if (!selection.contains(step))
            continue;
        if ((startTime<=theSequence.at(step)->getTimeStamp() && theSequence.at(step)->getTimeStamp()<=endTime))
        {
            double startTimeDifference;
            if (firstStep==-1)
                firstStep = step;
            if (step==0)
                startTimeDifference = DBL_MAX;
            else
                startTimeDifference = theSequence[step]->getTimeStamp()-theSequence[step-1]->getTimeStamp();
            //We need to have recomputed chord top steps here!!!
            if (theSequence[step]->chordTopStep!=-1 || startTimeDifference<=interval)
            {
                if (step>0)
                    theSequence[step-1]->triggers = step;
                theSequence[step]->triggeredBy = step-1;
                theSequence[step]->firstInChain = firstInThisChain;
            }
            else
            {
                if (step>0)
                    theSequence[step-1]->triggers = -1;//The last note in a chain triggers nothing
                theSequence[step]->triggeredBy = -1; //This will be set based on the shortest note near the start of this group, done below
                firstInThisChain = step;
                theSequence[step]->firstInChain = step;
                targetNoteTimes.add(theSequence[step]->getTimeStamp());
    //            std::cout << "firstInChain " << step << " time " << targetNoteTimes.getLast() << "\n";
            }
        }
    }
    if (firstStep!=-1)
        targetNoteTimes.addIfNotAlreadyThere(theSequence[firstStep]->getTimeStamp()); //First must always be a target note
    return stepActivityList;
}

/*=============================================================
 <#loadSequence#>
 */
//Loads the file in fileToLoad which must be set before calling if LoadType is load
void Sequence::loadSequence(LoadType loadFile, Retain retainEdits)
{
    if (retainEdits == doNotRetainEdits)
    {
        targetNoteTimes.clear();
        chainingInterval = 12.0;
        bookmarkTimes.clear();
        setTempoMultiplier(1.0, false);
        if (loadFile==LoadType::loadFile)
        {
            //This is not a LoadType::reAnalyzeOnly so clear everything
            chords.clear();
            allNotes.clear();
        }
    }
    setLoadingFile(true);
    
    if (fileToLoad.getFileName().length() > 0 && loadFile == Sequence::loadFile)
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
//                std::cout << "tempoChange at " << msg.timeStamp << " " << tempo << "\n";
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
//            std::cout << "Time sig change " << msg.timeStamp << " " << numerator << " " << denominator << "\n";
        }
        if (timeSigChanges.size() > 0)
            timeSigChanges[0].getTimeSignatureInfo(numerator, denominator);
        else
            timeSigChanges.add(MidiMessage::timeSignatureMetaEvent(4, 4));

//        timeIncrement = 96.0*tempo*(numerator/denominator)/60000.0;//Per tick.See spreadsheet in source directory. 96 is ppq, always the same
        
        numTracks = midiFile.getNumTracks();
        
        //Track overview
        if (retainEdits==Sequence::doNotRetainEdits)
        {
            trackDetails.clear();
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
            }
        seqDurationInTicks = 96.0*seqDurationInTicks/ppq;
//        std::cout << "seqDurationInTicks " << seqDurationInTicks << "\n";
        if (retainEdits==Sequence::doNotRetainEdits)
        {
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
            
            //Compute start and end measure in each track with any notes
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
                else if (retainEdits==Sequence::doNotRetainEdits && key == "trackDetails")
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
                else if (key == "chordDetails") //Each chord
                {
//                    String propertyStr = String("chordDetails:")+String(chords[i].timeStamp) +" "+String(chords[i].scaleFactor)+
//                    +" "+chords[i].timeSpec+" "+String(chords[i].timeRandScale)+" "+String(chords[i].timeRandSeed)
//                    +" "+chords[i].velSpec+" "+String(chords[i].velRandScale)+" "+String(chords[i].velRandSeed);
                    
                    ChordDetail chDet;
                    StringArray values;
                    values.addTokens(value, " ", "\"");
                    chDet.timeStamp = values[0].getIntValue();
                    chDet.scaleFactor=values[1].getFloatValue();
                    
                    chDet.timeSpec=values[2];
                    chDet.timeRandScale=values[3].getFloatValue();
                    chDet.timeRandSeed=values[4].getIntValue();
                    
                    chDet.velSpec=values[5];
                    chDet.velRandScale=values[6].getFloatValue();
                    chDet.velRandSeed=values[7].getIntValue();
                    chords.push_back(chDet);
                    if (chords.size()<5)
                        std::cout << "Loaded chord "<< chords.size()-1<<" "<< chords.back().timeStamp <<" "<<chords.back().timeSpec << "\n";
                }
                else if (key == "chordNote") //Each note that is a member of a chord
                {
                    StringArray values;
                    values.addTokens(value, " ", "\"");
                    const int chordIndex = values[0].getIntValue();
                    const int offset = values[1].getIntValue();
                    const String noteId = values[2];
                    if (chordIndex<chords.size())
                    {
                        chords[chordIndex].offsets.push_back(offset);
                        chords[chordIndex].noteIds.push_back(noteId);
                    }
                    if (chords.size()<5)
                        std::cout << "Chord note: chordIndex "<<chordIndex<<" offset "<<offset<<" ID "<<noteId<< "\n";
                    //This will give us the chord's note timestamps, but after allNotes is constructed
                    // we will have to scan it for notes with each timeStamp and add pointers to the notes
                    // to the chords's notePointer vector.  Also set each note's chordIndex to refer to this chord.
                    //The notePointer vector should be sorted from high to low note number (it will have been loaded in that order)
//                    String propertyStr = String("chNote:")
//                    +String(chords[i].notePointers[note]->getTimeStamp())+" "+noteSig+" "
//                    +String(i); //Save chord number of this note
                }
                else
                    sequenceProps.setValue(key, value);
            }
        }
        else //midi file
        {
            //Defaults for a new midi file
            sequenceProps.setValue("tempoMultiplier", var(1.0));
            sequenceProps.setValue("chordTimeHumanize", var(0.0));
            sequenceProps.setValue("chordVelocityHumanize", var(1.0));
            sequenceProps.setValue("autoPlaySustains", var(true));
            sequenceProps.setValue("autoPlaySofts", var(true));
            sequenceProps.setValue("reVoiceChords", var(true));
            sequenceProps.setValue("exprVelToOriginalValRatio", var(1.0));
            
            sequenceProps.setValue("horizontalScale", var(1.0));
        }
        //Get values from sequenceProps
        setTempoMultiplier(sequenceProps.getDoubleValue("tempoMultiplier"), false);
        chordTimeHumanize = sequenceProps.getDoubleValue("chordTimeHumanize", var(1.0));
        chordVelocityHumanize = sequenceProps.getDoubleValue("chordVelocityHumanize", var(1.0));
        autoPlaySustains = sequenceProps.getBoolValue("autoPlaySustains", var(true));
        autoPlaySofts = sequenceProps.getBoolValue("autoPlaySofts", var(true));
        reVoiceChords = sequenceProps.getBoolValue("reVoiceChords", var(true));
        exprVelToScoreVelRatio = sequenceProps.getDoubleValue("exprVelToScoreVelRatio", var(1.0));
        
        StringPairArray props = sequenceProps.getAllProperties();
        StringArray keys = props.getAllKeys();
        StringArray vals = props.getAllValues();
//        std::cout <<"Properties loaded: " <<"\n";
//        for (int i=0;i<props.size();i++)
//            std::cout <<" property: " << keys[i] << " / " << vals[i] <<"\n";
        ckfSysex.clear();
        sendChangeMessage(); //Is this needed?
        setChangedFlag (false);
        
        if (allNotes.size()==0) //Assemble allNotes[ ][ ]
        {
            for (int trkNumber=0;trkNumber<numTracks;trkNumber++)
            {
                const MidiMessageSequence *theTrack = midiFile.getTrack(trkNumber);
                const int numEvents = theTrack->getNumEvents();
                allNotes.push_back(std::vector<std::shared_ptr<NoteWithOffTime>>());
                int noteCount=0;
                for (int i=0;i<numEvents;i++)
                {
                    if (theTrack->getEventPointer(i)->message.isNoteOn())
                    {
                        std::shared_ptr<NoteWithOffTime> msg (new NoteWithOffTime);
                        msg->track = trkNumber;
                        msg->setTimeStamp(theTrack->getEventPointer(i)->message.getTimeStamp());
                        msg->channel = theTrack->getEventPointer(i)->message.getChannel();
                        msg->noteNumber = theTrack->getEventPointer(i)->message.getNoteNumber();
                        msg->velocity = theTrack->getEventPointer(i)->message.getFloatVelocity();
                        msg->offTime = theTrack->getTimeOfMatchingKeyUp(i);
                        
                        if (msg->offTime <= msg->getTimeStamp()) //In a correct sequence this should not happen
                            msg->offTime = msg->getTimeStamp()+50; //But if it does, turn it into a short note  with non neg duration
                        const double ts = msg->getTimeStamp();
                        msg->setTimeStamp(96.0*ts/ppq);
                        msg->originalVelocity = msg->velocity;
                        if (trkNumber==2 && noteCount==0)
                            std::cout<< "set Velocity " <<msg->velocity <<"\n";
                        const double ot = 96.0*msg->offTime/ppq;
                        msg->offTime = ot;
                        msg->indexInTrack = theTrack->getIndexOf(theTrack->getEventPointer(i));
                        allNotes[trkNumber].push_back(msg);
                        noteCount++;
                    }
                }
            }
        }
        theControllers.clear();
        for (int trkNumber=0;trkNumber<allNotes.size();trkNumber++)
        {
            //Controllers
            const MidiMessageSequence *theTrack = midiFile.getTrack(trkNumber);
            if (theTrack!=NULL)
            {
                for (int i=0;i<theTrack->getNumEvents();i++)
                {
                    if (theTrack->getEventPointer(i)->message.isController())
                    {
                        ControllerMessage ctrMsg(trkNumber, theTrack->getEventPointer(i)->message);
                        theControllers.push_back(ctrMsg);
                        //                    std::cout << "Add Controller: timeStamp " << ctrMsg.getTimeStamp()
                        //                    << " Track " << trkNumber
                        //                    << " Channel " << ctrMsg.getChannel()
                        //                    << " cc " << ctrMsg.getControllerNumber()
                        //                    << " cc " << ctrMsg.getControllerName(ctrMsg.getControllerNumber())
                        //                    << " Value " << ctrMsg.getControllerValue()
                        //                    <<"\n";
                    }
                }
            }
        }
    }
    
    //End of reloading file ####################################################################################
//    compareAllNotes("End of reloading file");
    theSequence.clear();
    
//      Transfer tracks to "theSequence"
    for (int trkNumber=0;trkNumber<allNotes.size();trkNumber++)
    {
        const int numEvents = allNotes[trkNumber].size();
//        std::cout
//        << "Loading track "<< trkNumber;
        //Notes
        if (isActiveTrack(trkNumber))
        {
//            std::cout
//            << ", noteOns "<< trackDetails[trkNumber].nNotes
//            << "\n";
            for (int i=0;i<numEvents;i++)
            {
                theSequence.push_back(allNotes[trkNumber][i]);
                //                std::cout << "msg.pRecordsWithEdits " <<   stepNum << " "<<
//                " "<< theSequence[stepNum].pRecordsWithEdits->size() <<"\n";;
//
//                std::cout <<theSequence.size()<< " Add note: Track, TimeStamp, NN " << msg.track  << ", "
//                << msg.timeStamp<< " " << msg.getNoteNumber() <<"\n";
            }
        }
    }
    
    if (theSequence.size()>0)
    {
        struct {
            bool operator()(std::shared_ptr<NoteWithOffTime> a, std::shared_ptr<NoteWithOffTime> b) const
            {
                if (a->getTimeStamp()==b->getTimeStamp())
                    return a->noteNumber > b->noteNumber;
                else
                    return a->getTimeStamp() < b->getTimeStamp();
            }
        } customCompare;
        std::sort(theSequence.begin(), theSequence.end(),customCompare);
        std::sort(theControllers.begin(), theControllers.end());
        //Remove exact duplicate notes
        std::shared_ptr<NoteWithOffTime> pPrevMsg = NULL;
        for (int step=0; step<theSequence.size();step++)
        {
            if (pPrevMsg!=NULL && pPrevMsg->channel==theSequence[step]->channel &&
                    pPrevMsg->noteNumber==theSequence[step]->noteNumber &&
                    pPrevMsg->getTimeStamp()==theSequence[step]->getTimeStamp())
                theSequence.erase(theSequence.begin()+step);
            else
                pPrevMsg = theSequence[step];
        }
        for (int step=0;step<theSequence.size();step++)
        {
            theSequence[step]->currentStep = step;
        }
        seqDurationInTicks = theSequence.back()->getTimeStamp(); //We update this here so that it reflects currently active tracks

        //###
        //Build the chords list if we either loaded a midi file or loaded a ckf file (and probably read a chords list)
        //Issue - What if sequence does not include all tracks?
        if (loadFile==Sequence::loadFile)
        {
            if (loadedCkfFile==true && chords.size()>0)  //It Was a ckf file so finish creating the chords list loaded from the file
            {
                //For each chord in the chords Array, and for each note in its chordNotes list,
                //find notes with that timeStamp in theSequence
                //Set each found note's chordIndex to refer to this chord
                //And give all non chord notes a different negative value for chordIndex
                //We assume theSequence has been created and chords has been created or loaded from the file.  Both must be sorted by ascending timeStamp.
                int negativeInt = -1;
                for (int step=0;step<theSequence.size();step++)
                {
                    theSequence[step]->chordIndex = negativeInt--;
                }
                int chStartStep=0;
                for (int chIndex=0;chIndex<chords.size();chIndex++)
                {
                    while (chStartStep<theSequence.size() && theSequence[chStartStep]->getTimeStamp()!=chords[chIndex].timeStamp)
                        chStartStep++;
                    //We are now at the start of the next chord
                    for (int ntIndex=0;ntIndex<chords[chIndex].offsets.size();ntIndex++)
                    {
                        int step;
                        for (step=chStartStep;step<theSequence.size();step++)
                        {
                            String noteId = String(theSequence[step]->track)
                            +"_"+String(theSequence[step]->channel)
                            +"_"+String(theSequence[step]->noteNumber);
                            if (chords[chIndex].noteIds[ntIndex]==noteId)
                                break;
                        }
                        if (step<theSequence.size())
                        {
                            const double noteTimeStamp = chords[chIndex].timeStamp+chords[chIndex].offsets[ntIndex];
                            theSequence[step]->setTimeStamp(noteTimeStamp);
                            chords[chIndex].notePointers.push_back(theSequence[step]);
                            theSequence[step]->chordIndex = chIndex;
                            theSequence[step]->noteIndexInChord = ntIndex;
                        }
                    }
                }
            }
            else //It Was a midi file so create a new chords list
            {
                chords.clear();
                double thisChordTimeStamp;
                int uniqueNonChordIndicator = -1;
                for (int step=0; step<theSequence.size();step++)
                {
                    thisChordTimeStamp = theSequence[step]->getTimeStamp();
    //                std::vector<std::shared_ptr<NoteWithOffTime>> tempChordNotes;
                    ChordDetail detail;
                    while (step<theSequence.size() && theSequence[step]->getTimeStamp() == thisChordTimeStamp)
                    {
                        //                    nt.indexOfChordDetail = chords.size();
                        //                    nt.indexOfChordDetail = -1; //Mark as not in a chord for now
                        detail.notePointers.push_back(theSequence[step]);
                        step++;
                    }
                    step--;
                    
                    for (int j=0;j<detail.notePointers.size();j++)
                    {
                        detail.notePointers[j]->noteIndexInChord = j; //Tell this note it's current index in the chord
                        //We mark every non chord note with a unique negative integer and every chord note with the index in chords[ ]  of its chord.
                        if (detail.notePointers.size()==1) //One note, so not a chord
                            detail.notePointers[j]->chordIndex = uniqueNonChordIndicator--; //A negative id different for each non chord note
                        else
                        {
                            detail.notePointers[j]->chordIndex = chords.size();
                        }
                        //                    tempChordNotes.setUnchecked(j, tempChordNotes[j]);
                        
                        //                    std::cout <<"Next original note: index " << originalNotes.size()-1
                        //                    << " key " << key
                        //                    << " timeStamp " <<tempChordNotes[j].timeStamp
                        //                    << " nChordNotes "<<tempChordNotes.size()
                        //                    << " indexOfChordDetail " << tempChordNotes[j].indexOfChordDetail
                        //                    <<"\n";
                        //                    originalNoteIndex[key] = originalNotes.size()-1;
                    }
                    
                    if (detail.notePointers.size()>1)
                    {
                        detail.timeStamp = detail.notePointers[0]->getTimeStamp();
                        chords.push_back(detail);
//                        std::cout <<"Next chord: chordNum "<<chords.size()-1
//                        <<" timeStamp " <<detail.timeStamp
//                        << " nNotes "<< detail.notePointers.size();
//                        if (detail.notePointers.size()>0)
//                            std::cout << " firstIndex " << detail.notePointers[0]->getTimeStamp();
//                        std::cout <<"\n";
                    }
                }
            }
        }
        
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
//                    std::cout << ctrMsg.timeStamp << " sustainPedalChange "<<ctrMsg.getControllerValue()<<"\n";
                    sustainPedalChanges.push_back(ctrMsg);
//                    prevTimeStamp = ctrMsg.timeStamp;
//                    std::cout << "i, Time " << i << ", " << 96.0*ctrMsg.timeStamp/ppq
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
                    
//                     std::cout << "i, Time " << i << ", " << ctrMsg.timeStamp
//                     //            << " Channel " << ctrMsg.getChannel()
//                     //            << " cc " << ctrMsg.getControllerNumber()
//                     //            << " Value " << ctrMsg.getControllerValue()
//                     << " " << softPedalDirection
//                     << " Type:" << ctrMsg.getDescription()
//                     <<"\n";
                }
            }
        }
        
        //###Humanize chord note start times and velocities
        double thisChordTimeStamp;
        std::vector<std::shared_ptr<NoteWithOffTime>> chord;
        int chordTopStep;
        for (int step=0; step<theSequence.size();step++)
        {
            //All sequential notes with the same chordIndex are assumed to be part of this chord (if any)
            int thisStepChordNoteIndex = theSequence[step]->chordIndex;
            int nextStepChordNoteIndex;
            if (step+1<theSequence.size())
            {
                nextStepChordNoteIndex = theSequence[step+1]->chordIndex;
            }
            else
                nextStepChordNoteIndex = INT32_MAX;
        
            chord.push_back(theSequence[step]);
            if (chord.size()==1)  //The first step is always the chord top
                chordTopStep=step;
            if (thisStepChordNoteIndex != nextStepChordNoteIndex)
            {
                if (chord.size()>1)
                {
                    struct {
                        bool operator()(std::shared_ptr<NoteWithOffTime> a, std::shared_ptr<NoteWithOffTime> b) const
                        {
                            return a->noteNumber > b->noteNumber;
                        }
                    } customCompare2;
                    std::sort(chord.begin(), chord.end(),customCompare2);
                    
//                    std::cout <<"Found chord " <<theSequence[step].timeStamp<<" "<< chord.size() <<"\n";
                    thisChordTimeStamp = theSequence[chordTopStep]->getTimeStamp();
                    //Rand seed based on thisStepChordNoteIndex different for all chords bot constant for a chord
                    srand(thisStepChordNoteIndex);
                    
                    double timeToNextNote;
                    if (step<theSequence.size()-1)
                        timeToNextNote = theSequence[step+1]->getTimeStamp()-thisChordTimeStamp;
                    else
                        timeToNextNote = DBL_MAX;
                    double localTimeFuzz = std::min(timeToNextNote*0.33,chordTimeHumanize);
                    
                    for (int i=1; i<chord.size(); i++)
                    {
                        chord[i]->chordTopStep = chordTopStep;
                        const int temp = localTimeFuzz*100;
                        double randAdd;
                        unsigned r = rand();
                        if (temp==0)
                            randAdd = 0;
                        else
                            randAdd = r%temp/100.0;
                        chord[i]->setTimeStamp(thisChordTimeStamp+randAdd);
                    }
                    if (reVoiceChords)
                    {
                        const float topNoteVel = chord[0]->originalVelocity;
                        const float userEditFactor = chord[0]->velocity/topNoteVel;
                        if (chord.size()==2)
                        {
                            const float originalVel = chord[1]->originalVelocity;
                            const float ckVel = 0.7f * topNoteVel;
                            const float proRatedVel = ckVel * chordVelocityHumanize + originalVel * (1.0f - chordVelocityHumanize);
                            chord[1]->setVelocity(proRatedVel*userEditFactor);
                        }
                        else // (chord.size()>2)
                        {
                            for (int j=1;j<chord.size()-1;j++)
                            {
                                const float originalVel = chord[j]->originalVelocity;
                                const float ckVel = 0.6f * topNoteVel;
                                const float proRatedVel = ckVel * chordVelocityHumanize + originalVel * (1.0f - chordVelocityHumanize);
                                chord[j]->setVelocity(proRatedVel*userEditFactor);
                            }
//                            int step =  chord[chord.size()-1];
                            const float originalVel = chord[chord.size()-1]->originalVelocity;
                            const float ckVel = 0.8f * topNoteVel;
                            const float proRatedVel = ckVel * chordVelocityHumanize + originalVel * (1.0f - chordVelocityHumanize);
                            chord[chord.size()-1]->setVelocity(proRatedVel*userEditFactor);
                        }
                    }
                    chord[0]->chordTopStep=-1;
                }
                else
                    theSequence[step]->chordTopStep=-1;
                chord.clear();
            }
            else
                ;//Do nothing
        }
        
//        for (int i=0;i<chords.size();i++)
//            std::cout << "Chord "<<i<<" "<<chords[i].timeStamp<<" "<<chords[i].nNotes<<"\n";
        std::sort(theSequence.begin(), theSequence.end(), customCompare);
        for (int step=0;step<theSequence.size();step++)
            theSequence[step]->currentStep = step;
        //        //This reconstructs the chains using targetNoteTimes loaded from the ck file or constructed by chain command
        if(targetNoteTimes.size()>0) //If loaded from ck file or previously created for midi file by chain ()
        {
            int firstInThisChain = 0;
            double prevTS = -1.0;
//            std::cout<< "Chain in loadSequence " << "\n";
            for (int step=0; step<theSequence.size();step++)
            {
//                if (step<14)
//                    std::cout<< step
//                    << " ts " << theSequence[step].timeStamp
//                    << " firstInChain " << targetNoteTimes.contains(theSequence[step].timeStamp) << "\n";
                if (prevTS != theSequence[step]->getTimeStamp() && targetNoteTimes.contains(theSequence[step]->getTimeStamp()))
                {
                    if (step>0)
                        theSequence[step-1]->triggers = -1;//Last note in chain triggers nothing
                    theSequence[step]->triggeredBy = -1; //This will be set based on the shortest note near the start of this group, done below
                    firstInThisChain = step;
                    theSequence[step]->firstInChain = step;
                }
                else
                {
                    if (step>0)
                    {
                        theSequence[step-1]->triggers = step;
                        theSequence[step]->triggeredBy = step-1;
                    }
                    else
                        theSequence[step]->triggeredBy = step-1;
                    theSequence[step]->firstInChain = firstInThisChain;
                }
                prevTS = theSequence[step]->getTimeStamp();
            }
        }
        else
        {
            chain(Array<int>(),chainingInterval); //This is used only for when a plain midi file is loaded
        }
    
        
        //Find chainTriggers and set this property for all steps in a given chain
        assert (theSequence.size()>0);
        int step = 0;
        int chainTrigger = 0;
        while (step < theSequence.size())
        {
            //If this step is a firstInChain scan all notes with time stamp equal to that of the firstInChain for the shortest note
            if (theSequence[step]->triggeredBy==-1 && theSequence[step]->chordTopStep==-1)
            {
                chainTrigger = step;
//                int highestNote = -1;
                int stepOfhighestNote = step;
                int subStep = step;
                while (subStep<theSequence.size()-1 && theSequence[subStep]->getTimeStamp()==theSequence[theSequence[step]->firstInChain]->getTimeStamp())
                {
                    if (theSequence[subStep]->noteNumber >= theSequence[stepOfhighestNote]->noteNumber)
                    {
                        stepOfhighestNote = subStep;
                    }
                    subStep++;
                }
                chainTrigger = stepOfhighestNote;
            }
            assert(0<=chainTrigger && chainTrigger<theSequence.size());
            theSequence[step]->chainTrigger = chainTrigger;
            step++;
        }

        //Store highest velocity in each chain in every step of the chain
        for (int step=0;step<theSequence.size();step++)
        {
            int firstInChain = step;
            float highestVelocity = -1;
            int firstStep = step;
            bool enteredLoop = false;
            while (step<theSequence.size() && theSequence[step]->firstInChain==firstInChain)
            {
                enteredLoop = true;
                if (theSequence[step]->velocity > highestVelocity)
                    highestVelocity = theSequence[step]->velocity;
                step++;
            }
            if (enteredLoop)
                step--;
            
            //Store highest velocity in this chain in every step of this chain
            for (int i=firstStep; i<=step; i++ )
            {
                theSequence[i]->highestVelocityInChain = highestVelocity;
            }
        }
        
        //Determine which steps are triggeredNotes and triggeredOffNotes
        //triggeredNotes are steps that start no later than the triggeredNoteLimit from the chainTrigger.
        //triggeredOffNotes are triggeredNotes that end before the END of the next chainTrigger note.
        for (int step=0; step<theSequence.size();step++)
        {
            if ( theSequence[step]->getTimeStamp() <= theSequence[theSequence[step]->chainTrigger]->getTimeStamp()+triggeredNoteLimit)
            {
                theSequence[step]->triggeredNote=true;
                //Scan for nextFirstInChain
                int nextFirstInChain = theSequence[step]->firstInChain+1;
                while (nextFirstInChain<theSequence.size()-1 && theSequence[nextFirstInChain]->triggeredBy != -1)
                    nextFirstInChain++;
                
                //Two cases for a step to be a triggeredOffNote:
                //1) Where a the step ends before the next chain trigger
                //2) Where the step's offtime is no more than the start of any triggered note of the chain that starts after it)
                if (nextFirstInChain<theSequence.size()-1
                    && theSequence[step]->triggeredNote && theSequence[step]->offTime < theSequence[nextFirstInChain]->getTimeStamp())
                    theSequence[step]->triggeredOffNote = true;
                else
                    theSequence[step]->triggeredOffNote = false;
                
                //Scan for case (2) above
                int i = nextFirstInChain;
                bool shouldSustain = false;
                while (i<theSequence.size() && theSequence[i]->getTimeStamp()<= theSequence[nextFirstInChain]->getTimeStamp()+triggeredNoteLimit) //All triggered notes in next chain
                {
                    if (theSequence[step]->offTime>theSequence[i]->getTimeStamp())
                    {
                        shouldSustain = true;
                        break;
                    }
                    i++;
                }
                theSequence[step]->triggeredOffNote = !shouldSustain;
                theSequence[step]->autoplayedNote = false;
            }
            else
            {
                theSequence[step]->autoplayedNote = true;
            }
        }
    }
    setLoadingFile(false);
//    dumpData(0, 20, -1);
    //We assume that rewind will always be called after loadSequence, and that rewind calls sendChangeMessage
//    compareAllNotes("End of loadSequence");
} //End of loadSequence

SortedSet<int> Sequence::getNotesUsed(int &minNote, int &maxNote)
{
    SortedSet<int> used;
    minNote = 127;
    maxNote = 0;
    for (int noteIndex=0;noteIndex<theSequence.size();noteIndex++)
    {
        const int note = theSequence.at(noteIndex)->noteNumber;
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

void Sequence::dumpData(Array<int> sel)
{
    if (sel.size()>0)
        dumpData(sel.getFirst(),sel.getLast(),-1);
    else
        dumpData(-1,0,-1);
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
    << " vel "
    << " timeStamp "
    << " offTime "
    << " firstInChain "
    << " triggers "
    << " triggeredBy "
    << " chainTrigger "
    << " triggered "
    << " triggeredOff "
    << " chordTop "
    << "\n";
    if (end>theSequence.size())
        end = theSequence.size();
    for (int i=start; i<theSequence.size() && i<=end;i++)
    {
        if (nn==-1 || theSequence[i]->noteNumber==nn )
        {
            String note = MidiMessage::getMidiNoteName (theSequence[i]->noteNumber, true, true, 3);
            std::cout
            << i <<" "
            << (int)theSequence[i]->track <<" "
            << theSequence[i]->channel <<" "
            << theSequence[i]->noteNumber <<" "
            << note<<" "
            << std::round(theSequence[i]->velocity*127.0) <<" "
            << theSequence[i]->getTimeStamp()<<" "
            << theSequence[i]->offTime<<" "
            << theSequence[i]->firstInChain<<" "
            << theSequence[i]->triggers<<" "
            << theSequence[i]->triggeredBy<<" "
            << theSequence[i]->chainTrigger<<" "
            << theSequence[i]->triggeredNote<<" "
            << theSequence[i]->triggeredOffNote<<" "
            << theSequence[i]->chordTopStep<<" "
            <<"\n";
        }
    }
}

//std::vector<NoteWithOffTime*> Sequence::getNotesInTimeRange(double minTimeStamp,
//                                                                double maxTimeStamp)
//{
//    std::vector<NoteWithOffTime*> notes;
//    notes.clear();
//    for (int noteIndex=0;noteIndex<theSequence.size();noteIndex++)
//    {
//        NoteWithOffTime *msg = theSequence.at(noteIndex);
//        if (msg->getOffTime()>=minTimeStamp && msg->getTimeStamp()<=maxTimeStamp)
//            notes.push_back(msg);
//    }
//    return notes;
//}

Array<MidiMessage> Sequence::getCurrentTimeSig(double timeStamp)
{
    return Array<MidiMessage>();
}
