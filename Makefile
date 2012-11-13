OBJDIR=build
CC = gcc
# Add -DVGWRAP_INCLUDE_FONTS to get font support
CFLAGS = -O2 -Wall -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads

VGWRAP_SRCS = oglinit.c vgwrap_render.c vgwrap_terminal.c vgwrap_fonts.c vgwrap_init.c vgwrap_images.c

SRCS = pislides.c $(VGWRAP_SRCS)

OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

.PHONY: clean

all: pislides


# oglinit.o:	oglinit.c eglstate.h


pislides.o:	pislides.c vgwrap.h


pislides:	$(OBJS)
	gcc $(CFLAGS) -o pislides $(OBJS) -L/opt/vc/lib -lGLESv2 -ljpeg


clean:
	$(RM) $(OBJDIR)/*.o *~ pislides

font2openvg:	font2openvg.cpp
	g++ -I/usr/include/freetype2 font2openvg.cpp -o font2openvg -lfreetype

fonts:	font2openvg
	for f in /usr/share/fonts/truetype/ttf-dejavu/*.ttf; do fn=`basename $$f .ttf`; ./font2openvg $$f $$fn.inc $$fn; done
