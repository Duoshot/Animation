Gorman Law
10053193
CPSC 587 Assignment 3 Mass Spring System
November 19, 2015
TA: Andrew Owens
Done in Visual Studio Community 2015 with GLUT 3.7.6 and GLEW 3.1. Tested and working on lab computers

There are no keyboard commands unfortunately.

To compile type 'make' in console.
To run, type './mss' in console.

To restart, close the window, and type './mss' again.

Choose model to display by typing in the number in brackets

Descriptions

	(1)Mass on a spring
		One really heavy mass attached by a string to a normal mass. Nothing special

	(2)Chain pendulum
		One really heavy mass connected to a series of particles with normal mass.

	(3)Cube of jelly
		I used a loop to generate a bunch of particles a set distance apart from each other in 3D
		I then used a nested for loop to do distance adding. If the particles are a certain distance away from each other, I add a spring between them. This is also used to add shear springs (diagonal springs) so that the cube doesn't collapse on itself. The bottom of the jelly also uses a very primitive collision detection with the floor.
		I also raised the spring constant very high so that the springs are stiffer and don't collapse as easily

	(4) Hanging cloth
		Similar to (3) I used a loop to generate particles but in 2D.
		I used distance adding to create springs, bending constraint springs(springs that connect to another particle further away), and shear springs, to prevent the cloth from bending too much or collapsing on itself.
		Again, the spring constant is set very high so the cloth doesn't drag all the way down.
		I pinned both top corners by setting the mass extremely high.

	(5) Flag flapping in the wind (Bonus)
		I used the same method as (4) to create the flag, but instead of being pinned at the top, it is pinned at the left side. I only added shear springs to the flag. To simulate wind, I added a wind force that blows in the positive X direction. The wind force oscillates from low to high.

	(6) Window with drapes moving in the wind (Bonus)
		Using methods from (4) and (5), I created a drape that blows slowly in the wind. I also added a wind force, but it blows weaker than the wind on the flag so simulate a light breeze. I only used shear springs and I pinned all the springs to the top of the window by setting the masses high.


Any and all sources were from tutorials. Collaborated with Mike Wang, so some code might look similar.
