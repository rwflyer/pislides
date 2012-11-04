#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "vgwrap.h"
#include "eglstate.h"

extern STATE_T * state;	// global graphics state




//
// Transformations
//

// Translate the coordinate system to x,y
void Translate(VGfloat x, VGfloat y) {
	vgTranslate(x, y);
}

// Rotate around angle r
void Rotate(VGfloat r) {
	vgRotate(r);
}

// Shear shears the x coordinate by x degrees, the y coordinate by y degrees
void Shear(VGfloat x, VGfloat y) {
	vgShear(x, y);
}

// Scale scales by  x, y
void Scale(VGfloat x, VGfloat y) {
	vgScale(x, y);
}

//
// Style functions
//

// setfill sets the fill color
void setfill(VGfloat color[4]) {
	VGPaint fillPaint = vgCreatePaint();
	vgSetParameteri(fillPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetParameterfv(fillPaint, VG_PAINT_COLOR, 4, color);
	vgSetPaint(fillPaint, VG_FILL_PATH);
	vgDestroyPaint(fillPaint);
}

// setstroke sets the stroke color
void setstroke(VGfloat color[4]) {
	VGPaint strokePaint = vgCreatePaint();
	vgSetParameteri(strokePaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
	vgSetParameterfv(strokePaint, VG_PAINT_COLOR, 4, color);
	vgSetPaint(strokePaint, VG_STROKE_PATH);
	vgDestroyPaint(strokePaint);
}

// StrokeWidth sets the stroke width
void StrokeWidth(VGfloat width) {
	vgSetf(VG_STROKE_LINE_WIDTH, width);
	vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
	vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_MITER);
}

//
// Color functions
//
//



// RGBA fills a color vectors from a RGBA quad.
void RGBA(unsigned int r, unsigned int g, unsigned int b, VGfloat a, VGfloat color[4]) {
	if (r > 255) {
		r = 0;
	}
	if (g > 255) {
		g = 0;
	}
	if (b > 255) {
		b = 0;
	}
	if (a < 0.0 || a > 1.0) {
		a = 1.0;
	}
	color[0] = (VGfloat) r / 255.0f;
	color[1] = (VGfloat) g / 255.0f;
	color[2] = (VGfloat) b / 255.0f;
	color[3] = a;
}

// RGB returns a solid color from a RGB triple
void RGB(unsigned int r, unsigned int g, unsigned int b, VGfloat color[4]) {
	RGBA(r, g, b, 1.0f, color);
}

// Stroke sets the stroke color, defined as a RGB triple.
void Stroke(unsigned int r, unsigned int g, unsigned int b, VGfloat a) {
	VGfloat color[4];
	RGBA(r, g, b, a, color);
	setstroke(color);
}

// Fill sets the fillcolor, defined as a RGBA quad.
void Fill(unsigned int r, unsigned int g, unsigned int b, VGfloat a) {
	VGfloat color[4];
	RGBA(r, g, b, a, color);
	setfill(color);
}


// setstops sets color stops for gradients
void setstop(VGPaint paint, VGfloat *stops, int n) {
	VGboolean multmode = VG_FALSE;
	VGColorRampSpreadMode spreadmode = VG_COLOR_RAMP_SPREAD_REPEAT;	
	vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, spreadmode);
	vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_PREMULTIPLIED, multmode);
	vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 5*n, stops);
	vgSetPaint(paint, VG_FILL_PATH);
}

// LinearGradient fills with a linear gradient
void FillLinearGradient(VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2, VGfloat *stops, int ns) {
	VGfloat lgcoord[4] = {x1, y1, x2, y2};
	VGPaint paint = vgCreatePaint();
	vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
	vgSetParameterfv(paint, VG_PAINT_LINEAR_GRADIENT, 4, lgcoord);
	setstop(paint, stops, ns);
	vgDestroyPaint(paint);	
}

// RadialGradient fills with a linear gradient
void FillRadialGradient(VGfloat cx, VGfloat cy, VGfloat fx, VGfloat fy, VGfloat radius, VGfloat *stops, int ns) {
	VGfloat radialcoord[5] = {cx, cy, fx, fy, radius};
	VGPaint paint = vgCreatePaint();
	vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
	vgSetParameterfv(paint, VG_PAINT_RADIAL_GRADIENT, 5, radialcoord);
	setstop(paint, stops, ns);
	vgDestroyPaint(paint);	
}


//
// Shape functions
//

// newpath creates path data
VGPath newpath() {
	return vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL);
}

// makecurve makes path data using specified segments and coordinates
void makecurve(VGubyte * segments, VGfloat * coords) {
	VGPath path = newpath();
	vgAppendPathData(path, 2, segments, coords);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
	vgDestroyPath(path);
}

// CBezier makes a quadratic bezier curve
void Cbezier(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat px, VGfloat py, VGfloat ex, VGfloat ey) {
	VGubyte segments[] = { VG_MOVE_TO_ABS, VG_CUBIC_TO };
	VGfloat coords[] = { sx, sy, cx, cy, px, py, ex, ey };
	makecurve(segments, coords);
}

// QBezier makes a quadratic bezier curve
void Qbezier(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat ex, VGfloat ey) {
	VGubyte segments[] = { VG_MOVE_TO_ABS, VG_QUAD_TO };
	VGfloat coords[] = { sx, sy, cx, cy, ex, ey };
	makecurve(segments, coords);
}

// interleave interleaves arrays of x, y into a single array
void interleave(VGfloat * x, VGfloat * y, int n, VGfloat * points) {
	while (n--) {
		*points++ = *x++;
		*points++ = *y++;
	}
}

// poly makes either a polygon or polyline
void poly(VGfloat * x, VGfloat * y, VGint n, VGbitfield flag) {
	VGfloat points[n * 2];
	VGPath path = newpath();
	interleave(x, y, n, points);
	vguPolygon(path, points, n, VG_FALSE);
	vgDrawPath(path, flag);
	vgDestroyPath(path);
}

// Polygon makes a filled polygon with vertices in x, y arrays
void Polygon(VGfloat * x, VGfloat * y, VGint n) {
	poly(x, y, n, VG_FILL_PATH);
}

// Polyline makes a polyline with vertices at x, y arrays
void Polyline(VGfloat * x, VGfloat * y, VGint n) {
	poly(x, y, n, VG_STROKE_PATH);
}

// Rect makes a rectangle at the specified location and dimensions
void Rect(VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	VGPath path = newpath();
	vguRect(path, x, y, w, h);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
	vgDestroyPath(path);
}

// Line makes a line from (x1,y1) to (x2,y2)
void Line(VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2) {
	VGPath path = newpath();
	vguLine(path, x1, y1, x2, y2);
	vgDrawPath(path, VG_STROKE_PATH);
	vgDestroyPath(path);
}

// Roundrect makes an rounded rectangle at the specified location and dimensions
void Roundrect(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat rw, VGfloat rh) {
	VGPath path = newpath();
	vguRoundRect(path, x, y, w, h, rw, rh);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
	vgDestroyPath(path);
}

// Ellipse makes an ellipse at the specified location and dimensions
void Ellipse(VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	VGPath path = newpath();
	vguEllipse(path, x, y, w, h);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
	vgDestroyPath(path);
}

// Circle makes a circle at the specified location and dimensions
void Circle(VGfloat x, VGfloat y, VGfloat r) {
	Ellipse(x, y, r, r);
}

// Arc makes an elliptical arc at the specified location and dimensions
void Arc(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sa, VGfloat aext) {
	VGPath path = newpath();
	vguArc(path, x, y, w, h, sa, aext, VGU_ARC_OPEN);
	vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH);
	vgDestroyPath(path);
}

// Start begins the picture, clearing a rectangular region with a specified color
void Start(int width, int height) {
	VGfloat color[4] = { 255, 255, 255, 1 };
	vgSetfv(VG_CLEAR_COLOR, 4, color);
	vgClear(0, 0, width, height);
	color[0] = 0, color[1] = 0, color[2] = 0;
	setfill(color);
	setstroke(color);
	StrokeWidth(0);
	vgLoadIdentity();
}

// End checks for errors, and renders to the display
void End() {
	assert(vgGetError() == VG_NO_ERROR);
	eglSwapBuffers(state->display, state->surface);
	assert(eglGetError() == EGL_SUCCESS);
}

// dumpscreen writes the raster
void dumpscreen(int w, int h, FILE * fp) {
	void *ScreenBuffer = malloc(w * h * 4);
	vgReadPixels(ScreenBuffer, (w * 4), VG_sABGR_8888, 0, 0, w, h);
	fwrite(ScreenBuffer, 1, w * h * 4, fp);
	free(ScreenBuffer);
}

// SaveEnd dumps the raster before rendering to the display 
void SaveEnd(char *filename) {
	FILE *fp;
	assert(vgGetError() == VG_NO_ERROR);
	if (strlen(filename) == 0) {
		dumpscreen(state->screen_width, state->screen_height, stdout);
	} else {
		fp = fopen(filename, "wb");
		if (fp != NULL) {
			dumpscreen(state->screen_width, state->screen_height, fp);
			fclose(fp);
		}
	}
	eglSwapBuffers(state->display, state->surface);
	assert(eglGetError() == EGL_SUCCESS);
}

// clear the screen to a solid background color
void Background(unsigned int r, unsigned int g, unsigned int b) {
	Fill(r, g, b, 1);
	Rect(0, 0, state->screen_width, state->screen_height);
}

// clear the screen to a background color with alpha
void BackgroundRGB(unsigned int r, unsigned int g, unsigned int b, VGfloat a) {
	Fill(r, g, b, a);
	Rect(0, 0, state->screen_width, state->screen_height);
}
