#!/bin/bash

# Scan for and fix bogus file extensions
find -iname "*.jpeg" -print

# Scan for and delete bogus filename turds left by MacOS
find -name "._*.jpg" -print

# Recursive resize
find -iname "*.jpg" -exec mogrify -resize 1920x1024 {} \;

# Recursive iOS image rotation fixups
find -iname "*.jpg" -exec jhead -autorot {} \;

#mogrify -resize 1920x1024 *.jpg
#jhead -autorot *.jpg
