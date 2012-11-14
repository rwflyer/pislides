PiSlides
========

PiSlides is a simple slideshow display app for the Raspberry Pi that runs without
all the overhead associated with X Windows or a special build of Linux for
browser-only Kiosk modes.

PiSlides shows Jpeg files.  It does not attempt to show all file types known
to man.  If you want to use it, have your files in JPG format.

The OpenVG API set is used to draw images to the screen. OpenVG's drawing code
is hardware accelerated so image scaling is reasonably fast.  However PiSlides
currently does not use the GPU to decode the JPG file format, it uses
software and thus can be slow especially for large images directly off cameras.

Usage
-----

Create a folder tree of Jpeg image files under the images directory.
Start PiSlides (probably on the Raspberry Pi console).  The image tree
will be scanned and all images displayed, in random order, forever.
While running PiSlides flips the display into graphics mode and the
console disappears.  Images are scaled to fit onto the screen and
centered on a black background.

The display code is designed to always display every image on each
rotation through the directory tree, so there won't be images that you
rarely if ever see.

To stop the slideshow simply Ctrl-C PiSlides.  The console will be restored.

Recommendations
---------------

Raspberry Pis are memory constrained.  You can exceed this and/or
the GPU's memory capacity by attempting to display ginourmous Jpeg files
directly off the camera.  There is no reason to do this since you won't be able
to see all the detail in those wonderful 20 Megapixel images on the 1920x1024
HDMI display out that the Raspberry Pi supports.

Resizing images before running PiSlides is recommended.  Here's a handy
shell script that will do that:

    sudo apt-get install imagemagick
    cd <top of image directory tree>
    find -iname "*.jpg" -exec mogrify -resize 1920x1024 {} \;

If you see images that are not properly oriented, but they show up correctly on
your Mac or iOS device, then use this command:

    sudo apt-get install jhead
    cd <top of image directory tree>
    find -iname "*.jpg" -exec jhead -autorot {} \;

Eventually the reorientation of images will be performed automatically by
PiSlides, but not in this version.
