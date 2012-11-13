#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// #include <fcntl.h>
#include <dirent.h>
#include <fnmatch.h>

#include "vgwrap.h"

int screenWidth, screenHeight;


typedef struct _CenteredScaledImage {
  VGImage img;
  VGfloat imageHeight;
  VGfloat imageWidth;
  // scaling ratios for each dimension, smallest indicates the dominant axis
  // for scaling
  VGfloat scaleX;
  VGfloat scaleY;

  // final scaling factor used in setting up the transform
  VGfloat finalScale;
  VGfloat offsetX;
  VGfloat offsetY;
} CenteredScaledImage;


CenteredScaledImage * LoadScaledImage(char * filename)
{
  CenteredScaledImage * csv = (CenteredScaledImage *) malloc(sizeof(CenteredScaledImage));
  
  csv->img = createImageFromJpeg(filename);

  // calculate transform to make the image appear scaled and centered
  // on screen
  VGfloat imageHeight, imageWidth;
  VGfloat screenWidthf = (VGfloat) screenWidth;
  VGfloat screenHeightf = (VGfloat) screenHeight;
  imageWidth = (VGfloat) vgGetParameteri(csv->img, VG_IMAGE_WIDTH);
  csv->imageWidth = imageWidth;
  imageHeight = (VGfloat) vgGetParameteri(csv->img, VG_IMAGE_HEIGHT);
  csv->imageHeight = imageHeight;

  VGfloat scaleX, scaleY;
  scaleX = screenWidthf / imageWidth;
  csv->scaleX = scaleX;
  scaleY = screenHeightf / imageHeight;
  csv->scaleY = scaleY;

  // the smallest scale factor is the dominant dimension to use for scaling
  // both axes, since we want to preserve the aspect ratio
  VGfloat finalScale;
  VGfloat translateX, translateY;
  if (scaleY < scaleX) {
    finalScale = scaleY;
    // we're scaling more on the Y axis, so x axis gets offset
    translateY = 0;
    translateX = (screenWidthf - (imageWidth * finalScale)) / 2;
  }
  else {
    finalScale = scaleX;
    translateX = 0;
    translateY = (screenHeightf - (imageHeight * finalScale)) / 2;
  }

  csv->offsetX = translateX;
  csv->offsetY = translateY;
  csv->finalScale = finalScale;

  return csv;
}


void FreeScaledImage(CenteredScaledImage * csv)
{
  vgDestroyImage(csv->img);
  free(csv);
}

// Given an already initialized frame buffer, resets the image transform
// and draws the image.
void SetTransformAndDrawScaledImage(CenteredScaledImage * csv)
{
  SetImageToSurfaceTransform();
  vgLoadIdentity();
  Translate(csv->offsetX, csv->offsetY);
  Scale(csv->finalScale, csv->finalScale);

  vgDrawImage(csv->img);  
}



// wait for a specific character 
void waituntil(int endchar) {
    int key;

    for (;;) {
        key = getchar();
        if (key == endchar || key == '\n') {
            break;
        }
    }
}


void render_image(char * filename)
{
  Start(screenWidth, screenHeight);
  Background(0, 0, 0);

  CenteredScaledImage * csv = LoadScaledImage(filename);
  SetTransformAndDrawScaledImage(csv);
  FreeScaledImage(csv);

  End();
}


void rotate_images()
{
  DIR * dirp = opendir("images");
  struct dirent * dp;
  while ((dp = readdir(dirp)) != NULL) {
    // select for *.jpg and *.JPG files, only allow for standard files
    if (dp->d_type && DT_REG) {
      if (fnmatch("*.JPG", dp->d_name, 0) == 0 ||
	  fnmatch("*.jpg", dp->d_name, 0) == 0) {

	char buf[1024];
	strncpy(buf, "images/", sizeof(buf) - 1);
	int image_dir_length = strlen(buf);

	strncat(buf, dp->d_name, sizeof(buf) - 1 - image_dir_length);
	render_image(buf);
	sleep(5);
      }
    }
  }
  closedir(dirp);
}


int main(int argc, char ** argv)
{
  saveterm();
  vgwrap_init(&screenWidth, &screenHeight, 1);
  rawterm();

  //render_image("images/DSC01660.JPG");
  rotate_images();

  waituntil(0x1b);

  restoreterm();
  vgwrap_finish();
  return 0;
}
