//
// shapedemo: testbed for OpenVG APIs
// Anthony Starks (ajstarks@gmail.com)
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <fnmatch.h>

#include "VG/openvg.h"
#include "VG/vgu.h"

// this from the openvg wrapper library
#include "fontinfo.h"
#include "shapes.h"

int screenWidth, screenHeight;

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
  Image(0, 0, 1920, 1024, filename);
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
  init(&screenWidth, &screenHeight);
  rawterm();

  //render_image("images/DSC01660.JPG");
  rotate_images();

  waituntil(0x1b);

  restoreterm();
  finish();
  return 0;
}
