#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "vgwrap.h"
#include "eglstate.h"		// data structures for graphics state

STATE_T _state;
STATE_T * state = &_state;	// global graphics state

#ifdef VGWRAP_INCLUDE_FONTS
static int loaded_fonts = 0;
#endif

// init sets the system to its initial state
void vgwrap_init(int *w, int *h, int include_fonts)
{
  bcm_host_init();
  memset(state, 0, sizeof(*state));
  oglinit(state);

#ifdef VGWRAP_INCLUDE_FONTS
  if (include_fonts) {
    loaded_fonts = 1;
    LoadAllFonts();
  }
#endif

  *w = state->screen_width;
  *h = state->screen_height;
}

// finish cleans up
void vgwrap_finish()
{
#ifdef VGWRAP_INCLUDE_FONTS
  if (loaded_fonts) {
    UnloadAllFonts();
  }
#endif

  glClear(GL_COLOR_BUFFER_BIT);
  eglSwapBuffers(state->display, state->surface);
  eglMakeCurrent(state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroySurface(state->display, state->surface);
  eglDestroyContext(state->display, state->context);
  eglTerminate(state->display);
}
