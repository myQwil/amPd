in_pd
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