JGMOD library 0.23
5 April 1998

----------------
What is JGMOD ??
----------------
JGMOD is a mod library for MOD musics. It is built on top of Allegro lower
level digital sound routines. Currently supports MOD and S3M.

I am doing this library for 4 main reasons.

1.  For the sake of learning. I always wanted to learn how to do a mod
    player.
2.  I am tired of using Mikmod. I have to initialisz a lot of things
    just to make speakers rock.
3.  There are no mod library specifically for Allegro yet. 
4.  It is kinda great to create a MOD library. :-)


---------
Copyright
---------
JGMOD is a free mod library. Also it requires you to register at the url
below. You can register anytime when you are using JGMOD. If you have
any suggestion, comments or whatever, please include it too. 

You can do whatever you like to my codes. Modify, add, delete or whatever.
However, I prefer the codes to be molested. Beware. My codes is also known
to molest many people. :-)

Register at the URL below :-
http://www.geocities.com/siliconvalley/vista/8890/jgmod_sign.html


----------
Disclaimer
----------
I will not be responsible to any destruction or damage my library does to
you or your computer. 


------------------------------
Protracker effects unsupported
------------------------------
Arpeggio
Tremolo
Glissando
Set tremolo waveform


-----------------------
S3M effects unsupported
-----------------------
J   (Arpeggio)
Q   (Retrig + Volume Slide)
R   (Tremolo)
S1  (Glissando)
S4  (Set tremolo waveform)



----------------------
U S I N G    J G M O D
----------------------

int fast_loading;
    This variable affects only S3M loader. If FALSE, the S3M loader will use
    a method to detect the no of channels used by the music accurately but
    SLOW. If TRUE, the channels detection is less accurate but MUCH FASTER.
    Speed difference between these two settings depending on the speed of
    computer and cache memory. Try these two settings and decide yourself
    which is better. 


int enable_m15;
    When load_mod is called, it will detect the correct type of mod
    (eg. MOD, S3M, XM) and will call the appropriate loader. 15 instruments
    protracker uses a different detection method than 31 instruments
    protracker. Sometimes, the 15 insturments detector might detect wrongly.
    By default, it has the value of FALSE which means the detector is
    disabled. To enable, set this variable to TRUE.
    

void reserve_voices(int digi_voices, int midi_voices);
    This is an Allegro function to specify the number of voices that are to
    be used by the digital and MIDI sound drivers respectively. This library
    uses the digital sound drivers. So sending wantever values for the MIDI
    will not intefere with JGMOD library. Must be done BEFORE calling
    install_sound() function. 32 voices is the maximum. Allegro documentation
    states that audio quality will suffer when more digi_voices are used.
    This is very true for 8bit output. 

int install_mod (int no_voices)
    Call this to allocate no of channels (or voices) for the MOD. Returns
    a positive number if sucessful or negative if unsucessful. Do not try
    do allocate more than you reserve. You will hear some missing channels.
    If you need to play some digital soundfx, allocate even lesser voices.
    Remember to install timer or there will be no output to the speaker and
    call this function AFTER install_sound().


JGMOD *load_mod (uchar *filename)
    This function will detect the type of MOD and will call the appropriate
    function to load it. Able to load MOD 15 and 31 instruments and
    S3M successfully. XM is on the way but I have trouble understanding
    the official documentation. Currently, this library will not load any
    MOD from the Allegro datafile.


void play_mod (JGMOD *j, int loop)
    Play the mod j. Pass FALSE if you don't want to loop the music. TRUE
    to loop the music.


void stop_mod (void)
    Stop the mod from playing.


void next_mod_track (void)
    Jump to the next pattern. If no next pattern, then the music will end.


void prev_mod_track (void)
    Jump to the previous pattern. If no previous pattern, then restart
    the current pattern.


void goto_mod_track (int new_track)
    To jump directly to a new track.


void pause_mod (void)
    Pause the currently playing mod.


void resume_mod (void)
    Resume a paused mod.    


int is_mod_playing (void)
    Use this function to check if any mod is currently playing. Returns
    TRUE if a mod is playing. Else, returns FALSE


int is_mod_paused (void)
    Check for any currently playing mod is paused. If paused, returns TRUE.
    If not pause, returns FALSE. If not playing any music, returns FALSE. 


void destroy_mod (JGMOD *j)
    Frees a JGMOD structure and then set it to NULL.

void set_mod_volume (int volume)
    Set the mod volume. Range of 0-255.

int get_mod_volume (void)
    Returns the mod volume.



Compiling
---------
Include jgmod.h after including allegro.h. For example,

#include <allegro.h>
#include <jgmod.h>


Make sure you include jgmod.h after allegro.h. Otherwise you will get a
error message when compiling. When linking, link libjgmod.a before liballeg.a.
For example,

-ljgmod -lalleg

If you are using Rhide, go to the Options/Libraries menu, type 'jgmod' 
into an empty space which is above alleg, and make sure the box next to it
is checked.


About JGMOD.EXE
---------------
I use this program to test the musics that I have. You should see the numbers
below similar in jgmod.exe. Well, let me explain what these numbers are.

1:  4  1 15678Hz 255
2:  2 11 18363Hz   0
3: 60 32 32567Hz 127
4: -- -- -----Hz ---
5: 45 64 45678Hz  34
6:  7 34   911Hz 204

Column 1 : Channel no.
Column 2 : Instrument it is currently playing beginning with 1 as the first
           instrument.
Column 3 : Volume. 0 to 64 scale.
Column 4 : Frequency the insturment is being played. In herz.
Column 5 : Pan.  0, left most panning. 255, right most panning. 127, center
           panning.

Keys
----
P           : Pause/unpause music
+           : Increase volume
-           : Decrease volume
Left        : Previous pattern
Right       : Next pattern
Space / Esc : Exit


Problems found
--------------
Since it uses Allegro Digital Routines, whenever set_volume is called,
the volume of the mod music will be affected. You can simulate a software
volume mixing easily. Just don't use set_volume. I will demonstrate this
in the  future.


Words from me
-------------
Thanks for downloading this library. If you have any questions you want to
ask, bug reports or just want to say something, just send a mail to
me (JGFW@USA.NET).

I just got myself a modem. So, anyone who wish to contact me, I could
reply now in less than a day.

You can send the mail to Allegro Mailing list too. If you do, please
put a MOD capitalized as the subject just to catch my attention. 
I will only read interesting mails. The rest, I just skip and then delete
it. So, I might accidently skip yours.

Once I have done XM, I will make a general format just like Mikmod's Unimod.
This general format can be loaded from Allegro Datafile without any problems.


Thanks to
---------
Pedro Cardoso
    For his help and codes for S3M panning commands

Spiritseeker   
    For telling me that fmoddoc2.zip exists. Without that document, I would
    still be confused about S3M now. 

Jean-Paul Mikkers
    For creating Mikmod. You have never answered any of my mails. I don't
    know why. I got some ideas on loading mod from your codes. So thanks
    anyway

Jake Stine
    For maintaining Mikmod 3.xx. You have improved the codes for loading
    mods over Mikmod 2.10. It is sad to know that you don't wish to maintain
    Mikmod anymore. Maybe because you are now interestd in playing QUAKE
    (If anyone doesn't believe what I am saying, just go to
    www.divent.simplenet.com and check it out yourself).


Contact Info
------------
Email   :           jgfw@usa.net

Snail Mail :        18, SS 17 / 1H
                    47500 Subang Jaya
                    Selangor
                    Malaysia.

ICQ number :        9712677. My user name is jgfw. 

Mailing list:       allegro@canvaslink.com. To add or remove yourself, 
                    write to listserv@canvaslink.com with the text 
                    "subscribe allegro yourname" or "unsubscribe 
                    allegro" in the body of your message.


You can always get the latest JGMOD update at surf.to/jgmod or
www.geocities.com/siliconvalley/vista/8890. If you have somehow can't
remember the addresses, just go to anypage which have allegro webring
(eg. Allegro homepage) and click on the home link. You should reach my page.
