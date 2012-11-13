// Get the dependencies for the OpenVG library
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "EGL/egl.h"
#include "GLES/gl.h"

#include "vgwrap_fontinfo.h"

// Initialization
extern void vgwrap_init(int * screen_width, int * screen_height, int include_fonts);
extern void vgwrap_finish();

// Terminal manipulation
extern void saveterm();
extern void restoreterm();
extern void rawterm();

// Transformations
extern void SetPathToSurfaceTransform();
extern void SetImageToSurfaceTransform();

extern void Translate(VGfloat, VGfloat);
extern void Rotate(VGfloat);
extern void Shear(VGfloat, VGfloat);
extern void Scale(VGfloat, VGfloat);

// Fonts and Text

#ifdef VGWRAP_INCLUDE_FONTS

extern Fontinfo load_font_from_data(const int * Points,
				    const int * PointIndices,
				    const unsigned char * Instructions,
				    const int * InstructionIndices,
				    const int * InstructionCounts,
				    const int * adv,
				    const short * cmap,
				    int ng);
extern void unload_font(VGPath * glyphs, int n);

void LoadAllFonts();
void UnloadAllFonts();

extern void Text(VGfloat, VGfloat, char *, Fontinfo, int);
extern void TextMid(VGfloat, VGfloat, char *, Fontinfo, int);
extern void TextEnd(VGfloat, VGfloat, char *, Fontinfo, int);
extern VGfloat TextWidth(char *, Fontinfo, int);

#endif

// Shape Rendering
extern void Cbezier(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
extern void Qbezier(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
extern void Polygon(VGfloat *, VGfloat *, VGint);
extern void Polyline(VGfloat *, VGfloat *, VGint);
extern void Rect(VGfloat, VGfloat, VGfloat, VGfloat);
extern void Line(VGfloat, VGfloat, VGfloat, VGfloat);
extern void Roundrect(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);
extern void Ellipse(VGfloat, VGfloat, VGfloat, VGfloat);
extern void Circle(VGfloat, VGfloat, VGfloat);
extern void Arc(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat);

// Painting
extern void Background(unsigned int, unsigned int, unsigned int);
extern void BackgroundRGB(unsigned int, unsigned int, unsigned int, VGfloat);
extern void setfill(VGfloat[4]);
extern void setstroke(VGfloat[4]);
extern void StrokeWidth(VGfloat);
extern void Stroke(unsigned int, unsigned int, unsigned int, VGfloat);
extern void Fill(unsigned int, unsigned int, unsigned int, VGfloat);
extern void RGBA(unsigned int, unsigned int, unsigned int, VGfloat, VGfloat[4]);
extern void RGB(unsigned int, unsigned int, unsigned int, VGfloat[4]);
extern void FillLinearGradient(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat *, int);
extern void FillRadialGradient(VGfloat, VGfloat, VGfloat, VGfloat, VGfloat, VGfloat *, int);

// Images
extern VGImage createImageFromJpeg(const char *filename);
extern void makeimage(VGfloat, VGfloat, int, int, VGubyte *);
extern void ImageToScreenWithoutTransform(VGfloat, VGfloat, int, int, char *);

// Rendering Buffer setup
extern void Start(int, int);
extern void End();
extern void SaveEnd(char *);
