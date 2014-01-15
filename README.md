
                                  MAY FIRST
         A game inspired by  http://en.wikipedia.org/wiki/Juno_First
                 Written 2013 by hmw, http://harald.ist.org/

Index
	1. Introduction
	2. Gameplay
	3. Building from source
	4. Mode of operation
	5. Beta testing questionaire
	6. TODO


*******************************************************************************
* 1. INTRODUCTION (alpha 0.2.0)
*******************************************************************************

The model for this game, Juno First, is a real master piece and a classic.
I immediately fell in love with this game for reasons like how certain effects
are achieved with almost no effort, or the speed rush one experiences when
getting into a firing frenzy, which somehow works extremly well in Juno First.

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
* 2. GAMEPLAY (alpha 0.1.7)
*******************************************************************************

The player is confronted with an armada of alien tetraeder shaped space
vessels. Dieing enemies drop "Resource" for the player to collect. Everything
is payed from this single resource, from weapon fire to absorbing hits.

While it builds up slowly over time, the player will get the main portion of
resource through killing enemies. Available weapons are:

- LASER Beam      Costs one unit of Resource per shot.
- Double LASER    Costs four units of Resource.
- Triple LASER    Press the keys for main and double fire at the same time.
                  Costs five units of Resource.
- Round Shot      Fires a range of beams in all directions. Costs a lot.

RESOURCE
--------
If enemies pass by the player's ship, they will re-appear a short while later
on the "field" until they are destroyed or the game ends.
Avoid the "black holes", they will drain your Resource quickly. If you get
too near to the center, the game will end.
If Resource is low (indicator blinks), it is a good idea to aim properly and
go for the small enemies (blue or green) in order to regain Resource.
If the ship gets hit, Resource is halved until it is below 50.
Taking a hit with Resource below 50 leads to death and the end of the game.

LEVEL DESIGN
------------
There is no level design yet, SPAWNING OF NEW ENEMIES is simply triggered by
killing existing ones. Currently there is only an ugly routine calculating the
probabilities of type and number of new enemies.

The ROUND SHOT DOES NOT TRIGGER NEW ENEMIES, so you should use it to "clean up"
the playing field every now and then to keep the number of enemies low.


*******************************************************************************
* 3. BUILDING FROM SOURCE (alpha 0.1.7)
*******************************************************************************

You may need to install a few packages before you can compile the program.
Under Debian 7.1 I had to issue:

	$ sudo apt-get install      \
		libsdl-image1.2-dev \
		libglew-dev         \
		libsdl-ttf2.0-dev   \
		libsdl-mixer1.2-dev \
		libsdl-gfx1.2-dev

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
ui.h       UI related settings (Full screen mode, sound)
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
  level_design.c  Level control, functions describing game rules
  world.c         Advances the world by one step
  player.c        Player specific routines (Taking hits, controls, ...)
  enemy.c         Enemy specific routines (Taking hits, AI, ...)
  formation.c     Organizes enemies into formations and provides controls


*******************************************************************************
* 5. BETA TESTING QUESTIONAIRE
*******************************************************************************

- OS, GPU
- Installation protocol
- Gameplay: Difficulty, Progress/Curve, "Frenzy-factor"
- Technical issues and bugs


*******************************************************************************
* 6. TODO
*******************************************************************************

// IDEAS //////////////////////////////////////////////////////////////////////
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
? Explosions trigger further explosions
? Hyperjump
? Particle effect for ship rocket
? Easter Egg
? Ricochets
? RunModes --> game_state
? Black Holes suck in enemies and Lasers, distance markers
? Black Holes refract lasers
\ Re-enable F11 (Toggle Fullscreen); Seems impossible - OpenGL?

*******************************************************************************
BUGS:
? Very, very rarely, player's lasers warp around and come back from behind
? When FIELD_HEIGHT is set to 1000, some enemy-polygons appear right
     in front of the camera (for one frame)
? New like above, but when MAX_ENEMIES changed from 100 to 256
*******************************************************************************

// TODO v1.0 //////////////////////////////////////////////////////////////////
! Shop every 200 distance
! Credits in intro screen
! Recode sound track (Many systems won't have OGG support by default)
! Licensed media files only
! Fix background image (is not properly "endless", also copyright)
! Use correct BPP
~ Rewrite: scene.c: Proper OpenGL usage
> Full documentation
> Improve CPU usage (Certain calculations are done quite expensively right now)
> Portability (Mac, Windows, Linux, some Mobiles)

// TODO v0.3 //////////////////////////////////////////////////////////////////
! Lasers should not only cause damage at their tips
! Improve font size calculated from screen resolution, also onResize
! check ON RESIZE effects
? asprintf shouldnt need to free  s  all the time
- Improve show_text() function(s) - at least allow sizing
- may_first.conf (keyboard, sound/music, play list, full screen)
- Command line switches (Music, Sound, Volume, Resolution, Fullscreen)
- Visualize volume while changing (OSD style)
> Revisit: Enemies aiming at the player's ship
> Revisit hit detection (low FPS), see world.c
! Revisit: Grid/PointSize, Lasers/LineWidth
! Revisit: Blackhole darkness (CHEAT OFFSET?)
! Shield: activate per key, consumes lots of resource
! Super-shot: charge from resource, release key-->roundshot

// TODO v0.2 //////////////////////////////////////////////////////////////////
~ Change to tier (1,2,3 instead of 1,3,6)
! Formation: Don't fill when it leads out of the screen
! Formation: Balanced refill depending on symmetry (diagonal refills)
- INVISIBLE_FIELD_FACTOR: Controlling visible field/out of view ratio
- FIELD_HEIGHT_FACTOR: level dependent "empty" area(s)
- Level design instead of random enemies (Wave(s), Boss, Wave(s), Boss...)
- Revisit: Weapon unlocking, level_design.c
- Revisit: Warp-around-malus- vs. kill-next level enemy (kill appears at once)
- Revisit: Shooting frequency of enemies
- Path: Types: Orbiting, zigzag, sine, spiral, follow, evade
- Formation: Reintegrate enemies (growth effect)
- Formation: various patterns, geometry growing dynamically -> most sphere like
- weapon property: locked_until_us
- New weapon upgrade: Use primary and double laser at the same time (tripple)
- Bosses: T1 uses auto, T2 double, T3 round, T4 ?
- Upgrades as objects (?type in bonus_bubble_t, double circle, ...)
- Announce NEW HISCORE
- Round-Shot: Deny-sound played multiple times?
- Bonus x2, x3, ...
- More models for enemies (Jelly Fish, Tetraeder-Dots)
! HitRatio negative: check Round Shot

Look for comments like //... in all files: $ grep -n '//\.\.\.' *.{c,h}
Kill a.out: # kill -9 $(ps -a | grep a.out | awk '{ print $1 }')
backup: $ rsync -avz ~/project/may_first/ viator:~/project/may_first
List used libraries: ldd a.out
