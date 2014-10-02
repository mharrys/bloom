Bloom
=====
Simple post-procesing effect using OpenGL and GLSL.

+ Press `Left arrow` or `Right arrow` to control the rotation speed.

License
-------
Licensed under GNU GPL v3.0.

Demo
----
Click [here](https://github.com/mharrys/bloom/raw/master/demo.webm) for a video demonstration.

How-to
------
You will need a C++11 compiler, GLM, GLEW, Assimp and SDL2. Consult SConstruct for
details.

Build and run

    $ scons
    $ cd bin
    $ ./bloom

Cleanup

    $ scons -c

References
----------
1. David Wolff. OpenGL 4.0 Shading Language Cookbook. Packt Publishing. 2011.
2. Efficient Gaussian blur with linear sampling. [downloaded 2014-09-15]. Available from http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/comment-page-1/.
