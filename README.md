# newconformist
Conformally map the hyperbolic plane to arbitrary shapes. See [our Bridges 2019 paper](https://archive.bridgesmathart.org/2019/bridges2019-91.html) for a full exposition.
See also [our interactive Web-based explorable explanation](http://www.roguetemple.com/z/sims/nconf/) to understand the core ideas better.

![sample picture](http://roguetemple.com/z/newconformist/ecat-basic.png)

![sample picture](http://roguetemple.com/z/newconformist/result.png)
[(twitter link)](https://twitter.com/ZenoRogue/status/989908535818506243)

![sample picture](http://roguetemple.com/z/newconformist/hilbert-std.png)
[(twitter link)](https://twitter.com/ZenoRogue/status/998967789510643712) [(animated version on YouTube)](https://www.youtube.com/watch?v=vxF8lwa0z3I)

![forked shape](http://roguetemple.com/z/newconformist/triskele-textured.png)

# Quick References

Play the tutorial in [HyperRogue](http://roguetemple.com/z/hyper) as an introduction to hyperbolic geometry.

According to [Riemann's mapping theorem](https://en.wikipedia.org/wiki/Riemann_mapping_theorem) any non-empty simply connected open subset of R^2
can be conformally mapped to the hyperbolic plane.

[This presentation by Vladimir Bulatov](http://bulatov.org/math/1001/index.html) is a nice introduction to conformal models of hyperbolic geometry.
Bulatov uses specific formulas to map to the hyperbolic plane to multiple shapes; on the other hand, newconformist takes an arbitrary shape and finds
how to map to this shape.

The computational aspect of the Riemann's mapping theorem is a well studied problem (see the references in the Wikipedia article above); the algorithm
used by newconformist is independent of this research, though. We have not compared it to the known algorithms. The inspiration for this project comes
from the recent rectangular conformal mappings by @ChristopherKing42 (we did not know what algorithm he has been using) (EDIT: method described [here](https://github.com/zenorogue/newconformist/issues/1)).

Newconformist can also conformally map an annulus to a subset which is homeomorphic to it. This is used to map the outside of the cat in the sample
image above. (We have not tried to actually prove a version of
Riemann's mapping theorem for annuli, but since newconformist works, we suppose it is true.)

# Algorithm

We are looking for a conformal mapping from our shape to the hyperbolic plane. The algorithm assumes that the hyperbolic plane is
represented in the band model. The "minus infinity" and "plus infinity" points of the band model are mapped to two points A and B on the boundary;
they split the boundary into two parts, one is mapped to the top of the band, and the other is mapped to the bottom.

We solve the discrete problem of finding the mapping from the pixels of our shape to the band. Conformal mappings are harmonic;
in discrete terms, it means that the position of each pixel inside is the average of the position of its neighbors. In the grid world, A and B will be 
mapped to large, but finite values.

The specification above can be understood as a linear system of equations. We do not know what A and B will be mapped to -- but we can assume A=0, B=1,
solve the system of equations, and find what scaling factor should we use on the x coordinate to make the result conformal.

The system of equations is solved with Gaussian elimination. Eliminating a pixel with k adjacent pixels takes time O(k^2), and makes all the adjacent pixels 
mutually adjacent. The running time of the algorithm depends on the order of elimination; newconformist tries to choose the order which makes it work fast.

Outsides are mapped to wrapped bands in the same way. The outside/inside border is mapped to the top/bottom of the band, and the ring is cut across a line --
for adjacent vertices on the opposite sides of the line, +1/-1 is added to the average.

# Forks

The algoritm works very well with snake-like shapes if the points A and B are chosen to be the ends of the snake. For the cat picture, A and B are the front leg
and the end of the tail. For forked shapes though, long "tails" except A and B may be not rendered nicely due to precision errors. (This does not yet happen 
with the cat's other legs, because they are short enough.)

To solve this problem, we can use the following method. Suppose that we are trying to map an Y-shape.

* First, perform mapping with A = bottom end of the Y, B = top left end of the Y.
* Second, perform mapping with A = bottom end of the Y, B = top right end of the Y.
* Choose a merging point -- this will be the center of the Y.
* When rendering the image, assume the first mapping first. Check the band x coordinate according to the second mapping, and compare it with the band x coordinate of the
merging point. If our x coordinate is greater (at least by 2), this means we should use the second mapping instead. Use the band coordinates of the merging point and adjacent points in both mappings 
to find a transformation which agrees at these points, and apply this transformation.

This process can be generalized to more forks. We need to perform mapping for any B which is an end, and find merging points for each pair of "adjacent" forks, until we merge everything.
Newconformist can find all the branching and ending points automatically; the methods works correctly for most shapes.

# Sample images

See http://roguetemple.com/z/newconformist/ for sample image files (shapes and tilings). Tilings come from HyperRogue, the Elegant Cat Silhouette by GDJ comes from
https://openclipart.org/detail/258901/elegant-cat-silhouette .

# Dependencies

Newconformist is written in C++17. It uses libgd for loading and saving the PNG files.

# Usage: quick start

Suppose your shape is in file `shape.png`, and you want to map it with the Zebra tiling, in `zebra.png`. Find the region you want to map in your shape, and coordinates of
some point inside this region, say, (100,50), Then you can execute the algorithm and see the resulting mapping with the following command:

* `make nconf` to compile
* `./nconf -mim shape.png -mapat 100 50 -li zebra.png -zebra -period 6 -draw` to draw. (`-zebra -period 6` tells newconformist the period of the picture, so that it knows how to recenter
it.)

If you want to map all the regions (outside, inside...) in the picture, do the following commands

* `./nconf -mim shape.png -mapall -sma shape.maps` will map all the regions and save the mappings in file `shape.maps`.
* `./nconf -lma shape.maps` to list all the mapped regions ("sides") and their identifiers.
* to draw the result, do e.g. `./nconf -lma shape.maps -side 0 -li zebra.png -zebra -period 6 -draw`. The part `-side 0 -li zebra.png -zebra -period 6` should be repeated for all the
sides, preferably with changed images; the number 0 is side ID listed by the previous command.

If you want to export the result to a file, replace `-draw` with `-export result.png`.

# More options

Newconformist is controlled via command line options. See the Makefile for the invocations used to create the sample image (although some of them are obsolete).
The (*) signifies options which should be there but are not implemented yet (they would be rather easy to add).

`-scale <scale>`: scale the loaded shapes down by the given factor. Useful for quick testing.

`-margin <margin>`: automatically add margins to the loaded shape images. When `-scale` is given after `-margin`, the margin is scaled too, otherwise it is not scaled.
Margin is automatically increased if the resulting size is smaller than the current map, so doing `-rectangle` first ensures that a given resolution (or larger) is used.
Default: 32.

`-mim <shape.png>`: set up an image file for mapping

`-mapat <x> <y>`: Compute the conformal mapping for the region including pixel (x,y) (given in original coordinates). This region contains all the pixels of the same color
as (x,y) and connected to it.

`-mapall`: Map all regions in the picture.

`-sma <file.map>`: save all the conformal mappings in the memory to a file.

`-lma <file.map>`: load a file saved with `-sma`.

`-side <id>`: specify the side that will be affected by the subsequent commands. The command `-lma` lists all the sides in the saved map file.

`-li <tiling.png>`: tile the current shape with the given hyperbolic tiling. The tiling picture should be in the Poincaré disk model. Pictures generated
by the "HQ shot" feature in HyperRogue's map editor work well (the periods assume that you have not moved nor rotated the screen). 

`-lband <band-image.png>`: tile the current shape with the given image in the band model. Band images exported from HyperRogue work great. Can be used many times to 
load multiple segments of the same band.

`-lbands <min> <max> <format.png>`: use -lband on a list of files according to the given (printf-style) format. For example, `-lbands 1 11 std%d.png` will load std1.png, std2.png, ...,
std11.png.

`-zebra`: assume that the tiling is periodic as in HyperRogue's Zebra pattern. This lets newconformist to take advantage of the periodicity and
map to a point close to the center of the tiling picture.
(*) More periods, such as HyperRogue's Palace and Vineyard patterns, or the patterns corresponding to the octagonal tiling, would be very useful.

`-period <value>`: period of the tiling when moving to the right (i.e., the period of the underlying band model). Specifying the period helps greatly with snake-like shapes. 
If `-zebra` is used, the unit is three heptagonal steps; absolute units are used otherwise.
Use 3 for the Reptile pattern, 6 for the Zebra pattern, 9 for the Windy Plains pattern, 1 for the tilings where every heptagonal cell looks the same.
 
`-measure`: give the statistics of the current map. The important statistic for outside tilings is the hyperbolic length of the loop, divided by the period. Tiling the
outside neatly is possible only if this value is an integer.

`-fix`: if the number returned by `-measure` is not an integer, cheat a bit (by stretching the tiling) so that it is. (For example, the outside of the Elegant Cat image
has hyperbolic length of 6.004 Zebra periods -- after stretching it is exactly 6 Zebra periods and the cheating is too small to be visible.)

`-draw`: draw the tiled shape on the screen.

`-eo <n>`: experiment with different elimination orders in Gaussian elimination (0, 1, 2, 3). The default is 3 (also the fastest).

`-export <image.png>`: export the tiled shape to PNG.

`-exportv <speed> <cnt> <image.png>`: export the animation to a sequence of PNGs; ffmpeg can be used to make a video out of them. (You can see the animation in the `-draw` mode by pressing
the `1`-`9` keys, `0` to stop, `r` to reverse.)

`-bandlen`: when animating a band, tells you what parameters should be given to make a loop.

`-chessmap <x> <y>`: instead of loading an image, display the hyperbolic straight line from <x1,y1> to <x2,y2>, and the orthogonal line passing throough <x,y>, with markers each 1 hyperbolic unit.

`-q`: do not display the progress while computing maps. Also `-qt` does not display the progress in text, and `-qd` does not display the progress as a picture.

`-joinparam <eps> <pixels> <y>`: set the parameters for the branching algorithm. Branches are detected when the Y coordinate of the mapping is below eps or above 1-eps in distance greater
than the given number of pixels. The branch merge point is chosen at the given y (or 1-y). The default values are: `-joinparam 1e-5 5 .1`.

`-joinoff`: disables the branching algorithm.

`-cvlgen <filename>`: use HyperRogue/RogueViz to generate tilings for a branching shape. See the Makefile for an example. The process is as follows:
* First, generate a mapping, and save it with e.g. `-sma maze.maps`.
* Run newconformist with `-lma maze.maps -cvlgen maze-cvl.txt` to generate a text file asking about points/rotations. 
* Run [RogueViz](https://github.com/zenorogue/hyperrogue/) with parameters `-cvlbuild maze-cvl.txt -cvldraw mcvl/maze-%02d-%03d.png` to generate a sequence of PNG files.
* Run newconformist with e.g. `-lma maze.maps -cvlimg mcvl/maze-%02d-%03d.png`. 

# Shapes

`-rectangle <X> <Y> -cm`: prepare a XxY rectangle from mapping.

`-circle <R> -cm`: prepare a circle of radius R for mapping. We know how to make a circular conformal model, so this is for testing our approximations.

`-triangle <L>`: prepare an equilateral triangle of edge length L.

`-diamond <L>`: prepare a diamond (actually, a square rotated by 45 degrees) of diagonal length 2L.

`-hilbert <level> <width> <border> -cm`: create a shape based on the Hilbert curve. Parameters are: the level of the Hilbert curve, width in pixels (including the border), and the width of 
the border in pixels (on one side). Sample values: `-hilbert 4 32 2 -cm`.

# Side experiments

`-tm`: an experiment with quincuncial-like triangular projection. You need to generate a `-triangle` map first, then do `-li <image> -tm`, where `<image>` is in equirectangular projection. See
[here](https://twitter.com/ZenoRogue/status/1026227019925868544).

`-spiral`: an experiment with spiral-like projection (undocumented). See [here](https://twitter.com/ZenoRogue/status/1057271562083229697).

`-quincunx`: make a Peirce's quincuncial projection. You need to generate a `-diamond` map first, then do `-li <image1> -quincunx -li <image2>`, where `<image1>` and `<image2>` are the two
hemispheres in stereographic projection. Alternatively, do `-quincunx -lquincunx <scale> <image>`, where `<image>` is an Euclidean image which will be mapped to a sphere using stereographic
projection. See [here](https://twitter.com/ZenoRogue/status/1145454607298174981).

`-cheetah`, `-excheetah`: an experiment with shape changing (undocumented). See [here](https://twitter.com/ZenoRogue/status/1096097650057846785).

# Obsolete commands

These commands come from earlier versions of Newconformist. It is better to use the commands above.

`-cbo <x> <y>`: prepare the outside of the image specified with `-mim` for mapping. Pixels with the same color as (x,y) are considered to be outside. The given coordinates need to
be straight to the left from the inner hole. Coordinates are given relative to the original image, i.e., before scaling and adding margins.

`-cbi <x1> <y1> <x2> <y2>`: prepare the inside of the image specified with `-mim` for mapping. Points with the same color as (x1,y1) are considered to be inside.
The points A and B are the points on the boundary closest to the given coordinates.
Coordinates are given relative to the original image, i.e., before scaling and adding margins.

`-sb <file>`: save the current boundaries in text format. (*) No way to load this format.

`-cm`: compute the mapping.

`-mergesides`: if you generate multiple sides manually (with -cbi or -cbo), it is necessary to use this after mapping each of them.

`-sm <file.map>`: save the current map to a file.

`-lm <file.map>`: load the previously computed map from a file.

`-lm2 <file.map>`: use both outside and inside map with -lm <outside-file.map> <tiling options...> -lm2 <inside-file.map> <tiling options...>.

`-lmjoin <file.map> <x> <y>`: merge a fork given in <file.map>. Coordinates specify the merging point; the current values of scale and margin are used to understand ,x> and <y>.

`-back`: the command `-lmjoin` by default merges the last side. This command tells the next `-lmjoin` to use the parent of that side instead. For example, to map a binary three with
eight leaves A, B, C, D, E, F, G, H, you would write `-lm A -lmjoin B -back -lmjoin C -lmjoin D -back -back -lmjoin E -lmjoin F -back -lmjoin G -lmjoin H -back -back -back` (merging
points omitted for clarity).


