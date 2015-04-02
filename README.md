Bloom
=====
High-dynamic-range (HDR) rendering with bloom as a post-processing effect. HDR
image from [Probes](http://www.pauldebevec.com/Probes/).

Screenshot
----------
![scrot](https://github.com/mharrys/bloom/raw/master/scrot.png)

How-to
------
This project depends on [Gust](https://github.com/mharrys/gust), see its
project page for details on dependencies.

Recursive clone required

    $ git clone --recursive <repository>

Build and run

    $ scons
    $ cd bin
    $ ./bloom

Cleanup

    $ scons -c

References
----------
1. David Wolff. OpenGL 4.0 Shading Language Cookbook. Packt Publishing. 2011.
2. Cube Maps: Sky Boxes and Environment Mapping. [downloaded 2015-04-01]. Available from http://antongerdelan.net/opengl/cubemaps.html.
3. High Dynamic Range Rendering in OpenGL. Fabien Houlmann; Stéphane Metz.
