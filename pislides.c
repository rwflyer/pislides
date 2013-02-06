// Simple app to rotate JPEGs on the Raspberry Pi using OpenVG to write directly
// to the GPU without having to run X Windows or some other slow and super-fat
// graphical environment.
//
// Intended for digital picture frame, could also be used for signage.
//
// See vgwrap for details of GPU API wrapper.
//   http://www.khronos.org/registry/vg/specs/openvg-1.1.pdf
//   http://www.khronos.org/files/openvg-quick-reference-card.pdf
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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




void render_image(char * filename)
{
  Start(screenWidth, screenHeight);
  Background(0, 0, 0);

  CenteredScaledImage * csv = LoadScaledImage(filename);
  vgSeti(VG_BLEND_MODE, VG_BLEND_SRC);
  SetTransformAndDrawScaledImage(csv);
  FreeScaledImage(csv);

  End();
}



// Traverse a directory structure, depth first, marking each containing
// folder's images together in case the slide show is to be sequential.
//
// Once we have a list of all images, we have a database which we can use
// to display randomly and to also ensure that images are always displayed at
// least once in a given rotation of the entire image set.
//

typedef struct _PhotoFileRecord {
  char * relativeFilePath;
  int directoryGroupIndex;
} PhotoFileRecord;

// The master store of photo file records
PhotoFileRecord * fileRecords = NULL;
int fileRecordCount = 0;

// used during construction
int fileRecordAllocated = 0;
int currentDirectoryGroup;

// an array of indexes into the photo file record which is the playback
// order for random playback.  This array is populated once per cycle and
// is always fileRecordCount in size.
int * randomPlaybackOrderArray = NULL;


void InitFileRecords()
{
  int i;
  if (fileRecords) {
    // free all filepath strings
    PhotoFileRecord * curRecord = fileRecords;
    for (i = 0; i < fileRecordCount; i++) {
      free(curRecord->relativeFilePath);
      curRecord++;
    }
    free((void *) fileRecords);
    fileRecords = NULL;
  }
  fileRecordCount = 0;
  fileRecordAllocated = 0;
  currentDirectoryGroup = 0;
}

// Adds a new file record. relativeFilePath memory becomes owned.
void AddFileRecord(char * relativeFilePath)
{
#define FILE_RECORD_ARRAY_GROWTH 50
  if (fileRecordCount == fileRecordAllocated) {
    fileRecordAllocated += FILE_RECORD_ARRAY_GROWTH;
    fileRecords = (PhotoFileRecord *) realloc(fileRecords, fileRecordAllocated * sizeof(PhotoFileRecord));
  }

  PhotoFileRecord * curRec = fileRecords + fileRecordCount;
  fileRecordCount++;
  curRec->relativeFilePath = relativeFilePath;
  curRec->directoryGroupIndex = currentDirectoryGroup;
}


// meant to be called recursively.  relativeDirPath should *not* end in a
// trailing slash.
void ScanImageDirectory(char * relativeDirPath)
{
  // array accumulator for paths of subdirs that need to be processed
  // recursively.  We process them sequentially at the end so as to avoid
  // messing up the current directory count and to keep the entries in the file
  // record array grouped together.
  char ** childDirsArray = NULL;
  int childDirsAllocated = 0;
  int childDirsCount = 0;

  DIR * dirp = opendir(relativeDirPath);
  struct dirent * dp;
  while ((dp = readdir(dirp)) != NULL) {
    if (dp->d_type == DT_REG || dp->d_type == DT_DIR) {
      // construct the path for this entry on the heap to save away
      int dirPathLength = strlen(relativeDirPath);
      char * entryPath = malloc(dirPathLength + strlen(dp->d_name) + 2);
      strcpy(entryPath, relativeDirPath);
      *(entryPath + dirPathLength) = '/';
      strcpy(entryPath + dirPathLength + 1, dp->d_name);

      if (dp->d_type == DT_DIR) {
	if (strcmp(dp->d_name, ".") == 0 ||
	    strcmp(dp->d_name, "..") == 0) {
	  free(entryPath);
	}
	else {
	  if (childDirsAllocated == childDirsCount) {
	    childDirsAllocated += 16;
	    childDirsArray = realloc(childDirsArray, childDirsAllocated * sizeof(char *));
	  }
	  *(childDirsArray + childDirsCount) = entryPath;
	  childDirsCount++;
	}
      }
      else {
	// we only process files that have JPG or jpg extensions.  Ignore
	// all other files.
	if (fnmatch("*.JPG", dp->d_name, 0) == 0 ||
	    fnmatch("*.jpg", dp->d_name, 0) == 0) {
	  AddFileRecord(entryPath);
	}
	else {
	  free(entryPath);
	}
      }
    }
  }
  closedir(dirp);
  currentDirectoryGroup++;

  // now process the subdirs
  if (childDirsArray) {
    char ** currentChildDirPath = childDirsArray;
    int i;
    for (i = 0; i < childDirsCount; i++) {
      ScanImageDirectory(*currentChildDirPath);
      free((void *) *currentChildDirPath);
      currentChildDirPath++;
    }
    free((void *)childDirsArray);
  }
}


void InitRandomPlaybackOrder()
{
  if (randomPlaybackOrderArray) {
    free(randomPlaybackOrderArray);
  }

  randomPlaybackOrderArray = malloc(sizeof(int) * fileRecordCount);

  int i;
  int * curIndex = randomPlaybackOrderArray;
  for (i = 0; i < fileRecordCount; i++) {
    *curIndex = i;
    curIndex++;
  }

  // implementation of the Durstenfeld version of the Fisher-Yates shuffle
  for (i = fileRecordCount - 1; i > 0; i--) {
    // pick random element // 0 <= k <= i
    int k = rand() % (i + 1);
    int temp = randomPlaybackOrderArray[k];
    randomPlaybackOrderArray[k] = randomPlaybackOrderArray[i];
    randomPlaybackOrderArray[i] = temp;
  }

#ifdef BOGUS_WAY
  // array that contains the indexes of unused photos.  This array is
  // always kept compacted and is how we select the next index for the
  // playback array, to avoid having to spin the random generator multiple
  // times to find an unallocated entry.
  int unusedArraySize = fileRecordCount;
  int * unusedPhotoIndexes = malloc(sizeof(int) * fileRecordCount);

  int selectedItem;
  int i, j;
  int * curIndex = unusedPhotoIndexes;
  for (i = 0; i < fileRecordCount; i++) {
    *curIndex = i;
    curIndex++;
  }
  
  curIndex = randomPlaybackOrderArray;
  for (i = 0; i < fileRecordCount; i++) {
    // select a random item from the unused array, then compact the array
    selectedItem = rand() % unusedArraySize;
    *curIndex++ = *(unusedPhotoIndexes + selectedItem);
    
    for (j = selectedItem; j < unusedArraySize - 1; j++) {
      *(unusedPhotoIndexes + j) = *(unusedPhotoIndexes + j + 1);
    }
    unusedArraySize--;
  }

  free(unusedPhotoIndexes);
#endif

}


#if 0

void PrintRandomPlaybackOrder()
{
  printf("Random playback order for %d files:\n", fileRecordCount);
  int i;
  int * curIndex = randomPlaybackOrderArray;
  for (i = 0; i < fileRecordCount; i++) {
    printf("%d: %d\n", i, *curIndex++);
  }
  printf("\n");
}


void PrintFileRecords()
{
  PhotoFileRecord * fileRec = fileRecords;
  int i;

  for (i = 0; i < fileRecordCount; i++) {
    printf("%d: %s %d\n", i, fileRec->relativeFilePath, fileRec->directoryGroupIndex);
    fileRec++;
  }
}

#endif

#ifdef RAW_TERMINAL

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

#endif


void DisplayImagesInPlaybackOrder()
{
  int i;
  PhotoFileRecord * selectedPhoto;
  int imageIndexToDisplay;
  for (i = 0; i < fileRecordCount; i++) {
    imageIndexToDisplay = *(randomPlaybackOrderArray + i);
    selectedPhoto = fileRecords + imageIndexToDisplay;
    render_image(selectedPhoto->relativeFilePath);
    sleep(12);
  }
}


void IntSignalHandler(int sig, siginfo_t *siginfo, void * context)
{
#ifdef RAW_TERMINAL
  restoreterm();
#endif
  vgwrap_finish();
  exit(0);
}


int main(int argc, char ** argv)
{
  srand(time(NULL));

  InitFileRecords();
  ScanImageDirectory("images");

#ifdef RAW_TERMINAL
  saveterm();
  rawterm();
#endif
  vgwrap_init(&screenWidth, &screenHeight, 1);

  while (1) {
    InitRandomPlaybackOrder();
    DisplayImagesInPlaybackOrder();
  }

#ifdef RAW_TERMINAL
  restoreterm();
#endif
  vgwrap_finish();

  return 0;
}
