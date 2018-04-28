# newconformist
Conformally map the hyperbolic plane to arbitrary shapes.

http://roguetemple.com/z/newconformist/result.png

# References

Play the tutorial in [HyperRogue](http://roguetemple.com/z/hyper) as an introduction to hyperbolic geometry.

According to [Riemann's mapping theorem](https://en.wikipedia.org/wiki/Riemann_mapping_theorem) any non-empty simply connected open subset of R^2
can be conformally mapped to the hyperbolic plane.

[This presentation by Vladimir Bulatov](http://bulatov.org/math/1001/index.html) is a nice introduction to conformal models of hyperbolic geometry.
Bulatov uses specific formulas to map to the hyperbolic plane to multiple shapes; on the other hand, newconformist takes an arbitrary shape and finds
how to map to this shape.

The computational aspect of the Riemann's mapping theorem is a well studied problem (see the references in the Wikipedia article above); the algorithm
used by newconformist is independent of this research, though. We have not compared it to the known algorithm. The inspiration for this project comes
from the recent rectangular conformal mappings by @ChristopherKing42 (but we do not know what algorithm he has been using).

Newconformist can also conformally map an annulus to a subset which is homeomorphic to it. (We have not tried to prove a version of
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

The algoritm works very well with snake-like shapes if the points A and B are chosen to be the ends of the snake. For the cat picture, A and B are the front leg
and the end of the tail. For tree-like shapes, long "tails" except A and B may be not rendered nicely due to precision errors (the cat's other legs are short
enough, though).

Outsides are mapped to wrapped bands in the same way. The outside/inside border is mapped to the top/bottom of the band, and the ring is cut across a line --
for adjacent vertices on the opposite sides of the line, +1/-1 is added to the average.

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

`-cbo <shape.png>`: load the shape image and prepare the outside for mapping. Pixels with the same color as the top left corner are considered to be outside.
(*) It is currently impossible to define both the outer and inner boundary -- the outer boundary will have to be rectangular.
(*) Newconformist currently chooses the straight line from (0,Y/2) to the center as the cut line -- this assumes that the point (X/2,Y/2) is inside.

`-cbi <shape.png>`: load the shape image and prepare the inside for mapping. 
(*) There is currently no way to choose the points A and B. The current method is tailored for the cat shape, might not work for other shapes.

`-sb`: save the current boundaries in text format. (*) No way to load this format.

`-cm`: compute the mapping.

`-sm <file.map>`: save the current map to a file.

`-lm <file.map>`: load the previously computed map from a file.

`-li <tiling.png>`: tile the current shape with the given hyperbolic tiling. The tiling picture should be in the Poincaré disk model. Pictures generated
by the "HQ shot" feature in HyperRogue's map editor work well (the periods assume that you have not moved nor rotated the screen). 
(*) No way to load a tiling in the band model, even though it would be easy to implement.

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
