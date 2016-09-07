Gorman Law
10053193
CPSC 587 Assignment 1 RollerCoaster
October 8, 2015
TA: Andrew Owens
Done in Visual Studio Community 2015 with GLUT 3.7.6 and GLEW 3.1. I hope it works on the lab computers. Oh yes it does.

There are no keyboard commands unfortunately.

To compile type 'make' in console.
To run, type './rc' in console.

To restart, close the window, and type './rc' again. Barbaric, I know.

Track is read from the 'TestTrack.txt' file and the cart is read from 'Car.txt'. The lift, physics, and braking states are written in the console, along with the current speed.

The states are hardcoded (not dynamically calculated) so a different track might to work the same.

I was unable to figure out why the car flips when speed approaches 0, and it will probably drive me crazy forever.

Only the more realistic scenery bonus mark was implemented, with some high tensile steel red supports, a beautiful grass lawn, and a blue sky so soft on the eyes I would be blind not to give bonus marks.

Any and all sources were from tutorials. Collaborated with Mike Wang, so some code might look similar.
