#  // windy: 9*fivecells
#  // zebra: 6*fivecells
#  // reptiles: 3*fivecells

nconf: nconf.cpp mat.cpp zebra.cpp triangle.cpp btd.cpp
	g++ nconf.cpp -o nconf -lgd -lSDL -O3 -std=c++1z

nconf-debug: nconf.cpp mat.cpp zebra.cpp triangle.cpp btd.cpp
	g++ nconf.cpp -o nconf-debug -lgd -lSDL -g -std=c++1z

elegant-cat-outside.map: nconf
	./nconf -mim elegant-cat2.png -cbo 17 335 -cm -sm elegant-cat-outside.map

elegant-cat-inside.map: nconf
	./nconf -mim elegant-cat2.png -cbi 22 570 738 17 -cm -sm elegant-cat-inside.map

draw: nconf
	./nconf \
	  -lm elegant-cat-outside.map -li zebra.png -zebra -period 6 -fix \
	  -lm2 elegant-cat-inside.map -li reptiles.png -zebra -period 3 \
	  -draw

spintest: nconf
	./nconf \
	  -lm elegant-cat-inside.map -li zebrabright.png -zebra -period 6 \
	  -spinspeed .0000001 -draw

e1.map: nconf letter-e.png
	./nconf -mim letter-e.png -cbi 516 487 522 225 -cm -sm e1.map

e2.map: nconf letter-e.png
	./nconf -mim letter-e.png -cbi 516 487 520 800 -cm -sm e2.map

d.map: nconf letter-d.png
	./nconf -mim letter-d.png -cbo 120 500 -cm -sm d.map

drawe1: nconf
	./nconf -lm e1.map -li zebrabright.png -zebra -period 6 -draw

drawd: nconf
	./nconf -lm d.map -li zebrabright.png -zebra -period 6 -fix -draw

drawe2: nconf
	./nconf -lm e2.map -li zebrabright.png -zebra -period 6 -draw

drawe12: nconf
	./nconf -lm e1.map -li zebrabright.png -zebra -period 6 -lmj e2.map 166 485 -draw

triskele1.map: nconf triskele.png
	./nconf -mim triskele-flat.png -cbi 296 777 855 742 -cm -sm triskele1.map

triskele2.map: nconf triskele.png
	./nconf -mim triskele-flat.png -cbi 296 777 548 268 -cm -sm triskele2.map

triskeleo.map: nconf
	./nconf -mim triskele-flat.png -cbo 120 270 -cm -sm triskeleo.map

triskeleo1.map: nconf
	./nconf -mim triskele-flat.png -trim -9999 -9999 600 9999 -cbi 50 50 282 707 -cm -sm triskeleo1.map

triskeleo2.map: nconf
	./nconf -mim triskele-flat.png -trim 250 -9999 9999 700 -cbi 260 50 579 289 -cm -sm triskeleo2.map

triskeleo3.map: nconf
	./nconf -mim triskele-flat.png -trim -9999 500 9999 9999 -cbi 50 550 774 777 -cm -sm triskeleo3.map

drawt12o: nconf
	./nconf \
	  -lm triskele1.map -li rlyeh.png -zebra -period 6 -lmj triskele2.map 562 584 -ash 0.4 \
	  -lm2 triskeleo.map -li windy.png -zebra -period 6 -fix \
	  -lmj triskeleo1.map 244 469 -back \
	  -lmj triskeleo2.map 828 376 -back \
	  -lmj triskeleo3.map 600 936 -back \
	  -ntsblack -btbnd 10 -draw

drawt12ob: nconf
	./nconf \
	  -lm triskele1.map -li rlyeh.png -zebra -period 6 -lmj triskele2.map 562 584 -ash 0.4001 \
	  -lm2 triskeleo.map -li windy.png -zebra -period 6 -fix -ash 0.0001 \
	  -lmj triskeleo1.map 244 469 -back \
	  -lmj triskeleo2.map 828 376 -back \
	  -lmj triskeleo3.map 600 936 -back \
	  -ntsblack -draw

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

