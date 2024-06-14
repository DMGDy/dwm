#include "tooltip.h"

#define MAX_LINE_LENGTH 24
Display
*init_display() {
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Failed to open display\n");
    exit(1);
  }
  return display;
}

XFontStruct 
*load_font(Display *display) {
  XFontStruct *font = XLoadQueryFont(display,"-misc-firacode nerd font ret-medium-*-*-*-19-*-*-*-*-*-*-*");
  if (font == NULL) {
    fprintf(stderr, "Failed to load font\n");
    exit(1);
  }
  return font;
}

void 
get_mouse_position(Display *display, Window root, int *mouse_x, int *mouse_y) {
  Window root_return, child_return;
  int root_x, root_y;
  unsigned int mask_return;

  XQueryPointer(display, root, &root_return, &child_return, mouse_x, mouse_y, &root_x, &root_y, &mask_return);
}

Window create_tooltip_window(Display *display, Window root, int x, int y, int width, int height, XVisualInfo *vinfo) {
  Colormap colormap = XCreateColormap(display, root, vinfo->visual, AllocNone);

  XSetWindowAttributes attributes;
  attributes.colormap = colormap;
  attributes.background_pixel = 0;
  attributes.border_pixel = 0;
  attributes.override_redirect = True;


  Window tooltip = XCreateWindow(display, root, x, y, width, height, 0, vinfo->depth, InputOutput, vinfo->visual, CWColormap | CWBackPixel | CWBorderPixel | CWOverrideRedirect, &attributes);

  XRectangle rect;
  rect.x = 0;
  rect.y = 0;
  rect.height = height;
  rect.width = width;

  XShapeCombineRectangles(display, tooltip, ShapeBounding, 0, 0, &rect, 1, ShapeSet, Unsorted);

  return tooltip;
  return tooltip;
}

Window
create_input_window(Display *display, Window root) {
  XSetWindowAttributes input_attributes;
  input_attributes.override_redirect = True;
  input_attributes.event_mask = ButtonPressMask;

  Window input_window = XCreateWindow(display, root, 0, 0, DisplayWidth(display, DefaultScreen(display)), DisplayHeight(display, DefaultScreen(display)), 0, CopyFromParent, InputOnly, CopyFromParent, CWOverrideRedirect | CWEventMask, &input_attributes);
  return input_window;
}

GC create_gc(Display *display, Window window, XFontStruct *font) {
  XGCValues values;
  GC gc = XCreateGC(display, window, 0, &values);
  XSetFont(display, gc, font->fid);
  return gc;
}

void
split_text_into_lines(const char *text, int max_length,
    char lines[][max_length + 1], int *num_lines) {
  int len = strlen(text);
  int line_index = 0;
  int char_index = 0;

  for (int i = 0; i < len; i++) {
    if (char_index >= max_length) {
      lines[line_index][char_index] = '\0';
      line_index++;
      char_index = 0;
    }
    lines[line_index][char_index] = text[i];
    char_index++;
  }
  lines[line_index][char_index] = '\0';
  *num_lines = line_index + 1;
}

void 
handle_events(Display *display, Window tooltip, Window input_window, GC gc,
    int width, int height, XFontStruct *font, Picture pict,char* text) {
  XEvent event;
  while (1) {
    XNextEvent(display, &event);

    if (event.type == Expose && event.xexpose.window == tooltip) {
      XRenderColor bg_color = {0x0000, 0x0000, 0x0000, (int)(TOOLTIP_OPACITY * 0xFFFF)};
      XRenderFillRectangle(display, PictOpOver, pict, &bg_color, 0, 0, width, height);

      XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
      //XDrawString(display, tooltip, gc, 5, font->ascent + 5, text, strlen(text));

      const int max_length = MAX_LINE_LENGTH;
      char lines[10][max_length + 1];
      int num_lines = 0;
      split_text_into_lines(text,max_length,lines,&num_lines);
      for (int i = 0; i < num_lines;++i) {
        XDrawString(display, tooltip, gc, 5, font->ascent + 5 + i * 
            (font->ascent + font->descent), lines[i], strlen(lines[i]));
      }
    } else if (event.type == ButtonPress && event.xbutton.window == input_window) {
      break;
    }
  }
}

void cleanup(Display *display, Window tooltip, Window input_window, GC gc, XFontStruct *font) {
  XFreeFont(display, font);
  XFreeGC(display, gc);
  XDestroyWindow(display, tooltip);
  XDestroyWindow(display, input_window);
  XCloseDisplay(display);
}

int 
create_tooltip(char* text) {
  Display *display = init_display();
  Window root = RootWindow(display, DefaultScreen(display));

  XFontStruct *font = load_font(display);

  int mouse_x, mouse_y;
  get_mouse_position(display, root, &mouse_x, &mouse_y);

  int tooltip_width = XTextWidth(font, text, MAX_LINE_LENGTH) + 10; // 20 is the max length per line
  int tooltip_height = (font->ascent + font->descent + 5) * ((strlen(text) / MAX_LINE_LENGTH) + 1) + 10;
  XVisualInfo vinfo;
  if (!XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &vinfo)) {
    fprintf(stderr, "No matching visual found\n");
    exit(1);
  }

  Window tooltip = create_tooltip_window(display, root, mouse_x, 
      mouse_y, tooltip_width, tooltip_height, &vinfo);
  Window input_window = create_input_window(display, root);

  XRenderPictFormat *pict_format = XRenderFindVisualFormat(display, vinfo.visual);
  Picture pict = XRenderCreatePicture(display, tooltip, pict_format, 0, NULL);

  XSelectInput(display, tooltip, ExposureMask);
  XMapWindow(display, tooltip);
  XMapWindow(display, input_window);

  GC gc = create_gc(display, tooltip, font);

  handle_events(display, tooltip, input_window, gc, tooltip_width, 
      tooltip_height, font, pict,text);

  cleanup(display, tooltip, input_window, gc, font);

  return 0;
}

