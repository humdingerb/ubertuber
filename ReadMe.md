![UberTuber icon](./images/ubertuber_icon_64.png) **UberTuber** for [Haiku](http://www.haiku-os.org)

* * *

UberTuber grabs clips from various sites and opens them in your media player while they're being downloaded. This is as close to streaming as it gets until Haiku's MediaKit grows real streaming support.

The actual work is done in the back-end, using the Python script `youtube-dl` which downloads the video file to your temporary folder. From there it is either played back while it gets downloaded or moved to another location you determine in a file dialog.

UberTuber monitors the system clipboard for URLs pointing to the supported websites and automatically inserts them into its URL text field. Now you just have to decide to either play or save the clip to start the download.

If the playback of a clip from a particular website works, depends on the underlying `youtube-dl` script, which gets updated from time to time. At this moment there is support for clips from:
```
_YouTube, Google video, MetaCafe, DailyMotion, Yahoo video,  
 Photobucket, DepositFiles, Vimeo.com, blip.tv, Facebook video,  
 Escapist magazine, and many many more sites..._
```

Youtube-dl's website has a list of [reportedly working sites](http://rg3.github.io/youtube-dl/supportedsites.html).

### Settings

There are only very few user settings:

![UberTuber window](https://dl.dropboxusercontent.com/u/21023348/images/ubertuber.jpg)

*   **Auto-play**  
     This starts playback automatically, if a valid URL is detected in the clipboard. Convenient if you run UberTuber in the background while surfing the web: just copy the link to a clip in the browser and a few seconds later your media player starts playing it back.

*   **Remove temporary files on quit**  
     The temporary folder is emptied when you shutdown Haiku. If you watch lots of long clips, disk space may become a concern. Check this option to have all temporary files deleted as soon as you quit UberTuber.

*   **Edit monitored URLs...**  
     UberTuber monitors the system clipboard and automatically puts a URL it recognizes into its URL text box. With this menu item, you can change or add to this list of 'known' websites. It opens the settings file `~/config/settings/UberTuber_monitored_sites` in your text editor. Just put the distinguishing part of a site's URL in a new line and save the changes. UberTuber will immediately start recognizing the new address.

### Tips & Tricks

*   **How can I play back embedded videos?**  
     In WebPositive, right-click on the black box of the embedded video and _Open frame in new window_. Now copy the URL and leave the rest to UberTuber's clipboard monitor.

*   **Can I use bookmarks?**  
     UberTuber accepts a drag & dropped bookmark file (in fact any file with a META:url attribute). You can build up a library of video bookmarks while surfing with WebPositive, put them all in a folder and drag & drop one of them onto UberTuber.  
     Or you can select all your video bookmarks and use Tracker's add-on _FileType_ to set UberTuber as their preferred application. Then you can start those video bookmarks in UberTuber with a doubleclick.

*   **What about starting from Terminal?**  
     When started from the Terminal, UberTuber accepts currently only one argument: either the path to a bookmark file or the URL to a clip.

## Troubleshooting

*   **Copied URLs won't automatically show up in UberTuber.**  
     The URL might fall through UberTuber's very simple test if it's from a supported site. You can add new URL-patterns or edit/remove existing patterns of that clipboard monitor, see the _Edit monitored websites..._ menu item mentioned above.

*   **UberTuber won't play/download clips from site X.**  
     Probably the website isn't supported by the `youtube-dl` script. Your only hope is that it has been added recently. Check the youtube-dl package for updates.

*   **Sometimes I get a General OS error.**  
     This seems to be MediaPlayer's way to say that there hasn't been enough of the clip downloaded yet. It depends on the clip format and the speed of your internet connection. You can try closing the empty MediaPlayer window that pops up after the "General OS error" and press UberTuber's "Play" button once more.  
     The alternative is to increase the buffering time a few seconds, but that would increase the time everyone has to wait for every clip... Email me, if it happens untolerably often to you and I may consider adding an option for the buffering.

*   **Sometimes UberTuber just sits there, buffering or whatever.**  
     UberTuber is a bit of a hack as it tries to juggle the youtube-dl script and playback in MediaPlayer, while handling a possible aborting of these processes or saving the clip after playback has started. Add to that, that I'm a very unexperienced amateur "coder"... we're lucky it works at all most of the time.  
     If things get out of control, you may find several stray threads of the scripts running in the background after UberTuber was quit. You can find and kill these threads with the ProcessController replicant of the Deskbar. Hint: the name of the threads contain "UberTuber" or "youtube-dl" etc. Yes, it's messy...

### Download

UberTuber is directly available through HaikuDepot from the HaikuPorts repository. You can also build it yourself using [Haikuporter](https://github.com/haikuports). The source is hosted at [GitHub](https://github.com/humdingerb/ubertuber).


### Bugreports and Feedback

Please use sourceforge's [bugreport form](http://sourceforge.net/projects/ubertuber/support?source=navbar) if you experience unusual difficulties or email your general feedback to [me](mailto:humdingerb@gmail.com). Also, email me if you'd like to provide more localizations.

### Thanks

I have to thank Leszek Lesner for providing his MIT licensed code for [YAVTD](http://www.zevenos.com/about/yavtd) which provided ideas and some lines of code for UberTuber.  
 Also thanks to Duggan from the haiku-3rdparty IRC channel for hints and help to find crashing set-backs.  
 Thanks to everyone that provided translations vor UberTuber.  
 And finally thanks to the people providing the [youtube-dl](http://rg3.github.com/youtube-dl) script.

If you kinda like UberTuber – or even more so if you don't – consider a [donation to Haiku Inc.](http://www.haiku-inc.org/donations.html#online) or even better small monthly donations. This will accelerate Haiku development and may even help perfecting video playback into WebPositive, so UberTuber can eventually retire.

### History

**0.9.0** - _24-09-2011:_

*   Initial release.

**0.9.1** - _30-09-2011:_

*   Forcing a MIME type video/mpeg4 when playing some clip. That way playback of some MP4 clips from sites like vimeo works, where before the media player complained it didn't know the format.  
     After playback the system does a mimeset -F on the clip to set the correct MIME type or going back to the generic appliction/octet-stream.

**0.9.2** - _07-12-2011:_

*   The forcing of the MIME type to video/mpeg4 didn't always work. We now just set this MIME type with any file, not just octet-streams. We also buffer buffer 2 seconds longer for good measure.  
     Aborting/killing the download should now work a bit better. Depending on length of the path to the UberTuber's installation folder, it didn't work at all before...

**0.9.3** - _17-11-2012:_

*   Added "youtu.be, blip.tv, escapistmagazine.com" to the clipboard monitor.
*   Fixed a bug that had "vimeo" URLs not auto-inserted from the clipboard.
*   Add META:url attribute with the download source URL also to the temporary file in /tmp/ubertuber.
*   Removed the need for youtube-dl.patch.
*   Truncate clip title if too long, don't resize the window anymore.
*   Fixed a small issue in the logic of the abort button.
*   Check if Python is installed and offer to install it if it's not.
*   Jump through another hoop to get thevideo title of newer versions of the youtube-dl script.

**0.9.4** - _24-11-2012:_

*   We now check at the start if Python is installed, instead when trying to download a clip.
*   Added the status "Installing Python" and "Installation finished" as feedback.
*   We should now correctly determine when MediaPlayer was closed.

**0.9.5** - _20-04-2013:_

*   Removed the need for git to update the youtube-dl script.
*   Added "youtube/embed" and "youtube-nocookie.com" to clipboard monitor. Now you can open embedded videos in a new tab from WebPositive's context menu and copy the URL from that new tab.
*   UberTuber is now hosted at http://sourceforge.net/p/ubertuber.

**0.9.6** - _14-10-2013:_

*   Removed the check for Python. Not needed anymore with Haiku's package management.
*   Adapted paths to new PM file system layout.
*   Better handling of filenames with special characters, e.g. /?:|  
     Still not perfect, I guess, but better...

**0.9.7** - _20-10-2013:_

*   The special character business doesn't seem to always solvable. Therefore I just take the restricted ASCII filename from now on. Not as beautiful, but it seems to work much better. Also set --max-quality=22 of youtube-dl, which seems to get the highest quality mp4 instead of downloading a flv instead...

**0.9.8** - _21-12-2013:_

*   Removed youtube-dl from UberTuber package and declare that as dependancy.
*   Removed menu to update youtube-dl. It always was a security hole and now that the package managment is enforcing read-only locations for packages, it would take too many hoops to jump through to maintain this security nightmare...
*   Simplify code, making use of ps' new -o option. Simplify tests for running thread name.

**0.9.9** - _20-02-2014:_

*   Keep the GUI responsive while getting the clip title. The GUI used to be frozen in the first few seconds of downloading a clip.
*   Simplified code a bit by avoiding youtube-dl creating .part files
*   The clipboard monitored URLs are now stored in a node monitored settings file `~/config/settings/UberTuber_monitored_sites`.
*   Added a menu item to edit the UberTuber_monitored_sites settings file in the text editor.
*   Added a menu item to clear the text from the URL field.
*   Added a menu item to open the current URL in the browser.
*   Added a "Select this folder" button to the save panel.
*   Accept dropped bookmark files (or any file with a META:url attribute).
*   Register bookmark files as supported types of UberTuber, enabling opening "video" bookmarks of WebPositive with UberTuber.
*   Accept a URL or a bookmark as argument on the command line.

**0.9.10** - _24-02-2014:_

*   Always make sure /tmp/ubertuber exists or there may be race conditions.
*   Try to kill any zombi threads when aborting or exiting UberTuber.

**0.9.11** - _15-06-2014:_

*   Localizations for German, Polish.
*   Use "--format best" as youtube-dl's "--max-quality" is deprecated.
