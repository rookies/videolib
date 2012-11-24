#!/bin/bash

LIBS="-lavcodec -lavformat -lavutil -lswscale -lsfml-window -lsfml-graphics"
SRC="videolib.cpp main.cpp"

g++ -o videolib-test $LIBS $SRC
