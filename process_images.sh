#!/bin/bash

mogrify -resize 1920x1024 *.jpg
jhead -autorot *.jpg
