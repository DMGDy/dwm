#ifndef TOOLTIP_H
#define TOOLTIP_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/extensions/shape.h>


#define TOOLTIP_OPACITY 0.4

int create_tooltip(char* text);

#endif
