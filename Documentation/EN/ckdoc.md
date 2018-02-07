
# Concert Keyboardist User's Manual

### Version and Licensing

* Document Version: January 24, 2018 for Concert Keyboardist Alpha
* Concert Keyboardist Licensing: [Licenses and Copyrights]

## Contact Information

For questions or comments contact: chrisgr99@gmail.com

## Quick Introduction to Concert Keyboardist

Concert Keyboardist is like an easy to learn expressive musical instrument that never lets you play a wrong note.  Load a midi file and it becomes an  instrument for performing a particular piece of music where you control in real time when and how notes are played.  It's magical to play like a "Concert Keyboardist" on your computer keyboard.   Or use a midi keyboard for even more expressiveness.

Where to get midi files to play?  There are tens of thousands available around the Internet for download or you can create your own.  Before Concert Keyboardist a replayed midi file was always exactly the same and could sound mechanical.  With Concert Keyboardist it comes alive under your fingers as you play.

## This is Concert Keyboardist

<iframe width="560" height="315" src="https://www.youtube.com/embed/xb66fzwdDS4?rel=0" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

<img src="../Common/img/Pathetique.jpg" alt="CK" style="width:100%;margin:20px 0px 10px 0px">

**How it works**:  In the above image the heads of the "Target Notes" have magenta heads and "Chained" notes have white or grey heads. Each time you press a key (any key) Concert Keyboardist triggers the next Target Note in the score.  It also schedules any notes Chained off the target note for autoplaying according to the tempo.

Then when you press the next key (any key) Concert Keyboardist triggers the next target note and schedules its chained notes, and so on.  If you trigger another target note before the previous chained notes have all been played, the new target note and its chained notes are added to the currently playing chain.

As you play the score scrolls right-to-left based on tempos set in the score's midi file.  You control when to play target notes and how long to hold them.  Concert keyboardist continually keeps you visually informed on played notes, sounding notes, upcoming unplayed target notes, suggested times to play them, and the tempo.  This carefully designed visual feedback is crucial to the ability to play a midi file live, and more than two years of testing went into perfecting it.

Played with a computer keyboard Concert Keyboardist uses note velocities from the score.  Played with a velocity sensitive midi keyboard Concert Keyboardist uses velocities from the keyboard for the target notes, and velocities of chained notes are adjusted proportional to the velocity of their target note.

Concert Keyboardist gradually adjusts its tempo (rate of scrolling) toward your playing tempo.  You are not locked to the tempo in the midi file.

Concert Keyboardist has tools to edit things like note velocities, note start times, note durations, and variations in the  tempo.  You can save your edited scores in Concert Keyboardist files, which are normal midi files but with an extra invisible track for your edits.

However, it's important to understand that in this version of Concert keyboardist it's not possible to create scores or add and remove notes.  There are many powerful applications for creating and manipulating midi files, but Concert Keyboardist is unique in its ability to allow you to "perform" them under real time control.

## Installation and Setup on a Macintosh

Concert Keyboardist is compatible with Mac OS 10.6 or higher. To install: 

* Copy the Concert keyboardist application to your Applications folder.  
* Copy the included folder of example files to anywhere you choose.  Possibly your Documents folder, Desktop, or a folder where you keep midi files. 
* Concert keyboardist can produce sound either by loading a VST or Audio Unit plugin, or by sending midi to another application.  
* If you are familiar with plugins and have any on your computer, load a suitable one (e.g. piano) as described in the Plugin Management Tools section below, and configure the Concert Keyboardist audio output as described in the Audio Settings [Settings Tools] topic below.  See To use an external application you need to route theme to a software synthesizer (or hardware synthesizer).  The easiest option on a Macintosh is to use GarageBand which is free and has a wide range of sounds to choose from.  Also GarageBand automatically receives midi messages from all midi output ports on your system so it automatically receives from the Concert Keyboardist default output.
* If you prefer to use an external midi software or hardware synthesizer
* Setup with GarageBand: 
    * Download and install GarageBand (it's free to Mac owners).  
    * Open GarageBand.
    * When it opens choose "Empty Project"
    * Choose "Software Instrument".  It should default to Classic Electric Piano.
    * You could play the Classic Electric Piano but it's better to use a true piano sound for the piano music included with Concert Keyboardist.
    * To choose a piano sound click on "Sounds" in the frame on the left, click on "Piano" and double click on "Steinway Grand Piano".
    * Switch to Concert Keyboardist and follow the Quick Start to Playing, below.  When you play you should hear piano notes.
* If you are more experienced with midi you should know how to configure your preferred sound generator to receive from the Concert Keyboardist default output.
* If you have a midi keyboard attached to your computer you can configure Concert Keyboardist to receive from it.  Use the "Audio and Midi Settings" command in the File menu to open the settings dialog.  Click next to your midi controller to make it active and close the dialog.  Concert Keyboardist will remember this setting between sessions and it will remain available as long as your controller is active when Concert Keyboardist is started.

## Installation and Setup on a Windows

Concert Keyboardist is compatible with Windows 7 and Windows 10 (possibly Windows XT but this has not been tested). To install: 

* Copy the Concert keyboardist application to your Program Files folder.  
* Copy the included folder of example files to anywhere you choose.  Possibly your Documents folder, Desktop, or a folder where you keep midi files.
* If you are familiar with VST plugins and have any on your computer, load a suitable one (e.g. piano) as described in the Plugin Management Tools section below, and configure the Concert Keyboardist audio output as described in the Audio Settings section below.
* You can also play by routing midi from Concert keyboardist to a exteral sound generator: a DAW, softsynth or hardware synthesizer.  
    * If your external sound generator has a virtual midi input port choose it as the Concert Keyboardist midi output:  Click on the Audio/Midi Settings tool in the CK toolbar and choose that output.
    * If your external sound generator does not have a virtual midi input port you will need to install a virtual midi routing driver such as [LoopBee](http://www.nerds.de/en/loopbe1.html).  Then choose it as the output to Concert Keyboardist and the input port on your sound generator.
    * In your sound generator choose a suitable sound such as piano.

<!-- ![](../Common/img/CKMainWindow.jpg) -->

<img src="../Common/img/CKMainWindow.jpg" alt="CK" style="float:right;width:75%;margin:0px 10px 10px 10px">

## Main Window

* Concert Keyboardist is like a player piano.  **Notes** are horizontal bars that scroll right to left horizontally based on the current **tempo**.  A note's vertical position shows pitch and its left end shows start time. Its length shows duration.  This information is read from the midi file and guides you when to play notes and how long to hold them.
* In real time while you play **you control note start times and durations**.  This is like playing sheet music.  The notes are there but you add expression as you play.  If you have a velocity sensitive midi keyboard you can also control note velocities.
* The score can be **scrolled** left or right with the mouse wheel or two finger drag on a touch pad.  The end of the score is at the far right.
* The blue vertical line is the **"Current Time Line"** (CTL) which indicates the current playing time in the score.  It turns amber when ready to play and green while you are playing.
* While you are playing, pressing any key instantly triggers the next unplayed **Target Note**, magenta-headed-notes.  The these are the notes you directly play.
* When a Target Note is played it also schedules for playing all notes **chained** from it up to just before the next target note.  Chained notes are those without magenta heads.  
* Chained notes are triggered at appropriate times in the future based on the current tempo.
* After all scheduled chained notes have been played no further notes will sound until you trigger the next target note.
* Although a target note's position suggests its start time, you can play it before or after that time, giving you control over the **flow and rhythm** of the music.  The **tempo** gradually adjusts itself to your playing speed.  If you continually play notes before their scheduled time the tempo speeds up and if you play notes behind the tempo slows.
* If you try to play the next target note while previously chained notes are still playing, that target note and its chained notes are added to the previous chain.
* Each time you trigger a target note a vertical yellow **"Relative Time Line"** is drawn at that exact position as a guide to when to play the next target note.  To play the upcoming target note at the current tempo, play it when it reaches the Relative Time Line.  You are free to play it earlier or later which alters the timing of that note and its chained notes.  i.e. The feel or rhythm of the music.  If you continue playing early or late the tempo gradually slows or speeds up.


## Quick Start to Playing
* **Load a file:**  You can use the File-Open menu, the toolbar button, or Cmd+O.  A good introductory example is "Minuet in G[ck].mid" which is included with Concert Keyboardist.  It should look something like the above image.
* **Prepare to play:**  Press the spacebar.  This turns the Current Time Line orange, indicating that Concert Keyboardist is ready to play.  In this mode as soon as you press a key the note at the CTL is played and the note bars start scrolling to the left.  The scroll rate is based on the tempo.
* **Triggering Notes:** On your computer keyboard press any letter key, or any of the characters "[ ] ; ' < > and /".  On an English language keyboard these are the rows from "q" to "]", from "a" to " ' " and from "z" to "/".  If you are using a midi keyboard playing **any** note triggers the next target note.
* **Note Timing:** To play at the original tempo and rhythm press a key each time a target note reaches the Current Time Line.  You can play with one finger, or two or more fingers to play faster or smoother.  
* **Stopping:** Press the spacebar again, or press the Return key for rewind, or roll the mouse wheel to scroll left or right.
* **Yellow note heads** are those that are currently sounding.  As you play, notice that long notes continue sounding even as you trigger shorter simultaneous notes.
* **Chords:** In the "Minuet in G" example the top notes of chords are target notes.  Simultaneous or nearby chord notes are chained from them, like any other chained notes.  This allows chords to be triggered by one note..
* **Trills** are like any other chained notes.  For example in bar 8 of Minuet in G the notes are chained making them easier to play.
* **Rewind:** Press the Return key.  The first time you press Return the transport rewinds to the place you most recently started.  A second press returns to the start.  You can also press Return while the transport is running to stop and transport and rewind in one step.
* **Basic Editing** You can edit which notes are target notes.  Click on the head a note to toggle its target vs chained status. Setting a note to be chained causes it to be added to the chain of the preceding target note.
* **Playing legato notes** This is a technique where the on and off times of successive notes are overlapped resulting in smooth sounding transitions.  If you play with at least two fingers you can overlap the on and off times of successive notes to create legato as in normal piano playing.  
* **The Relative Time Line:** This is the yellow vertical line that appears at the start of the most recently played target note. The Relative Time Line is a guide to when to play the next target note relative to when you played this note.  In playing expressively you may be triggering notes before or after the Current Time Line.  It's helpful to have a relative time marker showing when notes were actually played, not when they were suggested to be played.  Experiment with playing notes before or after the Current Time Line to understand this better.  The yellow line remains after you stop playing to mark the last note played before stopping.


## Navigating Around The Score

* **Scroll** with the touchpad or mouse wheel.  
* Press the **Return Key** to go where you most recently started playing.  Press a second time to return to the start.
* **The Right and Left arrow keys** step one target note at a time forward or backward in the score.
* **The Shift+Right Arrow and Shift+Left Arrow keys** step one measure at a time forward or backward in the score.
* **Bookmarks:** Cmd+Right Arrow and Cmd+Left Arrow (Ctrl on Windows) step one bookmark at a time forward or backward in the score.
*  Press Cmd+b to create a bookmark at the Current Time Line.  To delete a bookmark press Cmd+b with the bookmark already at the 
Current Time Line.

## Selecting Notes

Many commands act on "selected notes". To select a range of notes move the mouse pointer to anywhere not on a note bar, press the left mouse button and drag over the heads of notes to be selected and release the mouse button. The head of each selected note will be surrounded by a white box. To clear the selection click anywhere there is not a note bar.

To add notes to the selection hold down the shift key while some notes are selected and drag the marquee around other notes.  They will 
also become selected.  

To remove notes from the selection hold down the Option key and drag, starting with some notes selected.  Any selected 
notes you drag the marquee over will become unselected.

If you drag past the right or left end of the score the score will autoscroll and stop if it reaches that end of the score.

To select all notes in the score use the keyboard shortcut Cmd+a on Mac or Ctrl+a on Windows.  Press the Escape key to unselect all notes.

You can't select a single note by clicking directly on its head.  This toggles it between being a target or non target note.  To select a 
single note, start on the background near it and drag a marquee around it.

## Note Information Viewer 

<img src="../Common/img/Information Area.jpg" alt="CK" style="width:30%;margin:0px 0px 0px 10px">

The Note Information Viewer is in the lower left corner of the main window.  Hovering the mouse pointer over a note bar shows details on the note.Hovering over a light or dark grey note track reveals the note name and octave of that track.  Dragging on the display to select 
a range of notes shows details of the selection: The time in ticks of its first and last note and the duration in ticks from the first to the last note.

## Main Toolbar

### File Management Tools

<img src="../Common/img/FileManagementTools.jpg" alt="CK" style="width:9%;margin:0px 0px 0px 10px">

There are buttons for "File Open", "File Save" and "File Save As".  These do the same as the "Open", "Save" and 
"Save As" commands in the File menu, and are very similar to commands you are probably know from other applications.

File Open will open either normal midi files (extension .mid) or Concert Keyboardist files (extension [CK].mid).  Opening a normal midi file automatically imports it.  

When you save a file, it becomes a Concert keyboardist file and the [CK] marker is added before the extension.  Because it's also a normal midi files you should be able to load it into any other application that loads midi files.  

If you re-save a [CK].mid file from another application Concert Keyboardist may or may not retain any customizations.  These are in an extra track encoded in "system exclusive" format.  Whether another application retains sysex tracks depends on the application.

### Plugin Management Tools

<img src="../Common/img/PluginManagementTools.jpg" alt="CK" style="width:6%;margin:0px 5px 0px 25px">

There are two plugin related tools:

* The first tool reveals a menu where you can choose a plugin to load, a command to scan for plugins, and a command to unload the current plugin, if needed.  You can load VST, VST3 plugins on Mac or Windows, and Audio Unit plugins on Mac.  Only instrument plugins can be loaded, and only one at a time.
* The second tool is for opening the current plugin's window.

### Settings Tools

<img src="../Common/img/SettingsTools.jpg" alt="CK" style="width:6%;margin:0px 0px 0px 25px">  There are two settings tools:

* **Audio Settings**
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
* Normally this will change the tempo everywhere in the score keeping the tempo everywhere to be the 
same ratio TBD
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

    **Audio and Midi Settings**... Choose the midi input and output channels.  The audio settings are currently not used.  On Macintosh CK also automatically creates input and output midi ports called ConcertKeyboardist that can used used by other software such as a Digital Audio Workstation (DAW or softsynth) to receive midi from Concert keyboardist.

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
