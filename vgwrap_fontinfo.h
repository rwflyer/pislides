typedef struct {
	const short *CharacterMap;
	const int *GlyphAdvances;
	int Count;
	VGPath Glyphs[256];
} Fontinfo;

extern Fontinfo SansTypeface;
extern Fontinfo SerifTypeface;
extern Fontinfo MonoTypeface;
