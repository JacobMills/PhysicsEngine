PhysicsEngine
=============

2D physics engine, using Direct3D, that was started as part of a university module in third year.

Requires DirectX SDK from June 2010: http://www.microsoft.com/en-gb/download/details.aspx?id=6812

Presented here as-is from when I handed it in in 2012, complete with TODOs and unfinished funcitonality. Focus of the module was on programming quality and the quality of the completed features, with robustness of incomplete features also taken into account, so there was no pressure to add features in a crunch at the end or anything like that to reduce the quality of the code. However, it does mean the project was ended in an odd place, with velocities, mass etc implemented but not the time delta or anything in to actually move these bodies around on-screen. Again, this is a side-effect of the focus of the module, which took a bottom-up approach, meaning I was better off focusing on the bottom level stuff (rigid bodies, collision data, collision resolution), intending to expand and complete movement etc later on, if I had time. 

main.cpp is pretty much all setting up the demo scene. Explore core.cpp and the headers for implementations of the actual physics stuff.

The entire thing is based on the engine presented in the book Game Physics Engine Development by Ian Millington, adapted for 2D: http://procyclone.com/
