Known Issues
	In slow-mo, debries change its size via frame rather than time.
How to Use
	(as required)
	P: toggle pause/play
	T: hold to slow down
	Arrow left/right: turn
	Arrow Up: thrust (max speed limited)
	A: Spawn an asteroid
	N: Respawn player ship
	F1: Toggle debug mode
	F8: Hard reset
	Space: shoot a bullet

	XBOX CONTROLLER
	Left Stick: facing and thriving
	A:	single shoot
	RT:	Rapid shoot
	Start:	Respawn
	
	Gameplay:
	There are 3 beetles (each with 40 Health) at the beginning and several asteroids.
		The beetles' health are indicated by its center color.
	Beat down all beetles to finish game.
	Wasps and asteroids automatically respawn.
	You have 4 lifes. When you spawn and rewpawn, you have immunity for 3s.

Deep Learning

1. When I am adding more gameplay stuffs into this assignment, it seems my Game.cpp is exploding. At the end of coding it's very terrible to find exact piece of function I coded
before. This is terrible, and I'm still not sure if I should refactor it.
	I should have write this Game.cpp in more expendable approachs, but I restricted myself to write less code just to fulfill the temporary requirements. But now It
turns out that less code for now never garentees less code in future.

2. Better not copy & paste.
	By copy & paste similar code (like behavior of debries and bullets, beetles and wasps), I brought in a bunch of bugs. Most of them came from forgetting to change
an varible or constant that are desinated for the old part.
	I think writing again and then checking the previous working code in case forgetting a few additional operation is a better way than simply copy & paste.