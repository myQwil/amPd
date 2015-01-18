amPd
=====
Winamp Pure Data plugin
-----------------------
This plugin allows Winamp to play pure data files.

To install:
-----------
- put in_pd.dll into "C:\Program Files (x86)\Winamp\Plugins"
- if you want the test patches to work, take the "Common Files\Pd" folder and add it to "C:\Program Files (x86)\Common Files"

To use in_pd in your own patches, Add [r play] and [r vol] to your patch  
[r play] is sent a bang, which is used to start the patch  
[r vol]  is sent a float value of 1, which is used to turn the volume on

Adding tags:
------------
in_pd looks for artist, title, and length info. Adding this info to your patch is a simple matter of adding comments in pure data.

The format is as follows:
- artist : yourArtistName
- title : yourSongTitle
- length : lengthInMilliseconds

make sure you separate the field name from the value with " : " (space-colon-space)

Using Winamp's Equalizer
------------------------
Winamp's equalizer sends the following values to pure-data:
- eq
	- the On/Off switch (0-1)
- pre
	- the Preamp slider (0-63)
- eq0-eq9
	- the frequency sliders (0-63)