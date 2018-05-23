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
	  -lm elegant-cat-outside.map -li zebra.png -zebra -period 6 -fix \
	  -lm2 elegant-cat-inside.map -li reptiles.png -zebra -period 3 \
	  -draw

draws: nconf
	./nconf \
	  -lm elegant-cat-outside.map \
	  -lm2 elegant-cat-inside.map \
	  -draw

# empty rectangle of the size suitable for A6 postcards, in 300 dpi
# needs about 16 GB of RAM and lots of time (413 minutes)
postcard: nconf
	./nconf -rectangle 1750 1237 -cm -sm postcard.map

heart1: nconf
	./nconf -rectangle 1750 1237 -cbi heart1.png -cm -sm heart1-outside.map
	./nconf -rectangle 1750 1237 -cbo heart1.png -cm -sm heart1-inside.map

heart1-out: nconf
	./nconf \
	  -lm heart1-outside.map -li flowerpattern.png -zebra -period 6 \
	  -lm2 heart1-inside.map -li rosegarden.png -zebra -period 1 -fix \
          -export heart-out.png

pc2:
	./nconf -rectangle 1750 1321 -cm -sm postcard2.map
	#./nconf -rectangle 1750 1321 -cbi heart1.png -cm -sm heart1-outside2.map
	#./nconf -rectangle 1750 1321 -cbo heart1.png -cm -sm heart1-inside2.map

heart1-out2: nconf
	./nconf \
	  -lm heart1-outside2.map -li flowerpattern.png -zebra -period 6 \
	  -lm2 heart1-inside2.map -li rosegarden.png -zebra -period 1 -fix \
          -export heart-out2.png
	./nconf -lm postcard2.map -li zebrabright.png -zebra -period 6 -export postcard2.png

hilbert.map: nconf
	./nconf -hilbert 3 64 2 -sb hmap.txt -cm -sm hilbert.map

# needs hilbert.map
draw-hilbert-chaos: nconf
	./nconf \
	  -lm hilbert.map -lband chaos1.png -lband chaos2.png -lband chaos3.png \
	  -draw

draw-hilbert-std: nconf
	./nconf \
	  -lm hilbert.map -lbands 1 11 std%d.png  \
	  -draw

draw-hilbert-stdx: nconf
	./nconf \
	  -lm hilbert.map -lbands 1 11 std%d.png  \
	  -draw -export hilbert-std.png

hilbert-small.map: nconf
	./nconf -hilbert 3 64 2 -sb hmap.txt -cm -sm hilbert-small.map

draw-hilbert-stdv: nconf
	mkdir -p frames
	./nconf \
	  -lm hilbert-small.map -lbands 1 11 std%d.png  \
	  -draw -bandlen -exportv 0.4104375 1000 frames/hilbert-frames-%03d.png
