 -------------
/NeoCDPSP 0.5/
-------------

+++++++++++++++++++++++++++++
[INTRO]
--------------------------->

NeoCDPSP is based on :

Neo4ALL, an "Alternative Open Source NEOGEO/CD emulator for the Dreamcast console by Chui & Fox68k".
--> http://chui.dcemu.co.uk/neo4all.shtml

Ruka's psp emulators, for valuable sourcecode (learned a lot from it!).
--> http://rukapsp.hp.infoseek.co.jp/ 

Previous neocd psp emulator, NeoCD/PSP.
--> http://psp-dev.org/pukiwiki/index.php?NEOGEO%20CD

unofficial PSPSDK.
--> http://www.ps2dev.org

GFX from Pochi (psp background, icon, loading)
-->http://pochistyle.pspwire.net/

GFX from SNK (file selector & menu background)

+++++++++++++++++++++++++++++
[FEATURES]
--------------------------->
- neogeo cd emulation
- 2 gfx engine : software of hardware (using psp's gu)
- autofire
- memory card emulation
- zipped/unzipped game
- sound support
- music support (with mp3 tracks, using libmad)
- mutiples rendering mode with hardware stretching (thx to pspsdk & chp great work!)
- 222/266/333 Mhz

+++++++++++++++++++++++++++++
[INSTALL]
--------------------------->

/PSP/GAME/NEOCDPSP/
(or any others directory since v0.5)
                  + EBOOT.PBP
                  + STARTUP.BIN
                  + loading.bmp
                  + logo.bmp
                  + NEOCD.BIN (BIOS)
           
Games can be zipped or unzipped.
Music in mp3 format have to be in a "neocd.mp3" subfolder.

Example :

/PSP/GAME/NEOCDPSP/MetalSLug/
                            + mslug.zip
                            + neocd.mp3/
                                       + Metal Slug - Track 02.mp3
                                       + Metal Slug - Track 03.mp3           
                                       + Metal Slug - Track 04.mp3
                                       ....
                                       + Metal Slug - Track 20.mp3

You can uncompress the zip file for faster loading time.
MP3 Tracks have to finish by the "tracker number.mp3" 

=> "mslug-02.mp3" is good
=> "02-mslug.mp3" is wrong

+++++++++++++++++++++++++++++
[PLAY]
--------------------------->

PSP           NEOGEO
pad,analog    pad
CROSS         A
CIRCLE        B
SQUARE        C
TRIANGLE      D
START         START
SELECT        SELECT
LEFT TRIGGER  MENU
RIGHT TRIGGER SNAPSHOT

+++++++++++++++++++++++++++++
[HISTORY]

-
0.5
* Music (mp3) playback bug in pause/play fixed (Last Resort seems ok now).
* Rendering fixed (sprite disappearing)
* New render using psp hardware 
* Improved GUI
* New feature : autofire for A,B,C,D neogeo buttons
* Changed memory io and z80 emulation with NeoDC 2.3 sources (music in more games)
* New GFX from great pochi! http://pochiicon.hp.infoseek.co.jp/
* Screenshot (press R trigger), saved in uncompressed 24bits BMP (480x272)
* Directory independant, you only have to put the whole files & bios in the same dir
(no "/PSP/GAME/NEOCDPSP" directory only anymore)
-
0.1 - 2005/07/07 (first release)
-

+++++++++++++++++++++++++++++
[COMING NEXT]

* load/save state
* 

+++++++++++++++++++++++++++++
[KNOWN BUGS]

* emulation isn't perfect (lack of technical documentation for the neogeo cd)
-> so no sound/no music in some games
-> sometimes music is stopped when putting game in pause (pressing select/start) and 
not resumed when resuming game.
* some games won't work at all

* If you want this to change, you'll have to find me neogeo cd technical documentations
or an emulator which does the job better with sourcecode available ;-)

+++++++++++++++++++++++++++++
[EXIT]
--------------------------->

Have fun!

------------------>
http://yoyofr.fr.st
----->

yoyofr