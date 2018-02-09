
# Concert Keyboardist User's Manual

### Version and Licensing

* Document Version: January 24, 2018 for Concert Keyboardist Alpha
* Concert Keyboardist Licensing: [Licenses and Copyrights]

## Contact Information

For questions or comments contact: chrisgr99@gmail.com

## Quick Introduction to Concert Keyboardist

Concert Keyboardist is an easy to learn musical instrument that never lets you play a wrong note.  When you load any midi file Concert Keyboardist becomes an instrument to perform that particular piece of music.  You control when and how notes are played, in real time.  Play like a "Concert Keyboardist" on your computer keyboard or use a midi keyboard for even more expressiveness.

Where to get midi files to play?  There are tens of thousands available around the Internet for download or you can create your own.  Before Concert Keyboardist, midi files always replayed exactly the same.  With Concert Keyboardist music comes alive under your fingers.

## This is Concert Keyboardist

<iframe width="560" height="315" src="https://www.youtube.com/embed/xb66fzwdDS4?rel=0" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

<img src="../Common/img/Pathetique.jpg" alt="CK" style="width:100%;margin:20px 0px 10px 0px">

**How it works**:  In the above image the heads of the "Target Notes" have magenta heads and "Chained" notes have white or grey heads. Each successive key (any key) you press, Concert Keyboardist triggers the next successive Target Note in the score.  It also schedules any notes Chained off each target note for autoplaying according to the tempo.  Chained accompaniment notes can continue to play while you play overlapping target notes.  It's easy to play polyphonic music, even complex chords.  Play multi channel midi with multiple simultaneous instruments, or mute some combination of channels to concentrate on some part of the music.

As you play, the score scrolls right-to-left based on tempos in the score's midi file.  You control when to play target notes and how long to hold them.  Concert keyboardist continually shows you the notes you've played, currently playing notes, and upcoming unplayed target notes, and it gives you a visual cue of when to play the next target note at the current tempo.  

This carefully designed note triggering system and ongoing visual feedback is crucial to the ability to play a midi file live,  and more than two years of testing went into perfecting it.

When you play on computer keyboard Concert Keyboardist uses note velocities from the score.  When you play on a velocity sensitive midi keyboard Concert Keyboardist uses velocities from the keyboard for the target notes and velocities of chained notes are adjusted proportional to the velocity of their target note.

Concert Keyboardist gradually adjusts its tempo (rate of scrolling) toward your playing tempo.  You are not locked to the tempo in the midi file, and you don't have to play target notes exactly when Concert Keyboardist suggests.  If you did play them exactly when suggested and hold them exactly as long as suggested, the result would be exactly like playing back the midi file.  But even if you tried to do this your natural human variations would bring life to the music, or you could play notes at more varied times that feel right to you for the flow of the music, or vary the rhythm.

Concert Keyboardist has tools to edit which notes are target notes, note velocities, note start times, note durations, variations in the  tempo, and to "humanize" the relative notes times and velocities of notes in midi chords.  You can save your edited scores in Concert Keyboardist files which are normal midi files but with an extra invisible track for additional settings.

However, it's important to understand that in this version of Concert keyboardist it's not possible to create scores or add and remove notes.  There are many powerful applications for creating and manipulating midi files, but Concert Keyboardist is unique in its ability to allow you to "perform" them under real time control.

## Installation and Setup on a Macintosh

Concert Keyboardist is compatible with Mac OS 10.6 or higher. To install: 

* Download and uncompress the install files.
* Copy the Concert keyboardist application to your Applications folder.  
* Copy the included "Concert Keyboardist Files" folder to your Documents folder or a folder where you keep midi files. 
* Concert keyboardist can produce sound with a VST or Audio Unit plugin, or by sending midi to another software or hardware synthesizer.  
* If you are familiar with plugins load a piano-like one in using the [Plugin Management Tools].  Then configure the Concert Keyboardist audio output using Audio and Midi Settings in [Settings Tools].  
* If you don't use a plugin you will need to route midi from Concert keyboardist to a standlaone software or hardware synthesizer.  The easiest to set up is GarageBand which is free and has a wide range of sounds to choose from.  GarageBand automatically receives midi messages from all midi output on your system so it automatically receives from the Concert Keyboardist default output.
* To set up with GarageBand: 
    * Download and install GarageBand (it's free to Mac owners).  
    * Open GarageBand.
    * When it opens choose "Empty Project"
    * Choose "Software Instrument".  It should default to Classic Electric Piano.
    * You could play the Classic Electric Piano but it's better to use a true piano sound for the piano music included with Concert Keyboardist.
    * To choose a piano sound click on "Sounds" in the frame on the left, click on "Piano" and double click on "Steinway Grand Piano".
    * Then open Concert Keyboardist and follow the steps in [Quick Start to Playing], below.  When you play you should hear piano notes.
* If you are more experienced with midi you should know how to configure your preferred midi synthesizer to receive from the Concert Keyboardist default output.
* If you have a midi keyboard attached to your computer, configure it as the Concert Keyboardist midi input as shown in Audio and Midi Settings in [Settings Tools].  Be sure your controller is active when Concert Keyboardist is started.
* Go to the [Quick Start to Playing].

## Installation and Setup on Windows

Concert Keyboardist is compatible with Windows 7 and Windows 10 (possibly Windows XT but this has not been tested). To install: 

* Download the Windows installer.
* Double click on the installer to run it and follow the prompts.  
* It will install Concert Keyboardist to your Programs folder and create a directory called "Concert Keyboardist Files" of example files in your Documents directory.
* Run Concert Keyboardist. You should find shortcuts to Concert Keyboardist on Desktop and in the Start menu.  If you wish, pin it to the Taskbar.
* Configure the Concert Keyboardist audio output as described in the Audio and Midi Settings in [Settings Tools].
* Concert keyboardist can produce sound with a VST plugin, or by sending midi to another application. 
* If you are familiar with audio plugins and have any on your computer, load a suitable one (e.g. piano) as described in the [Plugin Management Tools] section below.
* Instead of a plugin you can route midi from Concert keyboardist to a standalone software or hardware synthesizer.  
* To route midi to a software synthesizer will need a virtual midi routing driver such as [LoopBee](http://www.nerds.de/en/loopbe1.html), which is free for the basic version, which is sufficient.  
* Install LoopBee, or the equivalent.  Then choose it as both the Concert Keyboardist midi output  as described in Audio and Midi Settings in [Settings Tools].  Also set LoopBee as the input port on your software synthesizer.
* If you have an external hardware synthesizer attached to your computer, choose that as the Concert Keyboardist midi output.
* On your synthesizer choose a suitable sound such as piano.
* Go to the [Quick Start to Playing].

## Quick Start to Playing

* If at any point you need more detail, refer to the [Overview of The Main Window].
* **Load a file:**  You can use the File-Open menu, the toolbar button, or Cmd+O.  A good introductory example is "Minuet in G[ck].mid" which is among the example files.  When loaded it should look something like the image in the [Overview of The Main Window].
* **Prepare to play:**  Press the spacebar.  This turns the Current Time Line orange, indicating that Concert Keyboardist is ready to play.  In this mode as soon as you press a key the note at the Current Time Line is played and the note bars start scrolling to the left.  The scroll rate is based on the tempo.
* **Triggering Notes:** On your computer keyboard press any letter key, or any of the characters "[ ] ; ' < > and /".  On an English language keyboard these are the rows from "q" to "]", from "a" to " ' " and from "z" to "/".  If you are using a midi keyboard playing **any** note triggers the next target note.
* **Note Timing:** To play at the original tempo and rhythm press a key each time a target note reaches the vertical yellow line drawn when the previous note was played.  You can play with one finger, or two or more fingers to play faster or smoother.  
* **Stopping:** Press the spacebar again, or press the Return key for rewind, or roll the mouse wheel to scroll left or right.
* **Yellow note heads** are those that are currently sounding.  As you play, notice that long notes continue sounding even as you trigger shorter simultaneous notes.
* **Chords:** In the "Minuet in G" example the top notes of chords are target notes.  Simultaneous or nearby chord notes are chained from them, like any other chained notes.  This allows chords to be triggered by one note..
* **Trills** are like any other chained notes.  For example in bar 8 of Minuet in G the notes are chained making them easier to play.
* **Rewind:** Press the Return key.  The first time you press Return the transport rewinds to the place you most recently started.  A second press returns to the start.  You can also press Return while the transport is running to stop and transport and rewind in one step.
* **Basic Editing** You can edit which notes are target notes.  Click on the head a note to toggle its target vs chained status. Setting a note to be chained causes it to be added to the chain of the preceding target note.
* **Playing legato notes** This is a technique where the on and off times of successive notes are overlapped resulting in smooth sounding transitions.  If you play with at least two fingers you can overlap the on and off times of successive notes to create legato as in normal piano playing.  
* **The Relative Time Line:** This is the yellow vertical line that appears at the start of the most recently played target note. The Relative Time Line is a guide to when to play the next target note relative to when you played this note.  In playing expressively you may be triggering notes before or after the Current Time Line.  It's helpful to have a relative time marker showing when notes were actually played, not when they were suggested to be played.  Experiment with playing notes before or after the Current Time Line to understand this better.  The yellow line remains after you stop playing to mark the last note played before stopping.

<!-- ![](../Common/img/CKMainWindow.jpg) -->

<img src="../Common/img/CKMainWindow.jpg" alt="CK" style="float:right;width:75%;margin:0px 10px 10px 10px">

## Overview of The Main Window

* Concert Keyboardist is like a player piano.  **Notes** are horizontal bars that scroll right to left horizontally based on the current **tempo**.  A note's vertical position shows pitch and its left end shows start time. Its length shows duration.  This information is read from the midi file and guides you when to play notes and how long to hold them.
* In real time while you play **you control note start times and durations**.  This is like playing sheet music.  The notes are there but you add expression as you play.  If you have a velocity sensitive midi keyboard you can also control note velocities.
* The score can be **scrolled** left or right with the mouse wheel or two finger drag on a touch pad.  The end of the score is at the far right.
* The blue vertical line is the **"Current Time Line"** which indicates the current playing time in the score.  It turns amber when ready to play and green while you are playing.
* While you are playing, pressing any key instantly triggers the next unplayed **Target Note**, magenta-headed-notes.  The these are the notes you directly play.
* When a Target Note is played it also schedules for playing all notes **chained** from it up to just before the next target note.  Chained notes are those without magenta heads.  
* Chained notes are triggered at appropriate times in the future based on the current tempo.
* After all scheduled chained notes have been played no further notes will sound until you trigger the next target note.
* Although a target note's position suggests its start time, you can play it before or after that time, giving you control over the **flow and rhythm** of the music.  The **tempo** gradually adjusts itself to your playing speed.  If you continually play notes before their scheduled time the tempo speeds up and if you play notes behind the tempo slows.
* If you try to play the next target note while previously chained notes are still playing, that target note and its chained notes are added to the previous chain.
* Each time you trigger a target note a vertical yellow **"Relative Time Line"** is drawn at that exact position as a guide to when to play the next target note.  To play the upcoming target note at the current tempo, play it when it reaches the Relative Time Line.  You are free to play it earlier or later which alters the timing of that note and its chained notes.  i.e. The feel or rhythm of the music.  If you continue playing early or late the tempo gradually slows or speeds up.

## Navigating Around The Score

* **Scroll** with the touchpad or mouse wheel.  
* Press the **Return Key** to go where you most recently started playing.  Press a second time to return to the start.
* **The Right and Left arrow keys** step one target note at a time forward or backward in the score.
* **The Shift+Right Arrow and Shift+Left Arrow keys** step one measure at a time forward or backward in the score.
* **Bookmarks:** Cmd+Right Arrow and Cmd+Left Arrow (Ctrl on Windows) step one bookmark at a time forward or backward in the score.
*  Press Cmd+b to create a bookmark at the Current Time Line.  To delete a bookmark press Cmd+b with the bookmark already at the 
Current Time Line.

## Selecting Notes

<<Video - Selecting Notes>>

Many commands act on the currently "selected notes".  Here is how to control the selection:

Select a range of notes:

* Move the mouse pointer to anywhere not on a note bar.
* Press the left mouse button and drag the yellow rectangle to surround heads of all notes to be selected.
* Release the mouse button. 
* The head of each selected note will be surrounded by a white box. 

Add notes to the selection:

* Hold down the shift key, starting with some notes selected.
* Drag the yellow rectangle around other notes.
* They will also be selected.  

Remove notes from the selection:

* Hold down the Option key and drag, starting with some notes selected.
* Any selected notes you drag the yellow rectangle around over will be deselected.

Drag the selection past the left or right of the visible display:

* Drag as above but move the mouse pointer a little bit past the left or right end of the score.
* The score will autoscroll in that direcction.
* It will stop if it reaches that end of the score.

Select all notes in the score:

* Use the keyboard shortcut Cmd+a on Mac or Ctrl+a on Windows.

Clear the selection:

* Click anywhere not on a note bar, or
* Press the Escape.

Select a single note:

* Start on the background near it and drag a yello rectangle around it.
* You can't select a single note by clicking directly on its head as this toggles it between being a target or non target note.  

## Note Information Viewer

<img src="../Common/img/Information Area.jpg" alt="CK" style="width:30%;margin:0px 0px 0px 10px">

The Note Information Viewer is in the lower left corner of the window.  

* Hover the mouse pointer over a note bar to see details of the note, its note name, note number, channel, track, etc.
* Hover over a note track to see the note name and octave of the track.
* While drag-selecting the viewer shows the time in ticks of the first and last selected note, and the duration of the selection in ticks.

## Main Toolbar

### File Management Tools

<img src="../Common/img/FileManagementTools.jpg" alt="CK" style="width:9%;margin:0px 0px 0px 10px">

These tools are the same as the commands in the File menu, and are very similar to commands you will have used in many other applications.

* "File Open" opens either a normal midi file (extension .mid) or a Concert Keyboardist file (extension [CK].mid).  Opening a normal midi file imports it.
* "File Save"  saves the current score replacing the file it was loaded from.  If you save an just-imported midi file this command is the same as Save As.  When you save an imported midi file it is saved as a Concert keyboardist file and [CK] is added to the name in front of the .mid extension.   
* "File Save As" creates a new file that you can give a different name.
* You can also open a file by dragging and dropping it on the Concert Keyboardist window.
* If you try to open a file without saving your edits to the previous file, Concert Keyboardist warns you and gives you a chance to saave your work.

### Plugin Management Tools

<img src="../Common/img/PluginManagementTools.jpg" alt="CK" style="width:6%;margin:0px 5px 0px 25px">

Before loading a plugin, scan your system making a list of plugins to choose from:

* Click on the Plugin Management Tool  on the left and choose that looks like a puzzle piece and choose "Manage Available Plugins...". 
* Click the Options button on the lower left and choose from the options to scan for VST and VST3 plusins.  
* On a Mac also scan for Audio Unit plugins.
* You should see plugins being added to the plugin management window.

To load a plugin:

* Click on the same button as above and choose any audio synthesizer in the menu.
* The plugin should be loaded and its window should open.
* Choose a patch (sound) in the window in whatever manner required by that plugin.
* You can leave the plugin window open and click on the main Concert keyboardist window to the front by clicking on it.
* Or close the plugin window.  You can reopen is as below.

To change the sound or edit settings of a loaded plugin:

* Click on the tool that looks like a puzzle piece with an orange arrow over it.

To unload a plugin (possibly to use an external synthesizer):

* Click on the tool that looks like a puzzle piece and choose "No Plugin".

To ennable or disable the midi out port (that routes midi to external synthesizers):

* Click on the Plugins menu at the top of the screen (on Mac) or the top of the window (on Windows).
* Choose "Enable Midi Out" to check or uncheck that menu item.

### Settings Tools

<img src="../Common/img/SettingsTools.jpg" alt="CK" style="width:6%;margin:0px 0px 0px 25px">  There are two settings tools:

* **Audio and Midi Settings**
    * Allows you to choose the audio output port used by plugin loaded into CK, if any.
    * Allows you to choose an midi input device controller such as a keyboard, you will be able play CK
    with the controller.  CK will always play the notes in the score no matter what notes you press on the keyboard.  
    * Allows you to choose a midi output device to which midi messages from CK will be sent.  Choose no output if you  
    are playing a plugin loaded into CK. 

* **Tracks Viewer and Settings** 
    * Displays information on all the tracks that were in the originally imported midi file.  
    * You can click on the button at the right to enable or disable tracks.  The notes of disabled tracks do not appear in the scrolling view and are not played.  Only tracks that contain notes can be selected for inclusion in the display.  
    At least one track must be enabled.  
    * Midi files may or may not contain some tracks that give information about the contents of the file,
    the file's creator, copyrights, and other information.
    

### Hide/Show Editing Tools 

<img src="../Common/img/ToggleEditToolbarTool.jpg" alt="CK" style="width:4%;margin:0px 0px 0px 25px">

* The Editing Toolbar defaults to being hidden.  
* Click the "Hide/Show Editing Tools" button to show the editing toolbar and click again to hide it.  
* The editing toolbar also automatically is hidden when you start playing and reappears when you stop.
* The keyboard shortcut hode/show the edit toolbar is is Cmd+e (Mac) and Ctrl+e (Windows).

### Bookmark Tool

<img src="../Common/img/BookmarkTool.jpg" alt="CK" style="width:4%;margin:0px 0px 0px 25px">

* Adds or removes bookmarks that allow you to quickly return to a place in the score:
* Use the keyboard shortcuts Cmd+Right/Left Arrow (Mac) or Ctrl+Right/eft Arrow (Windows) to step between bookmarks.
* Click the Bookmark Tool to add a bookmark at the position of the Current Time Line. 
* The Bookmark Tool turns into a Remove Bookmark Tool when any bookmark is directly above the Current Time Line.
Click the button to remove the bookmark.
* The keyboard shortcut for add/remove bookmark is Cmd+b (Mac) and Ctrl+b (Windows).

### Transport Tools

<img src="../Common/img/TransportTools.jpg" alt="CK" style="width:12%;margin:0px 0px 0px 25px">

* **Listen**:  The Listen button lets you hear the music starting at the Current Time Line.  Press the Spacebar to stop listening and return to where listening started.  While listening the display scrolls and you can hear the notes, but in this version sounding notes are not marked in yellow as they would be while actually playing.
* **Rewind**: 
    * Pressing Rewind once moves the display back to the first note you played the last time you started playing.  
    * Pressing Rewind again moves the display to the start of the score.  
    * Pressing it again returns again to the to the first note you played the last time you started playing.  
    * Press the Return key as a keyboard shortcut for Rewind.
* **Play**
    * Pressing the Play button turns the Current Time Line orange but does not start the score scrolling.  
    * The next playable key you press will trigger the Target Note that is at at the Current Time Line.
    * The display will start scrolling.
    * Press the Spacebar when not playing is is a keyboard shortcut for Play.
* **Stop**
    * Pressing the Stop button stops the display scrolling and sends not-offs for any notes that are on.
    * Press the Spacebar when playing as a keyboard shortcut for Stop.

### Tempo Graph and Tempo Tools

<img src="../Common/img/TempoTools.jpg" alt="CK" style="width:14%;margin:0px 0px 0px 25px">

* The Tempo Graph is the red horizontal line on the Concert Keyboardist display.  It graphs the **original tempo** at each 
position in the score. The tempo in midi file may have been configured to with position in the score as dicated by the music.
* Note that Midi files that were originally recorded live, such by recording from a midi keyboard, usually don't include tempo
variations.... TBD 
* The first number in the toolbar tempo tools is the tempo in beats per minute at the Current Time Line.
* To see the tempo at a given position in the score, scroll the score so that position is at the Current Time Line.
* You adjust how fast the entire score will play by by dragging up or down on the tempo number.  You will see the red line
move up or down.  
* Next to the tempo number is a **percentage** showing the ratio of the actual playing tempo as compared to the original score tempo.  
As you drag up/down on the temp number the percentage number will be updated to show how the currently selected tempo is of the 
original score tempo.
* Normally this will change the tempo everywhere in the score keeping the tempo everywhere to be the same ratio TBD
* The Tempo Change Tool
    * Inserts a "Tempo Change Bookmark" that allows a change in the ratio of the played tempo to the original tempo.
    * To remove a tempo change TBD

### Help Tool

<img src="../Common/img/HelpTool.jpg" alt="CK" style="width:4%;margin:0px 0px 0px 25px">

Opens the Concert keyboardist documentation in your default web browser.

## Editing Toolbar

### Undo and Redo Tools

<img src="../Common/img/UndoRedoTools.jpg" alt="CK" style="width:6%;margin:0px 0px 0px 25px">

These buttons perform undo and redo, for any of the following commands:

* The [Toggle Target Notes Tool].
* The [Note Chaining Tools].
* Dragging notes to change their start times.
* Dragging notes to change their durations.
* All [Note Velocity Tools].
* Humanize chord note start times (in [Chord Tools])
* Humanize chord note velocities (in [Chord Tools])

### Toggle Target Notes Tool

<img src="../Common/img/ToggleTargetNotesTool.jpg" alt="CK" style="width:4%;margin:0px 0px 0px 25px">

You can edit which notes are target notes and which are not.  

* The Toggle Target Notes Tool converts notes between being target and non-target notes.  To use it select one or more notes by dragging
a marquee around them and click the tool.  The **first or leftmost** note in the selection is toggled and all other notes in the selection are
set to the resulting state of the first note.  Click the tool again to switch all selected notes between being target or non-target notes.
* A keyboard shortcut for this Cmd+t or Ctrl+t on Windows.
* Another quick way to toggle target/non-target notes is simply to click on a note head.  That note is toggled between target and non-target status.  All other selected notes also change to be the same as the resulting status of the clicked on note.  Click again to switch them to
the opposite state.

### Note Chaining Tools

<img src="../Common/img/NoteChainingTools.jpg" alt="CK" style="width:9%;margin:0px 0px 0px 25px">

Editing to set target and non-target notes for a long midi file would be a lot of work!  Fortunately the chain command is an easier way 
do most of the work automatically.  We usually want each target note to follow a break in the flow of notes.  Notes that are closer 
together we would want to be chained from the preceding target note.

The chain command acts on a selected range of notes, or the entire midi file.  The numeric field next to the chain button gives the size of 
the break in sixteenth notes that should trigger a target note to be marked.  What works best for a given midi file make take some 
experimentation, but an amount of 1.0 sixteenths works well for a lot of files.  

The chain command is automaticaly run with an interval of one sixteenth when a midi file is first imported.  This usually results in a 
quite playable file depending on its complexity.  To try other intervals select a range or all notes in the score.  Then click on 
the "chain amount" number in the toolbar, and choose some other amount from the popup list.  The selected range of notes will 
instantly be rechained based on that interval.  

If you don't like the result you can undo it.  If it's close to what you want you can adjust some notes using the [Toggle Target Notes Tool].

The chain button reapplies chain command using the visible "chain amount" number.

After chaining, usually only one note in a chord (unless it's a long broken chord) is set as a target note.  The other notes are 
chained from it, either simultaneous or offset slightly in time.  The note that came first is made the target note, which is not necessarily the top note or bottom note of the chord as you might expect.  This is why it may appear somewhat arbitrary which note of a chord becomes a target note.  

Slight variations in chord note start times result in more natural sounding music.  If they were present in the original midi
file (probably because it was recorded live by a real human) they are retained and automatically replayed at the current 
tempo when you trigger the chord's target note.  Midi files that were manually entered or step-sequenced usually do not have 
these subtle variations in chord times but you can simulate them with one of the Concert Keyboardist [Chord Tools].

### Note Velocity Tools

<img src="../Common/img/VelocityTools.jpg" alt="CK" style="width:9%;margin:0px 0px 0px 25px">

### Chord Tools

<img src="../Common/img/ChordTools.jpg" alt="CK" style="width:24%;margin:0px 0px 0px 25px">

### Pedal Tools

<img src="../Common/img/PedalTools.jpg" alt="CK" style="width:6%;margin:0px 0px 0px 25px">


### Editing Note Start Times and Durations

### Editing Note Velocities

## Menu Commands

* File Menu

    **About Concert keyboardist** ... : Gives copyright information and information about the verision number of this copy of Concert Keyboardist.

    **Open**... : Open a file. Either normal midi files, extension .mid, or Concert Keyboardist files can be opened.  A CK file also has an extension of .mid but has [ck] added just before the extension.

    **Open Recent File** : The ten most recently opened files are shown.

    **Save** : Save the current file as a CK file.  If this file was opened a normal midi fileacs, is is converted to a CK file when saved.  The file retains the same tracks as the original midi file but has some extra tracks containing extra information as system exclusive blocks.

    **Save As**... : Brings up a prompt to save your work as a file with a different name.

    **Audio and Midi Settings**... See [Settings Tools]

    **Tracks**... : Show the tracks in the score.  Tracks can be marked as active or inactive.  Inactive tracks are not displayed and ignored while playing.  This is especially useful for multitimbral music where there are many instruments.

* Edit Menu

    **Undo** : Undo most recent command.  Can be performed repeatedely to undo more than one command.  Keyboard shorcut: Cmd+z.

    **Redo** : Redo the most recent undone command.  Can be performed repeatedely to redo more than one undone command.  Keyboard shortcut: Shift+Cmd+z.

    **Play/Pause** : Ready the score for playing, or stop it playing.  The keyboard shortcut for Play and Pause is the spacebar.

    **Listen** : Listen to the score starting at the note after the Current Time Line.  Press the spacebar to stop.  When listening is stopped the transport is returned to the place where listening started.  You can use this to listen to a passage to get a feel for it before playing it yourself.  Same as the Listen toolbar button.  Keyboard shortcut is "=" (no modifier key).

    Show Edit Toolbar : Toggles the hiding and showing of the edit toolbar.  This is the same as the equivalent toolbar button.

    Hide Measure Lines : Toggles the hiding and showing of the measure and beat lines on the score.  The reason you may want to do this
    is that most midi files that were recorded by a live performer do not have their notes aligned to the beat or measure lines.  They were 
    simply recorded at some constant tempo with the played note occuring in no particular relatinship to the measures of the nominal tempo.  You can identify a midi file like this when the notes seem to bear no relationship to the beats and measure.  The numerical tempo is also almost
    always constant at some value like 120 beats per minute.  For files like this it's usually better to hide the beat and measure lines to 
    remove clutter and avoid confusion at you play, where you might be expecting the lines to be relevant.  If you turn off beat and 
    measure lines this property is saved with the [CK].mid file and retained next time it is opened.


## Licenses and Copyrights

### Concert Keyboardist License

Copyright (C) 2017 Christopher Graham (chrisgr99@gmail.com)

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


### VST PlugIn Technology by Steinberg Media Technologies

<img src="../Common/img/VSTLogoBlack.jpg" alt="VST" style="float:left;width:10%;margin:00 30px 0px 0">

VST is a trademark and software of Steinberg Media Technologies GmbH
