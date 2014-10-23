in_pd
=====

Winamp Pure Data plugin

This plugin allows Winamp to play pure data files.

To install:
- put in_pd.dll into "C:\Program Files (x86)\Winamp\Plugins"
- if you want the test patches to work, take the "Common Files\Pd" folder and add it to "C:\Program Files (x86)\Common Files"

Your patch just needs [r play], which sends a bang to start the song, and [r vol], which sends the float 1 to volume. Or if you prefer, you can just set your patch up to start playing upon loading.
