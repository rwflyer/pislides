//
// libshapes: high-level OpenVG API
// Anthony Starks (ajstarks@gmail.com)
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vgwrap.h"

#ifdef VGWRAP_INCLUDE_FONTS

static const int MAXFONTPATH = 256;

#include "fonts/DejaVuSans.inc"	// font data
#include "fonts/DejaVuSerif.inc"
#include "fonts/DejaVuSansMono.inc"

Fontinfo SansTypeface, SerifTypeface, MonoTypeface;

void LoadAllFonts()
{
  SansTypeface = load_font_from_data(DejaVuSans_glyphPoints,
				     DejaVuSans_glyphPointIndices,
				     DejaVuSans_glyphInstructions,
				     DejaVuSans_glyphInstructionIndices,
				     DejaVuSans_glyphInstructionCounts,
				     DejaVuSans_glyphAdvances,
				     DejaVuSans_characterMap,
				     DejaVuSans_glyphCount);

  SerifTypeface = load_font_from_data(DejaVuSerif_glyphPoints,
				      DejaVuSerif_glyphPointIndices,
				      DejaVuSerif_glyphInstructions,
				      DejaVuSerif_glyphInstructionIndices,
				      DejaVuSerif_glyphInstructionCounts,
				      DejaVuSerif_glyphAdvances,
				      DejaVuSerif_characterMap,
				      DejaVuSerif_glyphCount);

  MonoTypeface = load_font_from_data(DejaVuSansMono_glyphPoints,
				     DejaVuSansMono_glyphPointIndices,
				     DejaVuSansMono_glyphInstructions,
				     DejaVuSansMono_glyphInstructionIndices,
				     DejaVuSansMono_glyphInstructionCounts,
				     DejaVuSansMono_glyphAdvances,
				     DejaVuSansMono_characterMap,
				     DejaVuSansMono_glyphCount);
}

void UnloadAllFonts()
{
  unload_font(SansTypeface.Glyphs, SansTypeface.Count);
  unload_font(SerifTypeface.Glyphs, SerifTypeface.Count);
  unload_font(MonoTypeface.Glyphs, MonoTypeface.Count);
}


//
// Font functions
//

// loadfont loads font path data
// derived from http://web.archive.org/web/20070808195131/http://developer.hybrid.fi/font2openvg/renderFont.cpp.txt
Fontinfo load_font_from_data(const int *Points,
			     const int *PointIndices,
			     const unsigned char *Instructions,
			     const int *InstructionIndices,
			     const int *InstructionCounts,
			     const int *adv, const short *cmap, int ng)
{
	Fontinfo f;
	int i;

	memset(f.Glyphs, 0, MAXFONTPATH * sizeof(VGPath));
	if (ng > MAXFONTPATH) {
		return f;
	}
	for (i = 0; i < ng; i++) {
		const int *p = &Points[PointIndices[i] * 2];
		const unsigned char *instructions = &Instructions[InstructionIndices[i]];
		int ic = InstructionCounts[i];
		VGPath path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_S_32,
					   1.0f / 65536.0f, 0.0f, 0, 0,
					   VG_PATH_CAPABILITY_ALL);
		f.Glyphs[i] = path;
		if (ic) {
			vgAppendPathData(path, ic, instructions, p);
		}
	}
	f.CharacterMap = cmap;
	f.GlyphAdvances = adv;
	f.Count = ng;
	return f;
}

// unloadfont frees font path data
void unload_font(VGPath * glyphs, int n) {
	int i;
	for (i = 0; i < n; i++) {
		vgDestroyPath(glyphs[i]);
	}
}


// Text renders a string of text at a specified location, size, using the specified font glyphs
// derived from http://web.archive.org/web/20070808195131/http://developer.hybrid.fi/font2openvg/renderFont.cpp.txt
void Text(VGfloat x, VGfloat y, char *s, Fontinfo f, int pointsize) {
	VGfloat size = (VGfloat) pointsize, xx = x, mm[9];
	int i;

	vgGetMatrix(mm);
	for (i = 0; i < (int)strlen(s); i++) {
		unsigned int character = (unsigned int)s[i];
		int glyph = f.CharacterMap[character];
		if (glyph == -1) {
			continue;	//glyph is undefined
		}
		VGfloat mat[9] = {
			size, 0.0f, 0.0f,
			0.0f, size, 0.0f,
			xx, y, 1.0f
		};
		vgLoadMatrix(mm);
		vgMultMatrix(mat);
		vgDrawPath(f.Glyphs[glyph], VG_FILL_PATH);
		xx += size * f.GlyphAdvances[glyph] / 65536.0f;
	}
	vgLoadMatrix(mm);
}

// TextWidth returns the width of a text string at the specified font and size.
VGfloat TextWidth(char *s, Fontinfo f, int pointsize) {
	int i;
	VGfloat tw = 0.0;
	VGfloat size = (VGfloat) pointsize;
	for (i = 0; i < (int)strlen(s); i++) {
		unsigned int character = (unsigned int)s[i];
		int glyph = f.CharacterMap[character];
		if (glyph == -1) {
			continue;	//glyph is undefined
		}
		tw += size * f.GlyphAdvances[glyph] / 65536.0f;
	}
	return tw;
}

// TextMid draws text, centered on (x,y)
void TextMid(VGfloat x, VGfloat y, char *s, Fontinfo f, int pointsize) {
	VGfloat tw = TextWidth(s, f, pointsize);
	Text(x - (tw / 2.0), y, s, f, pointsize);
}

// TextEnd draws text, with its end aligned to (x,y)
void TextEnd(VGfloat x, VGfloat y, char *s, Fontinfo f, int pointsize) {
	VGfloat tw = TextWidth(s, f, pointsize);
	Text(x - tw, y, s, f, pointsize);
}


#endif
