
                                  MAY FIRST
         A game inspired by  http://en.wikipedia.org/wiki/Juno_First
                 Written 2013 by hmw, http://harald.ist.org/

Index
  1. Introduction
  2. Gameplay
  3. Building from source
  4. Mode of operation
  5. Change log
  6. Beta testing questionaire
  7. TODO


*******************************************************************************
* 1. INTRODUCTION (alpha 0.2.0)
*******************************************************************************

The model for this game, Juno First, is a real master piece and a classic.
I immediately fell in love with this game for reasons like how certain effects
are achieved with almost no effort, or the speed rush one experiences when
getting into a firing frenzy, which somehow works extremly well in Juno First.
Gameplay of Juno First: http://www.youtube.com/watch?v=vWB-GiSElV4

Apropos frenzy, May First got its basic shape in two days, when I had an
accute programming attack. The following weeks of work mainly consisted of
removing ugly hacks and heavy refactoring. The game is quite playable now,
but there is a major flaw:

The OpenGL routine is written very inefficiently, it needs to be rewritten
from scratch. Especially the grid drawing function is bad; I had to add a
CHEAT_OFFSET_Y setting in order to manually adjust the calculation in the
routine that marks every "Distance"-unit and when calculating black-hole
related things.


*******************************************************************************
* 2. GAMEPLAY (alpha 0.2.3)
*******************************************************************************

The player is confronted with an armada of alien tetraeder shaped space
vessels. Killed enemies drop "Resource" for the player to collect. Everything
is payed from this single resource, from weapon fire to absorbing hits.
The first Resource bubble of a kind will unlock a weapon upgrade according to
the tier (color) of the enemy:

LASER Beam - Always available - Costs 1 unit of Resource per shot.

Auto Fire - Dropped by BLUE enemies
  Hold any weapon control to initiate a burst.

Double Shot - Dropped by GREEN enemies - Costs 4 units of Resource.

Triple Shot - Costs 5 units of Resource
  Press the keys for main and double fire at the same time.

Round Shot - Dropped by RED enemies - Costs a lot.
  Fires a range of beams in all directions.

If enemies pass by the player's ship, they will re-appear a short while later
on the "field" until they are destroyed or the game ends.

Avoid the "black holes", they will drain your Resource quickly. If you get
too near to the center, the game will end.

If Resource is low (indicator blinks), it is a good idea to aim properly and
go for the small enemies (blue or green) in order to regain Resource. Aim to
keep your Resource "green".

If the ship gets hit, Resource is halved until it is below 50.
Taking a hit with Resource below 50 leads to death and the end of the game.

LEVEL DESIGN
------------
In version 0.2.3, a new procedural level design routine has been added. It is
still somewhat boring, because the waves look similar except for the amount of
enemies. Better level design is in preparation.


*******************************************************************************
* 3. BUILDING FROM SOURCE (alpha 0.2.4)
*******************************************************************************

You may need to install a few packages before you can compile the program.
Under Debian 7.1 I had to issue:
  $ sudo apt-get install \
    libsdl-image1.2-dev  \
    libsdl-ttf2.0-dev    \
    libsdl-mixer1.2-dev  \
    libsdl-gfx1.2-dev

Under Fedora 20 I used:
  $ su -c 'yum install  \
    SDL_image-devel     \
    SDL_ttf-devel       \
    SDL_mixer-devel     \
    SDL_gfx-devel

Mac:
  $ brew install \
    sdl_gfx      \
    sdl_image    \
    sdl_mixer    \
   sdl_rtf       \
    sdl_sound    \
    sdl_ttf

  Disable DEBUG (main.h) and PLAY_MUSIC (ui.h). What is  sdl_rtf ?


Compile the program with:

  $ make clean && make && mv a.out may_first

Start the program:

  $ ./may_first

If the linker should complain about the package "SDL_ttf" not being found in
the pkg-config path, this helped me:

  cp ./SDL_ttf.pc /usr/share/pkgconfig


*******************************************************************************
* 4. MODE OF OPERATION (alpha0.1.13)
*******************************************************************************

May First is a state machine, using two structs to store the current state:

- program_state (PS) Holds times and user interface related variables.
- game_state (GS)    Keeps track of everyting related to the game itself.

The program starts in PS->run_mode = RM_INTRO. As long as this run mode is set,
a title screen is showing. After a key press, the program switches to
RM_RUNNING, starting a game. When the player dies, the run_mode RM_AFTER_LIFE
is activated in order to let some animations continue for a short while
(e.g. the explosion of the ship), then RM_MAIN_MENU is activated. After the
main menu has been shown for some time, the program switches back to RM_INTRO.

IMPORTANT FILES
---------------
main.h     Program state, control settings, types
ui.h       UI related settings (Full screen mode, sound related options)
presets.h  Game state control and detail settings
game.h     "Super" struct  game_state_t .

FILE HIERARCHY
--------------
main.c            Main loop handles frame rate, triggers simulation and rendering
ui.c              Sets up the window, handles input, sounds
draw_frame.c      Every tick, the screen is redrawn according to  PS->run_mode :
  intro.c         Intro screen showing the credits
  scene.c         Draws the 3D part of the game screen
  hud.c           Draws the overlay (Score, time, menus, etc.)
  gl_helpers.c    OpenGL related aiding functions (load_texture, gl_printf, ...)
game.c            Provides main game control and some aiding functions
  level_design.c  Level control, functions that define game rules
  world.c         Advances the world simulation by one step
  player.c        Player specific routines (Taking hits, controls, ...)
  enemy.c         Enemy specific routines (Taking hits, AI, ...)
  formation.c     Organizes enemies into formations and provides controls


*******************************************************************************
* 5. CHANGE LOG
*******************************************************************************

v0.2.3
- Added computer announcements when weapon upgrades are installed or lost
- Upgrades now obtained earlier (from lower tier enemies)


*******************************************************************************
* 6. BETA TESTING QUESTIONAIRE
*******************************************************************************

- OS, GPU
- Installation protocol
- Gameplay: Difficulty, Progress/Curve, "Frenzy-factor"
- Technical issues and bugs


*******************************************************************************
* 7. TODO
*******************************************************************************

// IDEAS //////////////////////////////////////////////////////////////////////
? Smaller enemies run out of energy faster, dock with mothership to refuel
? Color-combo ("chain bonus")
? Enemies: stick together to form larger structures, "bonds" have hitpoints
? Enemies: drop only items they actually used
? Enemies: may drop the best item they had
? Multiple lifes, bonus life at ?resource peak
? Powerup: Freeze enemies
? Limit single shots, but make them much cheaper than auto fire
? SPEED_Y increasing over distance? Bonus for not using extra weapons
? Weapon upgrades represented by "Cargo" box
? HUD optional (TAB, animation for sliding in and out)
? Save-points
? Homing device, can also be fired upon
? Momentum, ship slowly rotating into new attitude
? Explosions may trigger further explosions, if ships nearby
? Particle effect for ship rocket
? Easter Egg
? Ricochets
? RunModes --> game_state
? Black Holes suck in enemies and Lasers, distance markers
? Black Holes refract lasers
\ Re-enable F11 (Toggle Fullscreen); Seems impossible - OpenGL?
? Level Modes: Survive, KillAll (time-), Hurry (speed-), Aim! (HR bonus), ...
? Shop every 200 distance

*******************************************************************************
BUGS:
? Very, very rarely, player's lasers warp around and come back from behind
? When FIELD_HEIGHT is set to 1000, some enemy-polygons appear right
     in front of the camera (for one frame)
? New like above, but when MAX_ENEMIES changed from 100 to 256
? Sometimes; scene.c:draw_distance_marker() When texture coordinates are not
     applied, a shot fired by the player immediately hits the player's ship
     (100% reproducable, if the bug occurs at all)
     alpha0.2.3: Bug suddenly disappeared, even in old versions.
     Weird thing: I certainly did not update anything on neither of both my
     computers.
*******************************************************************************

// TODO v1.0 //////////////////////////////////////////////////////////////////
! Credits in intro screen
! Recode sound track (Many systems won't have OGG support by default)
! Licensed media files only
! Fix background image (is not properly "endless", also copyright)
! Use correct BPP
~ Rewrite: scene.c: Proper OpenGL usage
~ Improve: Multiple sounds played at the same time --> not all played.
> Full documentation
> Improve CPU usage (Certain calculations are done quite expensively right now)
> Portability (Mac, Windows, Linux, some Mobiles)

// TODO v0.3 //////////////////////////////////////////////////////////////////
! Lasers should not only cause damage at their tips
! Improve font size calculated from screen resolution, also onResize
! check ON RESIZE effects
? asprintf shouldn't need to free  s  all the time
- Improve show_text() function(s) - at least allow sizing
- may_first.conf (keyboard, sound/music, play list, full screen)
- Command line switches (Music, Sound, Volume, Resolution, Fullscreen)
- Visualize volume while changing (OSD style)
> Revisit: Enemies aiming at the player's ship
> Revisit hit detection (low FPS), see world.c
! Revisit: Grid/PointSize, Lasers/LineWidth
! Revisit: Blackhole darkness (CHEAT OFFSET?)
! Shield: activate with a key, consumes lots of resource
! Super-shot: charge from resource, release key-->roundshot
! Warp Button: Jump/Dive and speed forward some distance
- Several formation layouts, one of them could become a "cluster-ship"
  Nesting of formations?

// TODO v0.2 //////////////////////////////////////////////////////////////////
~ Change to tier (1,2,3 instead of 1,3,6)
? Formation: Don't fill when it leads out of the screen
! Formation: Balanced refill depending on symmetry (diagonal refills)
! NewEnemyAfterWarp: Sometimes you can't finish the current wave
- INVISIBLE_FIELD_FACTOR: Controlling visible field/out of view ratio
- FIELD_HEIGHT_FACTOR: level dependent "empty" area(s)
- Level design instead of random enemies (Wave(s), Boss, Wave(s), Boss...)
- Revisit: Weapon unlocking, level_design.c
- Revisit: Warp-around-malus- vs. kill-next level enemy (kill appears at once)
  Perhaps different game modes to choose from?
- Path: Types: Orbiting, zigzag, sine, spiral, follow, evade
- Formation: Reintegrate enemies (growth effect)
- Formation: various patterns, geometry growing dynamically -> most sphere like
- weapon property: locked_until_us
- New weapon upgrade: Use primary and double laser at the same time (tripple)
- Bosses: T1 uses auto, T2 double, T3 round, T4 ?
- Upgrades as objects (?type in bonus_bubble_t, double circle, ...)
- Announce NEW HISCORE, combine with "WAVE" insert
- Round-Shot: Deny-sound played multiple times?
- Bonus x2, x3, ...
- More models for enemies (Jelly Fish, Tetraeder-Dots)
- Beams, etc: owner --> creator
- Cooldown time for round shot, but cheap (10-20)
- AI Modes (Straigt line in modes)
- Left/Right roll smooth transition between the current 3 ground states
- Score: Enemy Speed = Nr. killed / distance

Look for comments like //... in all files: $ grep -n '//\.\.\.' *.{c,h}
Kill a.out: # kill -9 $(ps -a | grep a.out | awk '{ print $1 }')
backup: $ rsync -avz ~/project/may_first/ viator:~/project/may_first
List used libraries: ldd a.out
.../dev/$ git add . -u   $ git commit   $ git push
.../dev/$ git rm this.swp -f   $ git status
