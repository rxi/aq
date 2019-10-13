#!/bin/bash

atlas atlas\
  -f "  [ %s ] = { %d, %d, %d, %d },"\
  -g "%s+%d"\
  -p 1\
  -t atlas.txt\
  -o atlas.png\
  -r 32-127\
  -s 16

rm src/atlas.inl
convert atlas.png -channel A -separate -depth 8 gray:atlas_texture

echo "enum { ATLAS_WHITE = MU_ICON_MAX, ATLAS_FONT };" >> src/atlas.inl
echo "enum { ATLAS_WIDTH = 128, ATLAS_HEIGHT = 128 };" >> src/atlas.inl

echo ""                           >> src/atlas.inl
echo -n "static "                 >> src/atlas.inl
xxd -i atlas_texture | head -n -1 >> src/atlas.inl
echo ""                           >> src/atlas.inl
echo "static mu_Rect atlas[] = {" >> src/atlas.inl
cat atlas.txt                     >> src/atlas.inl
echo "};"                         >> src/atlas.inl

rm atlas_texture
rm atlas.png
rm atlas.txt
