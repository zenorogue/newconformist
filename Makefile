#  // windy: 9*fivecells
#  // zebra: 6*fivecells
#  // reptiles: 3*fivecells

nconf: nconf.cpp mat.cpp zebra.cpp triangle.cpp btd.cpp spiral.cpp automapper.cpp quincunx.cpp
	g++ nconf.cpp -o nconf -lgd -lSDL -O3 -std=c++1z -Wall -Wextra

nconf-debug: nconf.cpp mat.cpp zebra.cpp triangle.cpp btd.cpp spiral.cpp automapper.cpp quincunx.cpp
	g++ nconf.cpp -o nconf-debug -lgd -lSDL -g -std=c++1z

elegant-cat-outside.map: nconf
	./nconf -mim elegant-cat2.png -cbo 17 335 -cm -sm elegant-cat-outside.map

elegant-cat-inside.map: nconf
	./nconf -mim elegant-cat2.png -cbi 22 570 738 17 -cm -sm elegant-cat-inside.map

elegant-cat-inside-big.map: nconf
	./nconf -mim elegant-cat3.png -cbi 66 1710 2214 51 -cm -sm elegant-cat-inside-big.map

elegant-cat-outside-big.map: nconf
	./nconf -mim elegant-cat3.png -cbo 51 1005 -cm -sm elegant-cat-outside-big.map

big-cat.png: nconf
	./nconf \
	  -lm elegant-cat-outside-big.map -li zebra.png -zebra -period 6 -fix \
	  -lm2 elegant-cat-inside-big.map -li reptiles.png -zebra -period 3 \
	  -export big-cat.png

draw: nconf
	./nconf \
	  -lm elegant-cat-outside.map -li zebra.png -zebra -period 6 -fix \
	  -lm2 elegant-cat-inside.map -li reptiles.png -zebra -period 3 \
	  -draw

draw-map: nconf
	./nconf \
	  -lm elegant-cat-inside.map -chessmap 786 572 -draw -export chessmap.png2

draw-tree: nconf
	./nconf -qt -scale 6 -mim letter-e.png -mapin -mapout -li zebra.png -zebra -period 6 -draw

letter-p: nconf
	./nconf -qt -scale 2 -mim letter-p.png -mapat 189 378 -li zebra.png -zebra -period 6 -draw

bridges.maps: nconf
	./nconf -qt -scale 1 -mim bridges.png -mapall -sma bridges.maps

t: nconf
	./nconf -rectangle 400 200 -cm -sma t.maps

t2:nconf
	./nconf -lma t.maps -li p-chess.png -period 3.5 -draw

draw-bridges: nconf
	./nconf -lma bridges.maps \
	  -side 0 -killside \
	  -side 2 -li p-chess.png -p46 -period 1 \
	  -side 11 -li p-chess.png -p46 -period 1 \
	  -side 5 -li p45.png -p45 -period 1 \
	  -side 8 -li poincare.png -zebra -period 1 \
	  -side 12 -killside \
	  -side 13 -killside \
	  -side 14 -killside \
	  -side 15 -killside \
	  -side 1 -li p-icy.png -zebra -period 1 -fix \
	  -side 3 -li p-redrock.png -zebra -period 1 -fix \
	  -side 6 -li p-warped.png -zebra -period 1 -fix \
	  -side 9 -li reptiles.png -zebra -period 3 -fix \
	  -lineout 5 -ntswhite -btblack \
	  -draw -export bridgesmap.png

#	  -side 5 -li rosegarden.png -zebra -period 1 \
#	  -side 8 -li windy.png -zebra -period 9 \
#	  -side 2 -li zebra.png -zebra -period 6 \
#         -side 11 -li zebrabright.png -zebra -period 6 \

z:

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

# note: join points incorrect!

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

maze.maps: nconf
	./nconf -qt -scale 0.02 -margin 0 -mim maze.png -mapall -sma maze.maps

draw-maze: nconf
	./nconf -lma maze.maps \
	 -side 0 -li windy.png -zebra -period 9 -fix \
	 -side 1 -li poincare.png -zebra -period 1 -fix \
	 -side 9 -li zebra.png -zebra -period 6 \
	 -draw -export big-maze-mapped.png

maze2.maps: nconf
	time ./nconf -scale 1 -joinparams 3e-3 40 .1 -qt -margin 0 -mim maze2.png -mapat 10 10 -sma maze2.maps

maze-cvl.txt: nconf maze2.maps
	time ./nconf -lma maze2.maps -cvlgen maze-cvl.txt

mcvl/maze-00-000.png: maze-cvl.txt
	mkdir -p mcvl
	rm -f mcvl/*.png
	$(HYPER) -nogui -C -each 10 -shotxy 800 800 -wm 3 -mm 2 -smart 1 -cvlbuild maze-cvl.txt -cvldraw mcvl/maze-%02d-%03d.png

mcvl-gen: mcvl/maze-00-000.png

maze-cvl.png: mcvl-gen nconf
	./nconf -ntsblack -lma maze2.maps -cvlimg mcvl/maze-%02d-%03d.png -export maze-cvl.png

letter-e.maps: nconf
	./nconf -qt -scale 0.5 -joinoff -mim letter-e.png -mapat 185 229 -sma letter-e.maps

draw-letter-e: nconf #letter-e.maps
	./nconf -lma letter-e.maps \
	  -side 1 -li zebra.png -zebra -period 6 \
	  -side 0 -li reptiles.png -zebra -period 3 -fix \
	  -draw

hilbert.map: nconf
	./nconf -hilbert 3 64 2 -sb hmap.txt -cm -sm hilbert.map

hilbert-big.map: nconf
	./nconf -qt -hilbert 4 128 4 -sb hmap.txt -cm -sm hilbert-big.map

# needs hilbert.map
draw-hilbert-chaos: nconf
	./nconf \
	  -lm hilbert.map -lband chaos1.png -lband chaos2.png -lband chaos3.png \
	  -draw

draw-hilbert-std: nconf
	./nconf \
	  -lm hilbert.map -lbands 1 11 std%d.png  \
	  -draw

hilbert-std.png: nconf
	./nconf \
	  -lm hilbert-big.map -lbands 1 11 std%d.png  \
	  -export hilbert-std.png

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

diamond500.map: nconf
	./nconf -diamond 500 -cm -sm diamond500.map

quincunx: nconf
	./nconf -lm diamond500.map -li i1.png -quincunx -li i3.png  -draw -export i4.png

triangle-earth: nconf
	./nconf -lm triangle400.map -li earthspin.png -tm -export triangles/tearth.png

triangle-earth-draw: nconf
	./nconf -lm triangle400.map -li earthspin.png -tm -draw

cheetah.maps: cheetah.png
	./nconf -mim cheetah.png -joinoff -mapat 368 136 -sma cheetah.maps -draw

draw-cheetah: nconf cheetah.maps
	./nconf -ntswhite -btblack -lineout 3 -lma cheetah.maps -cheetah 3185.png -draw

leopard.maps: nconf leopard.png
	./nconf -mim leopard2.png -joinoff -zo 3 -mapat 1524 1116 -sma leopard.maps -draw

draw-leopard: nconf
	./nconf -zo 3 -ntswhite -btblack -lineout 3 -lma leopard.maps -cheetah leopard.png -draw

video-leopard: nconf
	./nconf -zo 3 -ntswhite -btblack -lineout 3 -lma leopard.maps -cheetah leopard.png -excheetah 50 cheetah/%02d.png

ex-leopard: nconf
	./nconf -ntswhite -btblack -lineout 3 -lma leopard.maps -li poincare.png -zebra -period 1 -export draw-leopard.png

debug: nconf-debug
	gdb nconf-debug
