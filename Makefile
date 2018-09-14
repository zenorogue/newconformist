#  // windy: 9*fivecells
#  // zebra: 6*fivecells
#  // reptiles: 3*fivecells

nconf: nconf.cpp mat.cpp zebra.cpp triangle.cpp
	g++ nconf.cpp -o nconf -lgd -lSDL -O3 -std=c++1z

elegant-cat-outside.map: nconf
	./nconf -bco elegant-cat2.png -cm -sm elegant-cat-outside.map

elegant-cat-inside.map: nconf
	./nconf -bci elegant-cat2.png 12 583 750 5 -cm -sm elegant-cat-outside.map

draw: nconf
	./nconf \
	  -lm elegant-cat-outside.map -li zebra.png -zebra -period 6 -fix \
	  -lm2 elegant-cat-inside.map -li reptiles.png -zebra -period 3 \
	  -draw

spintest: nconf
	./nconf \
	  -lm elegant-cat-inside.map -li zebrabright.png -zebra -period 6 \
	  -spinspeed .0000001 -draw

pointtest: nconf
	./nconf -scale 10 -cbi elegant-cat2.png 80 267 615 292 -sb pointtest.txt

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
	  -draw -bandlen -exportv 0.20521875 2000 frames/hilbert-frames-%03d.png
	ffmpeg -framerate 15 -i "frames/hilbert-frames-%03d.png" -f mp4 -vcodec libx264 -r 15 hilbert.mp4

triangle400.map: nconf
	./nconf -triangle 400 -cm -sm triangle400.map

triangle-earth: nconf
	./nconf -lm triangle400.map -li earthspin.png -tm -export triangles/tearth.png

