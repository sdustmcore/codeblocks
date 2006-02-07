rm -f tmp/*.png
rm -f output/*.png

set SHARP=hamming
set ANTIALIASED=cubic
set CFLAGS=-strip -antialias

rem **** Windows splash screen ****

convert %CFLAGS% -filter %ANTIALIASED% -geometry 500 curve.png  tmp/splash_new1.png
convert %CFLAGS% -filter %SHARP%       -geometry 500 curve.png tmp/splash_new2.png

convert %CFLAGS% -crop 214x24+130+93+ tmp/splash_new2.png tmp/crop1.png
convert %CFLAGS% -crop 141x24+166+117 tmp/splash_new2.png tmp/crop2.png

convert %CFLAGS% -draw "image over 130,93 0,0 tmp/crop1.png" -draw "image over 166,117 0,0 tmp/crop2.png" tmp/splash_new1.png tmp/final.png

convert %CFLAGS% tmp/final.png ( +clone -background black -shadow 45x8+6+8 ) +swap -background none -flatten +repage -depth 8 tmp/final.png

pngcrush -brute -q tmp/final.png output/splash_new.png


rem **** Unix splash screen ****

convert %CFLAGS% -filter %ANTIALIASED% -geometry 460 block.png  tmp/splash1.png
convert %CFLAGS% -filter %SHARP%       -geometry 460 block.png  tmp/splash2.png

convert %CFLAGS% -crop 260x25+120+60 tmp/splash2.png tmp/crop1.png
convert %CFLAGS% -crop 155x22+173+82 tmp/splash2.png tmp/crop2.png

convert %CFLAGS% -draw "image over 120,60 0,0 tmp/crop1.png" -draw "image over 173,82 0,0 tmp/crop2.png" tmp/splash1.png tmp/final.png

convert %CFLAGS% tmp/final.png -depth 8 tmp/final.png

pngcrush -brute -q tmp/final.png output/splash.png


rem **** start_here title ****

convert %CFLAGS% -filter %ANTIALIASED% -geometry 415 curve.png  tmp/title1.png
convert %CFLAGS% -filter %SHARP%       -geometry 415 curve.png  tmp/title2.png

convert %CFLAGS% -crop 176x20+110+78+ tmp/title2.png tmp/crop1.png
convert %CFLAGS% -crop 118x20+138+96 tmp/title2.png tmp/crop2.png

convert %CFLAGS% -draw "image over 110,78 0,0 tmp/crop1.png" -draw "image over 138,96 0,0 tmp/crop2.png" tmp/title1.png tmp/final.png

convert %CFLAGS% tmp/final.png ( +clone -background black -shadow 50x6+5+6 ) +swap -background white -flatten +repage -depth 8 tmp/final.png

pngcrush -brute -q tmp/final.png output/title.png

