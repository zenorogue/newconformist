# newconformist
Conformally map the hyperbolic plane to arbitrary shapes.

![sample picture](http://roguetemple.com/z/newconformist/result.png)
[(twitter link)](https://twitter.com/ZenoRogue/status/989908535818506243)

![sample picture](http://roguetemple.com/z/newconformist/hilbert-std.png)
[(twitter link)](https://twitter.com/ZenoRogue/status/998967789510643712) [(animated version on YouTube)](https://www.youtube.com/watch?v=vxF8lwa0z3I)

# References

Play the tutorial in [HyperRogue](http://roguetemple.com/z/hyper) as an introduction to hyperbolic geometry.

According to [Riemann's mapping theorem](https://en.wikipedia.org/wiki/Riemann_mapping_theorem) any non-empty simply connected open subset of R^2
can be conformally mapped to the hyperbolic plane.

[This presentation by Vladimir Bulatov](http://bulatov.org/math/1001/index.html) is a nice introduction to conformal models of hyperbolic geometry.
Bulatov uses specific formulas to map to the hyperbolic plane to multiple shapes; on the other hand, newconformist takes an arbitrary shape and finds
how to map to this shape.

The computational aspect of the Riemann's mapping theorem is a well studied problem (see the references in the Wikipedia article above); the algorithm
used by newconformist is independent of this research, though. We have not compared it to the known algorithms. The inspiration for this project comes
from the recent rectangular conformal mappings by @ChristopherKing42 (using the method described [here](https://github.com/zenorogue/newconformist/issues/1)).

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
merging point. If our x coordinate is greater, this means we should use the second mapping instead. Use the band coordinates of the merging point and adjacent points in both mappings 
to find a transformation which agrees at these points, and apply this transformation.

This process can be generalized to more forks. We need to perform mapping for any B which is an end, and find merging points for each pair of "adjacent" forks, until we merge everything.

# Sample images

See http://roguetemple.com/z/newconformist/ for sample image files (shapes and tilings). Tilings come from HyperRogue, the Elegant Cat Silhouette by GDJ comes from
https://openclipart.org/detail/258901/elegant-cat-silhouette .

# Options

Newconformist is controlled via command line options. See the Makefile for the invocations used to create the sample image.
The (*) signifies options which should be there but are not implemented yet (they would be rather easy to add).

`-rectangle <X> <Y>`: prepare a XxY rectangle from mapping.

`-scale <scale>`: scale the loaded shapes down by the given factor. Useful for quick testing.

`-margin <margin>`: automatically add margins to the loaded shape images. When `-scale` is given after `-margin`, the margin is scaled too, otherwise it is not scaled.
Margin is automatically increased if the resulting size is smaller than the current map, so doing `-rectangle` first ensures that a given resolution (or larger) is used.

`-mim <shape.png>`: set up an image file for mapping

`-cbo <x> <y>`: prepare the outside of the image specified with `-mim` for mapping. Pixels with the same color as (x,y) are considered to be outside. The given coordinates need to
be straight to the left from the inner hole. Coordinates are given relative to the original image, i.e., before scaling and adding margins.

`-cbi <shape.png> <x1> <y1> <x2> <y2>`: prepare the inside of the image specified with `-mim` for mapping. The points A and B are the points on the boundary closest to the given coordinates.
Coordinates are given relative to the original image, i.e., before scaling and adding margins.

`-sb <file>`: save the current boundaries in text format. (*) No way to load this format.

`-hilbert <level> <width> <border>`: create a shape based on the Hilbert curve. Parameters are: the level of the Hilbert curve, width in pixels (including the border), and the width of 
the border in pixels (on one side). Sample values: `-hilbert 4 32 2`.

`-cm`: compute the mapping.

`-sm <file.map>`: save the current map to a file.

`-lm <file.map>`: load the previously computed map from a file.

`-lm2 <file.map>`: use both outside and inside map with -lm <outside-file.map> <tiling options...> -lm2 <inside-file.map> <tiling options...>.

`-lmjoin <file.map> <x> <y>`: merge a fork given in <file.map>. Coordinates specify the merging point; coordinates in the original image are used as long as -scale and -margin are known
(i.e., given before in the same call), otherwise use the after-scale coordinates.

`-back`: the command `-lmjoin` by default merges the last side. This command tells the next `-lmjoin` to use the parent of that side instead. For example, to map a binary three with
eight leaves A, B, C, D, E, F, G, H, you would write `-lm A -lmjoin B -back -lmjoin C -lmjoin D -back -back -lmjoin E -lmjoin F -back -lmjoin G -lmjoin H -back -back -back` (merging
points omitted for clarity).

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

`-export <image.png>`: export the tiled shape to PNG.

`-exportv <speed> <cnt> <image.png>`: export the animation to a sequence of PNGs; ffmpeg can be used to make a video out of them. (You can see the animation in the `-draw` mode by pressing
the `1`-`9` keys, `0` to stop, `r` to reverse.)

`-bandlen`: when animating a band, tells you what parameters should be given to make a loop.
