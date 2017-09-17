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
    
    targetNoteTimes.clear();
    for (int step = 0;step<theSequence.size();step++)
        if (theSequence.at(step)->targetNote)
            targetNoteTimes.add(theSequence.at(step)->getTimeStamp());
    
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
//        scaleFactor = 1.0f;
//        timeSpec = "TimeSpec"; timeRandScale=1.0f; timeRandSeed=1;
//        velSpec = "VelSpec"; velRandScale=1.0f; velRandSeed=1;
        //Property "chordDetails"
        String propertyStr = String("chordDetails:")+String(chords.at(chIndex).chordTimeStamp) +" "+String(chords.at(chIndex).scaleFactor)+
        +" "+chords.at(chIndex).timeSpec+" "+String(chords.at(chIndex).timeRandScale)+" "+String(chords.at(chIndex).timeRandSeed)
        +" "+chords.at(chIndex).velSpec+" "+String(chords.at(chIndex).velRandScale)+" "+String(chords.at(chIndex).velRandSeed);
        int len = propertyStr.length();
        char buffer[128];
        propertyStr.copyToUTF8(buffer,128);
        MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
        sysexSeq.addEvent(sysex);
        //Property "chNote" : The notes in this chord
        for (int note=0;note<chords.at(chIndex).notePointers.size();note++)
        {
            //A unique signature for the note at this exact timeStamp
            String noteId = String(chords.at(chIndex).notePointers.at(note)->track)
                                            +"_"+String(chords.at(chIndex).notePointers.at(note)->channel)
                                            +"_"+String(chords.at(chIndex).notePointers.at(note)->noteNumber);
            const int offset = chords.at(chIndex).notePointers.at(note)->getTimeStamp() - chords.at(chIndex).chordTimeStamp;
            
            String propertyStr = String("chordNote:")+String(chIndex)+" "+String(offset)+" "+noteId;
            int len = propertyStr.length();
            char buffer[128];
            propertyStr.copyToUTF8(buffer,128);
            MidiMessage sysex = MidiMessage::createSysExMessage(buffer, len+1);
//            if (chIndex<5)
//                std::cout << " Write chord note - "<<propertyStr <<" "<<propertyStr.length() << "\n";
            sysexSeq.addEvent(sysex);
        }
    }
    //    std::cout << " Number of sysex records written - "<< seq->getNumEvents()<< "\n";

    int tracksToCopy = midiFile.getNumTracks();
    if (loadedCkfFile)
        tracksToCopy--;
    
    MidiFile outputFile;
    short timeFormat = 96;//midiFile.getTimeFormat();
    int firstTrkWithPedals = -1;
    int firstTrkWithNotes = -1;
    for (int trk=0;trk<trackDetails.size();trk++ )
    {
        if (firstTrkWithPedals==-1 && (trackDetails[trk].nSustains>0 || trackDetails[trk].nSofts>0))
            firstTrkWithPedals=trk;
        if (firstTrkWithNotes==-1 && trackDetails[trk].nNotes>0)
            firstTrkWithNotes=trk;
    }
    if (firstTrkWithPedals==-1)
        firstTrkWithPedals = firstTrkWithNotes;
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
                if (!msg.isController())
                    trackSeq.addEvent(msg);
                else if (msg.getControllerNumber()!=64 && msg.getControllerNumber()!=67) //Don't save original pedal messages
                    trackSeq.addEvent(msg);
            }
        }
        if (trk==firstTrkWithPedals)
        {
            int channel = trackDetails[firstTrkWithPedals].originalChannel;
            if (channel==-1)
                channel = trackDetails[firstTrkWithNotes].originalChannel;
//            if (channel==-1) channel =
            for (int i=0;i<sustainPedalChanges.size();i++)
            {
                MidiMessage msg=MidiMessage::controllerEvent(channel, 64, sustainPedalChanges.at(i).pedalOn?127:0);
                msg.setTimeStamp(sustainPedalChanges.at(i).timeStamp);
                trackSeq.addEvent(msg);
            }
            for (int i=0;i<softPedalChanges.size();i++)
            {
                MidiMessage msg=MidiMessage::controllerEvent(channel, 67, softPedalChanges.at(i).pedalOn?127:0);
                msg.setTimeStamp(softPedalChanges.at(i).timeStamp);
                trackSeq.addEvent(msg);
            }
        }
        for (int step=0;step<allNotes.at(trk).size();step++)
        {
            MidiMessage onMsg = MidiMessage::noteOn(allNotes.at(trk).at(step)->channel,
                                                  allNotes.at(trk).at(step)->noteNumber,
                                                  allNotes.at(trk).at(step)->velocity);
            if (allNotes.at(trk).at(step)->inChord) //If in a chord
                onMsg.setTimeStamp(chords.at(allNotes.at(trk).at(step)->chordIndex).chordTimeStamp); //Write the chords timestamp for all its notes
            else
                onMsg.setTimeStamp(allNotes.at(trk).at(step)->getTimeStamp()); //Otherwise write the actual note's timestamp
            trackSeq.addEvent(onMsg);
            MidiMessage offMsg = MidiMessage::noteOff(allNotes.at(trk).at(step)->channel,
                                                  allNotes.at(trk).at(step)->noteNumber,
                                                  0.0f);
            offMsg.setTimeStamp(allNotes.at(trk).at(step)->getOffTime());
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
    
    //For steps in selection, construct stepActivityList
    //Entries in stepActivityList are {int step; bool active}
    Array<Sequence::StepActivity> stepActivityList;
    if (!getLoadingFile())
    {
        for (int step=0;step<theSequence.size();step++)
        {
            if (!selection.contains(step))
                continue;
                if (theSequence.at(step)->targetNote)
                {
//                    theSequence.at(step)->targetNote = false;
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

    theSequence.at(0)->firstInChain = 0;
    theSequence.at(0)->triggeredBy = -1;
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
                startTimeDifference = theSequence.at(step)->getTimeStamp()-theSequence.at(step-1)->getTimeStamp();
            //We need to have recomputed chord top steps here!!!
            if (/*theSequence.at(step)->chordTopStep!=-1 || */startTimeDifference<=interval)
            {
                if (step>0)
                    theSequence.at(step-1)->triggers = step;
                theSequence.at(step)->triggeredBy = step-1;
                theSequence.at(step)->firstInChain = firstInThisChain;
                theSequence.at(step)->targetNote = false;
            }
            else
            {
                if (step>0)
                    theSequence.at(step-1)->triggers = -1;//The last note in a chain triggers nothing
                theSequence.at(step)->triggeredBy = -1; //This will be set based on the shortest note near the start of this group, done below
                firstInThisChain = step;
                theSequence.at(step)->firstInChain = step;
                theSequence.at(step)->targetNote = true;
            }
        }
    }
    if (firstStep!=-1)
        theSequence.at(firstStep)->targetNote = true; //First must always be a target note
    return stepActivityList;
}

/*=============================================================
 <#loadSequence#>
 */
//Loads the file in fileToLoad which must be set before calling if LoadType is load
bool Sequence::loadSequence(LoadType loadFile, Retain retainEdits)
{
//    std::cout << "entering loadSequence \n";
    targetNoteTimes.clear();
  try {
        
    if (retainEdits == doNotRetainEdits)
    {
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
//        std::cout << "entering loadSequence: load file \n";
        if (!fileToLoad.exists()) {
            std::cout << "File " << fileToLoad.getFullPathName() << " does not exist.\n";
            return false;
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
                        {
//                            std::cout << "Sustain on" << trk << "\n";
                            nSustains++;
                        }
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
        alreadyChained = false;
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
                        alreadyChained = true;
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
                    int playability = values[1].getIntValue();
//                    std::cout << "track " << track <<" playability " << playability <<" "<<trackDetails[track].nNotes<<"\n";
                    if (playability==TrackTypes::Track_Autoplay)
                        playability = TrackTypes::Track_Play;
                    const int assignedChannel = values[2].getIntValue();
                    TrackDetail trkDet = trackDetails[track];
                    trkDet.playability = playability;
                    trkDet.assignedChannel = assignedChannel;
                    trackDetails.set(track, trkDet);
//                    std::cout << "loadedTrack " <<track <<" playability "<<playability <<" assignedChannel "<<assignedChannel <<"\n";
                }
                else if (key == "tnt") //target Note Times
                {
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
                    chDet.chordTimeStamp = values[0].getIntValue();
                    chDet.scaleFactor=values[1].getFloatValue();
                    chDet.timeSpec = values[2];
                    if (chDet.timeSpec=="random" || chDet.timeSpec=="arp") //Ignore obsolete property names
                        chDet.timeSpec = "manual";
                    chDet.timeRandScale=values[3].getFloatValue();
                    chDet.timeRandSeed=values[4].getIntValue();
                    chDet.velSpec=values[5];
                    chDet.velRandScale=values[6].getFloatValue();
                    chDet.velRandSeed=values[7].getIntValue();
                    chords.push_back(chDet);
//                    if (chords.size()<5)
//                        std::cout << "Loaded chord "<< chords.size()-1<<" "<< chords.back().timeStamp <<" "<<chords.back().timeSpec << "\n";
                }
                else if (key == "chordNote") //Each note that is a member of a chord
                {
                    StringArray values;
                    values.addTokens(value, " ", "\"");
                    const int chordIndex = values[0].getIntValue();
                    int offset = values[1].getIntValue();
                    const String noteId = values[2];
                    if (chordIndex<chords.size())
                    {
                        int foo;
                        if (offset>200000 || offset<-200000)
                        {
                            foo=0;
                            offset = 0;
                        }
                        chords.at(chordIndex).offsets.push_back(offset);
                        chords.at(chordIndex).noteIds.push_back(noteId);
                    }
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
            sequenceProps.setValue("exprVelToOriginalValRatio", var(1.0));
            
            sequenceProps.setValue("horizontalScale", var(1.0));
        }
        //Get values from sequenceProps
        setTempoMultiplier(sequenceProps.getDoubleValue("tempoMultiplier"), false);
        chordTimeHumanize = sequenceProps.getValue("chordTimeHumanize", var("1.0"));
        chordVelocityHumanize = sequenceProps.getValue("chordVelocityHumanize", var("68"));
        autoPlaySustains = sequenceProps.getBoolValue("autoPlaySustains", var(true));
        autoPlaySofts = sequenceProps.getBoolValue("autoPlaySofts", var(true));
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
                        msg->setOfftime(theTrack->getTimeOfMatchingKeyUp(i));
                        
                        if (msg->getOffTime() <= msg->getTimeStamp()) //In a correct sequence this should not happen
                            msg->setOfftime(msg->getTimeStamp()+50); //But if it does, make a short note with non neg duration
                        const double ts = msg->getTimeStamp();
                        msg->setTimeStamp(96.0*ts/ppq);
                        msg->originalVelocity = msg->velocity;
                        const double ot = 96.0*msg->getOffTime()/ppq;
                        msg->setOfftime(ot);
                        msg->indexInTrack = theTrack->getIndexOf(theTrack->getEventPointer(i));
                        allNotes.at(trkNumber).push_back(msg);
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
                    }
                }
            }
        }
        //Extract pedal changes
        sustainPedalChanges.clear();
        softPedalChanges.clear();
        bool sustainOn;
        bool softOn;
        int trackWithSustains = -1;
        int maxSustainsInATrack = -1;
        for (int trk=0;trk<trackDetails.size();trk++)
        {
            if (trackDetails[trk].nSustains>maxSustainsInATrack)
            {
                maxSustainsInATrack = trackDetails[trk].nSustains;
                trackWithSustains = trk;
            }
        }
        int trackWithSofts = -1;
        int maxSoftsInATrack = -1;
        for (int trk=0;trk<trackDetails.size();trk++)
        {
            if (trackDetails[trk].nSofts>maxSoftsInATrack)
            {
                maxSoftsInATrack = trackDetails[trk].nSofts;
                trackWithSofts = trk;
            }
        }
        for (int i=0;i<theControllers.size();i++)
        {
            ControllerMessage ctrMsg = theControllers[i];
            ctrMsg.setTimeStamp(96.0*ctrMsg.getTimeStamp()/ppq);
            if (ctrMsg.track==trackWithSustains && ctrMsg.getControllerNumber()==64)   //Sustain pedal
            {
//                std::cout<< ctrMsg.getTimeStamp()<<" sust Track " << ctrMsg.track<<" "<<ctrMsg.getControllerValue()<<"\n";
                if ((ctrMsg.isSustainPedalOn() && !sustainOn)||trackWithSustains==-1)
                {
                    Sequence::PedalMessage pedalMsg = Sequence::PedalMessage(ctrMsg.getTimeStamp(),true);
                    sustainPedalChanges.push_back(pedalMsg);
                    sustainOn = true;
                }
                else if (ctrMsg.isSustainPedalOff() && sustainOn)
                {
                    Sequence::PedalMessage pedalMsg = Sequence::PedalMessage(ctrMsg.getTimeStamp(),false);
                    sustainPedalChanges.push_back(pedalMsg);
                    sustainOn = false;
                }
            }
            else if (ctrMsg.track==trackWithSofts && (ctrMsg.isSoftPedalOn() || ctrMsg.isSoftPedalOff()))  //Soft pedal
            {
                if ((ctrMsg.isSoftPedalOn() && !softOn))
                {
                    Sequence::PedalMessage pedalMsg = Sequence::PedalMessage(ctrMsg.getTimeStamp(),true);
                    softPedalChanges.push_back(pedalMsg);
                    softOn = true;
                }
                else if ((ctrMsg.isSoftPedalOff() && softOn))
                {
                    Sequence::PedalMessage pedalMsg = Sequence::PedalMessage(ctrMsg.getTimeStamp(),false);
                    softPedalChanges.push_back(pedalMsg);
                    softOn = false;
                }
            }
        }
    }
  } catch (const std::out_of_range& ex) {
      std::cout << " error loadSequence: reloading file " << "\n";
  }
    
    //End of reloading file ###
  try {
    theSequence.clear();
    
//      Transfer tracks to "theSequence"
//        std::cout << "Transfer tracks to theSequence \n";
    for (int trkNumber=0;trkNumber<allNotes.size();trkNumber++)
    {
        const int numEvents = allNotes.at(trkNumber).size();
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
  } catch (const std::out_of_range& ex) {
      std::cout << " error loadSequence: building theSequence " << "\n";
  }
    
    if (theSequence.size()>0)
    {
      try {
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
            if (pPrevMsg!=NULL && pPrevMsg->channel==theSequence.at(step)->channel &&
                    pPrevMsg->noteNumber==theSequence.at(step)->noteNumber &&
                    pPrevMsg->getTimeStamp()==theSequence.at(step)->getTimeStamp())
                theSequence.erase(theSequence.begin()+step);
            else
                pPrevMsg = theSequence[step];
        }
        seqDurationInTicks = 0.0;
        for (int step=0;step<theSequence.size();step++)
        {
            theSequence.at(step)->currentStep = step;
            if (theSequence.at(step)->getOffTime() > seqDurationInTicks)
                seqDurationInTicks = theSequence.at(step)->getOffTime();
        }
      } catch (const std::out_of_range& ex) {
          std::cout << " error loadSequence: before chord processing " << "\n";
      }
        
        //seqDurationInTicks = theSequence.back()->getTimeStamp(); //We update this here so that it reflects currently active tracks

        //###
        //Build the chords list if we either loaded a midi file or loaded a ckf file (and probably read a chords list)
        //Issue - What if sequence does not include all tracks?
//        std::cout << "Build the chords list \n";
      try {
        if (loadFile==Sequence::loadFile || loadFile==Sequence::updateChords)
        {
            if (loadFile!=Sequence::updateChords && loadedCkfFile==true &&
                chords.size()>0)  //It Was a ckf file so finish creating the chords list loaded from the file
            {
                //For each chord in the chords Array, and for each note in its chordNotes list,
                //find notes with that timeStamp in theSequence
                //Set each found note's chordIndex to refer to this chord
                //Set each chord note's inChord to true
                //We assume theSequence has been created and chords has been created or loaded from the file.  Both must be sorted by ascending timeStamp.
                for (int step=0;step<theSequence.size();step++)
                {
                    theSequence.at(step)->inChord = false;
                }
                int chStartStep=0;
                for (int chIndex=0;chIndex<chords.size();chIndex++)
                {
                    while (chStartStep<theSequence.size() &&
                           theSequence.at(chStartStep)->getTimeStamp()!=chords.at(chIndex).chordTimeStamp)
                        chStartStep++;
                    //We are now at the start of the next chord
                    for (int ntIndex=0;ntIndex<chords.at(chIndex).offsets.size();ntIndex++)
                    {
                        int step;
                        for (step=chStartStep;step<theSequence.size();step++)
                        {
                            String noteId = String(theSequence.at(step)->track)
                            +"_"+String(theSequence.at(step)->channel)
                            +"_"+String(theSequence.at(step)->noteNumber);
                            if (chords.at(chIndex).noteIds.at(ntIndex)==noteId)
                                break;
                        }
                        if (step<theSequence.size())
                        {
                            const double noteTimeStamp = chords.at(chIndex).chordTimeStamp+chords.at(chIndex).offsets[ntIndex];
                            theSequence.at(step)->setTimeStamp(noteTimeStamp);
                            chords.at(chIndex).notePointers.push_back(theSequence.at(step));
                            theSequence.at(step)->chordIndex = chIndex;
                            theSequence.at(step)->noteIndexInChord = ntIndex;
                            theSequence.at(step)->inChord = true;
                        }
                    }
                }
            }
            else //It Was a midi file so create a new chords list
            {
                chords.clear();
                double thisChordTimeStamp;
                for (int step=0; step<theSequence.size();step++)
                {
                    thisChordTimeStamp = theSequence.at(step)->getTimeStamp();
    //                std::vector<std::shared_ptr<NoteWithOffTime>> tempChordNotes;
                    ChordDetail detail;
                    while (step<theSequence.size() && theSequence.at(step)->getTimeStamp() == thisChordTimeStamp)
                    {
                        //                    nt.indexOfChordDetail = chords.size();
                        //                    nt.indexOfChordDetail = -1; //Mark as not in a chord for now
                        detail.notePointers.push_back(theSequence[step]);
                        step++;
                    }
                    step--;
                    
                    for (int j=0;j<detail.notePointers.size();j++)
                    {
                        detail.notePointers.at(j)->noteIndexInChord = j; //Tell this note it's current index in the chord
                        //We mark every non chord note-1 and every chord note with the index in chords[ ]  of its chord.
                        if (detail.notePointers.size()==1) //One note, so not a chord
                            detail.notePointers.at(j)->inChord = false;
                        else
                        {
                            detail.notePointers.at(j)->chordIndex = chords.size();
                            detail.notePointers.at(j)->inChord = true;
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
                        detail.chordTimeStamp = detail.notePointers.at(0)->getTimeStamp();
                        detail.timeSpec = "manual";
                        chords.push_back(detail);
//                        std::cout <<"Next chord: chordNum "<<chords.size()-1
//                        <<" timeStamp " <<detail.timeStamp
//                        << " nNotes "<< detail.notePointers.size();
//                        if (detail.notePointers.size()>0)
//                            std::cout << " firstIndex " << detail.notePointers.at(0)->getTimeStamp();
//                        std::cout <<"\n";
                    }
                }
            }
        }
      } catch (const std::out_of_range& ex) {
          std::cout << " error loadSequence: updating chords " << "\n";
      }
        
        //###
//        std::cout << "rds \n";
      try {
        if (targetNoteTimes.size()>0)
        {
            double prevTimeStamp = -1;
            for (int step=0;step<theSequence.size();step++)
            {
                const double timeStamp = theSequence.at(step)->getTimeStamp();
                theSequence.at(step)->targetNote = false;
                if (timeStamp != prevTimeStamp)
                {
                    if (targetNoteTimes.contains(theSequence.at(step)->getTimeStamp()))
                        theSequence.at(step)->targetNote = true;
                }
                prevTimeStamp = timeStamp;
            }
        }
        
        double thisChordTimeStamp;
        std::vector<std::shared_ptr<NoteWithOffTime>> chordNotes;
        int chordTopStep;
        for (int step=0; step<theSequence.size();step++)
        {
            //All sequential notes with the same chordIndex are assumed to be part of this chord (if any)
            int thisStepChordNoteIndex = theSequence.at(step)->chordIndex;
            int nextStepChordNoteIndex;
            if (step+1<theSequence.size())
            {
                nextStepChordNoteIndex = theSequence.at(step+1)->chordIndex;
            }
            else
                nextStepChordNoteIndex = INT32_MAX;
        
            chordNotes.push_back(theSequence[step]);
            if (chordNotes.size()==1)  //If there's one step it's always the chord top
                chordTopStep=step;
            if (thisStepChordNoteIndex != nextStepChordNoteIndex)
            {
                if (chordNotes.size()>1 & thisStepChordNoteIndex!=-1)
                {
                    //We are now at the start of the next chord and chordNotes[ ] contains pointers to its notes, and
                    // thisStepChordNoteIndex is the chord's index
//                    std::cout << thisStepChordNoteIndex<< " "<<step<<" timeSpec before "
//                    <<chords[thisStepChordNoteIndex].timeSpec<<"\n";
                    String timeSpec = chords[thisStepChordNoteIndex].timeSpec;
                    if (timeSpec.startsWith("h:"))
                    {
                        timeSpec = timeSpec.fromFirstOccurrenceOf(":", false, true);
                        const String randomnessStr = timeSpec.initialSectionContainingOnly("0123456789");
                        String chordDurationStr =  timeSpec.getLastCharacters(timeSpec.length()-randomnessStr.length());
                        String seedStr = chordDurationStr.fromFirstOccurrenceOf(":", false, true);
                        chordDurationStr = chordDurationStr.upToFirstOccurrenceOf(":", false, true).substring(1,999);
                        double variation = randomnessStr.getDoubleValue();
                        int chordDirection =  timeSpec.containsAnyOf("/") ? 1 : -1; //It contains either '/' or '\'
                        double chordDuration = chordDurationStr.getDoubleValue();
                        int seed = seedStr.length()>0 ? seedStr.getIntValue() : (int) thisChordTimeStamp;
                        struct {
                            bool operator()(std::shared_ptr<NoteWithOffTime> a, std::shared_ptr<NoteWithOffTime> b) const
                            {
                                return a->noteNumber > b->noteNumber;
                            }
                        } customCompare2;
                        std::sort(chordNotes.begin(), chordNotes.end(),customCompare2);
                        
                        thisChordTimeStamp = chords[thisStepChordNoteIndex].chordTimeStamp;
                        chords[thisStepChordNoteIndex].notePointers.clear();
                        chords[thisStepChordNoteIndex].offsets.clear();
                        chords[thisStepChordNoteIndex].notePointers.push_back(chordNotes.at(0));
                        chords[thisStepChordNoteIndex].offsets.push_back(0);

                        //Note that some of this code is for use in the future ability to do broken chords
                        for (int i=0; i<chordNotes.size(); i++)
                            chordNotes.at(i)->chordTopStep = chordTopStep;
                        
                        int lastChordNote = 0;
                        int firstChordNote = INT_MAX;
                        for (int j=0;j<chordNotes.size();j++)
                        {
                            if (chordNotes.at(j)->currentStep > lastChordNote)
                                lastChordNote = chordNotes.at(j)->currentStep;
                            if (chordNotes.at(j)->currentStep < firstChordNote)
                                firstChordNote = chordNotes.at(j)->currentStep;
                        }
                        double nextNoteTime, prevNoteTime;
                        if (lastChordNote<theSequence.size()-1)
                            nextNoteTime = theSequence.at(lastChordNote+1)->getTimeStamp();
                        else
                            nextNoteTime = theSequence.back()->getTimeStamp();
                        if (firstChordNote==0)
                            prevNoteTime = 0;
                        else
                            prevNoteTime = theSequence.at(firstChordNote-1)->getTimeStamp();
                        
                        double chordSpan;
                        if (chordDirection>0) //Put notes before chord timeStamp
                            chordSpan = thisChordTimeStamp - prevNoteTime;
                        else
                            chordSpan = nextNoteTime - thisChordTimeStamp;
                        
                        if (chordSpan<chordDuration)
                            chordDuration = chordSpan - 1;
                        
                        if (variation >= 0.5*(nextNoteTime-thisChordTimeStamp))
                            variation = 0.5*(nextNoteTime-thisChordTimeStamp);

                        //TODO
                        //- adjust randomness for space?
                        //- compute non random note times
                        //- add randomness to each note adjusting the resulting time to be between prevNoteTime & nextNoteTime
                        srand(seed);
                        chords[thisStepChordNoteIndex].timeRandSeed = seed;
                        //double increment = -1 * chordDirection * chordDuration / (chordNotes.size()-1);
                        for (int i=1; i<chordNotes.size(); i++) //Don't change top chord note so start at 1
                        {
                            int foo;
                            if (thisStepChordNoteIndex>1138)
                                foo = 0;
                            double proposedNoteTime = thisChordTimeStamp;// + i*increment;
                            const int temp = variation*100;
                            double randomAdd;
                            unsigned r = rand();
                            if (temp==0)
                                randomAdd = 0;
                            else
                                randomAdd = r%temp/100.0;
//                            std::cout << step << " randomAdd " << randomAdd << std::endl;

                            proposedNoteTime += randomAdd;
//                            const double noteDuration = chordNotes.at(i)->getOffTime()-chordNotes.at(i)->getTimeStamp();
                            if (proposedNoteTime>=seqDurationInTicks)
                                proposedNoteTime = seqDurationInTicks - 1;
                            chordNotes.at(i)->setTimeStamp(proposedNoteTime);
                            if (chordNotes.at(i)->getOffTime() > seqDurationInTicks)  //???
                                chordNotes.at(i)->setOfftime(seqDurationInTicks);
                            chords[thisStepChordNoteIndex].notePointers.push_back(chordNotes.at(i));
                            const int offset = chordNotes[i]->timeStamp - thisChordTimeStamp;
                            chords[thisStepChordNoteIndex].offsets.push_back(offset);
                        }
                        
                        //Also to do:
                        //- rationalize saving of random seed
                        //- display of chord properties
                        //- save original note times?
                    }
                    String velSpec = chords[thisStepChordNoteIndex].velSpec;
                    if (velSpec.startsWith("h:"))
                    {
                        velSpec = velSpec.fromFirstOccurrenceOf(":", false, true);
                        Array<double> strengths;
                        std::string numStr = velSpec.toStdString();
                        std::string delimiter = ",";
                        size_t pos = 0;
                        std::string token;
                        while ((pos = numStr.find(delimiter)) != std::string::npos) {
                            token = numStr.substr(0, pos);
                            strengths.add(String(token).getDoubleValue());
//                            std::cout << "strength " << strengths.getLast() << std::endl;
                            numStr.erase(0, pos + delimiter.length());
                        }
                        strengths.add(String(numStr).getDoubleValue());
//                        std::cout << "strength " << strengths.getLast() << std::endl;
                        
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
//                    std::cout << thisStepChordNoteIndex<< " offsets[1] " <<chords[thisStepChordNoteIndex].offsets[1]<<"\n";
                    chordNotes.at(0)->chordTopStep=-1;
                }
                else
                    theSequence.at(step)->chordTopStep=-1;
                chordNotes.clear();
            }
        }
      } catch (const std::out_of_range& ex) {
          std::cout << " error loadSequence: processing chords " << "\n";
      }
        
//        for (int i=0;i<chords.size();i++)
//            std::cout << "Chord "<<i<<" "<<chords[i].timeStamp<<" "<<chords[i].nNotes<<"\n";
      try {
          struct {
              bool operator()(std::shared_ptr<NoteWithOffTime> a, std::shared_ptr<NoteWithOffTime> b) const
              {
                  if (a->getTimeStamp()==b->getTimeStamp())
                      return a->noteNumber > b->noteNumber;
                  else
                      return a->getTimeStamp() < b->getTimeStamp();
              }
          } customCompare;
          std::sort(theSequence.begin(), theSequence.end(), customCompare);
        for (int step=0;step<theSequence.size();step++)
            theSequence.at(step)->currentStep = step;
        //        //This reconstructs the chains using targetNoteTimes loaded from the ck file or constructed by chain command
        if(alreadyChained) //If loaded from ck file or previously created for this new midi file by chain ()
        {
            int firstInThisChain = 0;
            double prevTS = -1.0;
//            std::cout<< "Chain in loadSequence " << "\n";
            for (int step=0; step<theSequence.size();step++)
            {
                if (prevTS != theSequence.at(step)->getTimeStamp() && theSequence.at(step)->targetNote)
                {
                    if (step>0)
                        theSequence.at(step-1)->triggers = -1;//Last note in chain triggers nothing
                    theSequence.at(step)->triggeredBy = -1; //This will be set based on the shortest note near the start of this group, done below
                    firstInThisChain = step;
                    theSequence.at(step)->firstInChain = step;
                }
                else
                {
                    if (step>0)
                    {
                        theSequence.at(step-1)->triggers = step;
                        theSequence.at(step)->triggeredBy = step-1;
                    }
                    else
                        theSequence.at(step)->triggeredBy = step-1;
                    theSequence.at(step)->firstInChain = firstInThisChain;
                }
                prevTS = theSequence.at(step)->getTimeStamp();
            }
        }
        else
        {
            chain(Array<int>(),chainingInterval); //This is used only for when a plain midi file is loaded
            alreadyChained = true;
        }
    
        
        //Find chainTriggers and set this property for all steps in a given chain
//        std::cout <<"Find chainTriggers \n";
        assert (theSequence.size()>0);
        int step = 0;
        int chainTrigger = 0;
        while (step < theSequence.size())
        {
            //If this step is a firstInChain scan all notes with time stamp equal to that of the firstInChain for the shortest note
            if (theSequence.at(step)->targetNote)
            {
                chainTrigger = step;
//                int highestNote = -1;
                int stepOfhighestNote = step;
                int subStep = step;
                while (subStep<theSequence.size()-1 && theSequence.at(subStep)->getTimeStamp()==theSequence.at(theSequence.at(step)->firstInChain)->getTimeStamp())
                {
                    if (theSequence.at(subStep)->noteNumber >= theSequence.at(stepOfhighestNote)->noteNumber)
                    {
                        stepOfhighestNote = subStep;
                    }
                    subStep++;
                }
                chainTrigger = stepOfhighestNote;
            }
            assert(0<=chainTrigger && chainTrigger<theSequence.size());
            theSequence.at(step)->chainTrigger = chainTrigger;
            step++;
        }

        //Store highest velocity in each chain in every step of the chain
        for (int step=0;step<theSequence.size();step++)
        {
            int firstInChain = step;
            float highestVelocity = -1;
            int firstStep = step;
            bool enteredLoop = false;
            while (step<theSequence.size() && theSequence.at(step)->firstInChain==firstInChain)
            {
                enteredLoop = true;
                if (theSequence.at(step)->velocity > highestVelocity)
                    highestVelocity = theSequence.at(step)->velocity;
                step++;
            }
            if (enteredLoop)
                step--;
            
            //Store highest velocity in this chain in every step of this chain
            for (int i=firstStep; i<=step; i++ )
            {
                theSequence.at(i)->highestVelocityInChain = highestVelocity;
            }
        }
        
        //Determine which steps are triggeredNotes and triggeredOffNotes
        //triggeredNotes are steps that start no later than the triggeredNoteLimit from the chainTrigger.
        //triggeredOffNotes are triggeredNotes that end before the END of the next chainTrigger note.
//        std::cout << "Determine which steps are triggered notes \n";
      } catch (const std::out_of_range& ex) {
          std::cout << " error loadSequence: chaining " << "\n";
      }

      try {
        for (int step=0; step<theSequence.size();step++)
        {
            if ( theSequence.at(step)->getTimeStamp() <= theSequence.at(theSequence.at(step)->chainTrigger)->getTimeStamp()+triggeredNoteLimit)
            {
                theSequence.at(step)->triggeredNote=true;
                //Scan for nextFirstInChain
                int nextFirstInChain = theSequence.at(step)->firstInChain+1;
                while (nextFirstInChain<theSequence.size()-1 && theSequence.at(nextFirstInChain)->triggeredBy != -1)
                    nextFirstInChain++;
                
                //Two cases for a step to be a triggeredOffNote:
                //1) Where a the step ends before the next chain trigger
                //2) Where the step's offtime is no more than the start of any triggered note of the chain that starts after it)
                if (nextFirstInChain<theSequence.size()-1
                    && theSequence.at(step)->triggeredNote && theSequence.at(step)->getOffTime() < theSequence.at(nextFirstInChain)->getTimeStamp())
                    theSequence.at(step)->triggeredOffNote = true;
                else
                    theSequence.at(step)->triggeredOffNote = false;
                
                //Scan for case (2) above
                int i = nextFirstInChain;
                bool shouldSustain = false;
                while (i<theSequence.size() && theSequence.at(i)->getTimeStamp()<= theSequence.at(nextFirstInChain)->getTimeStamp()+triggeredNoteLimit) //All triggered notes in next chain
                {
                    if (theSequence.at(step)->getOffTime() > theSequence.at(i)->getTimeStamp())
                    {
                        shouldSustain = true;
                        break;
                    }
                    i++;
                }
                theSequence.at(step)->triggeredOffNote = !shouldSustain;
                theSequence.at(step)->autoplayedNote = false;
            }
            else
            {
                theSequence.at(step)->autoplayedNote = true;
            }
        }
      } catch (const std::out_of_range& ex) {
          std::cout << " error loadSequence: finding triggered and triggeredOff notes " << "\n";
      }
    }
        
    setLoadingFile(false);
//    dumpData(0, 20, -1);
    //We assume that rewind will always be called after loadSequence, and that rewind calls sendChangeMessage
//    compareAllNotes("End of loadSequence");

//    std::cout << "End of loadSequence \n";
    return (theSequence.size()>0);
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
    << " autoPlayed "
    << " inChord "
    << " chordTopStep "
    << " chordIndex "
    << "\n";
    if (end>theSequence.size())
        end = theSequence.size();
    for (int i=start; i<theSequence.size() && i<=end;i++)
    {
        if (nn==-1 || theSequence.at(i)->noteNumber==nn )
        {
            String note = MidiMessage::getMidiNoteName (theSequence.at(i)->noteNumber, true, true, 3);
            std::cout
            << i <<" "
            << (int)theSequence.at(i)->track <<" "
            << theSequence.at(i)->channel <<" "
            << theSequence.at(i)->noteNumber <<" "
            << note<<" "
            << std::round(theSequence.at(i)->velocity*127.0) <<" "
            << theSequence.at(i)->getTimeStamp()<<" "
            << theSequence.at(i)->getOffTime()<<" "
            << theSequence.at(i)->firstInChain<<" "
            << theSequence.at(i)->triggers<<" "
            << theSequence.at(i)->triggeredBy<<" "
            << theSequence.at(i)->chainTrigger<<" "
            << theSequence.at(i)->triggeredNote<<" "
            << theSequence.at(i)->triggeredOffNote<<" "
            << theSequence.at(i)->autoplayedNote<<" "
            << theSequence.at(i)->inChord<<" "
            << theSequence.at(i)->chordTopStep<<" "
            << theSequence.at(i)->chordIndex<<" "
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
