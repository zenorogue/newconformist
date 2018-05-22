#  // windy: 9*fivecells
#  // zebra: 6*fivecells
#  // reptiles: 3*fivecells

nconf: nconf.cpp mat.cpp zebra.cpp
	g++ nconf.cpp -o nconf -lgd -lSDL -O3 -std=c++1z

elegant-cat-outside.map: nconf
	./nconf -bco elegant-cat2.png -cm -sm elegant-cat-outside.map

elegant-cat-inside.map: nconf
	./nconf -bci elegant-cat2.png -cm -sm elegant-cat-outside.map

draw: nconf
	./nconf \
	  -lm elegant-cat-outside.map -li zebra.png -period 6 -fix \
	  -lm2 elegant-cat-inside.map -li reptiles.png -period 3 \
	  -draw

draws: nconf
	./nconf \
	  -lm elegant-cat-outside.map \
	  -lm2 elegant-cat-inside.map \
	  -draw

# empty rectangle of the size suitable for A6 postcards, in 300 dpi
# needs about 16 GB of RAM and lots of time
postcard: nconf
	./nconf -rectangle 1750 1237 -cm -sm postcard.map
