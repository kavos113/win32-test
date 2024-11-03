#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>

#define DEFAULT_MAIN_TEXT "Hello, World!"
#define DEFAULT_EXIT_TEXT "Exit"
#define DEFAULT_BGCOLOR   "white"
#define DEFAULT_FGCOLOR   "black"
#define DEFAULT_BDWIDTH   1
#define DEFAULT_FONT      "fixed"

typedef struct XHELLO_PARAMS
{
    char *name;
    char **p_value_string;
} XHELLO_PARAMS;

char *mbgcolor = DEFAULT_BGCOLOR;
char *mfgcolor = DEFAULT_FGCOLOR;
char *mfont = DEFAULT_FONT;
char *ebgcolor = DEFAULT_BGCOLOR;
char *efgcolor = DEFAULT_FGCOLOR;
char *efont = DEFAULT_FONT;

char *mgeom_rsrc = NULL;
char *mtext_rsrc = NULL;
char *etext_rsrc = NULL;

char *display_name = NULL;

char *mtext_cline = NULL;
char *etext_cline = NULL;
char *mgeom_cline = NULL;

char *mtext = DEFAULT_MAIN_TEXT;
char *etext = DEFAULT_EXIT_TEXT;
char *mgeom = NULL;

/* 1. list of command line options */
XHELLO_PARAMS options[] =
    {
        {"-display",  &display_name},
        {"-d",        &display_name},
        {"-geometry", &mgeom_cline},
        {"-g",        &mgeom_cline},
        {"-mtext",    &mtext_cline},
        {"-m",        &mtext_cline},
        {"-etext",    &etext_cline},
        {"-e",        &etext_cline},
    };
int num_options = sizeof(options) / sizeof(XHELLO_PARAMS);

/* 2. list of X resources */
XHELLO_PARAMS resources[] =
    {
        {"background",      &mbgcolor},
        {"foreground",      &mfgcolor},
        {"font",            &mfont},
        {"geometry",        &mgeom_rsrc},
        {"text",            &mtext_rsrc},
        {"exit.background", &ebgcolor},
        {"exit.foreground", &efgcolor},
        {"exit.font",       &efont},
        {"exit.text",       &etext_rsrc},
    };
int num_resources = sizeof(resources) / sizeof(XHELLO_PARAMS);

char *app_name = "xhello";

XFontStruct *mfontstruct, *efontstruct;
unsigned long mbgpixel, mfgpixel, ebgpixel, efgpixel;

unsigned int ewidth, eheight;
int ex, ey;
int extxt, eytxt;

Display *p_display;
XWMHints wmhints;
XSizeHints sizehints;
Window Main, Exit;
GC main_gc, exit_gc;
XEvent event;
int Done = 0;
char default_geom[80];

void usage();

int main(int argc, char *argv[])
{
    int i, j;
    char *tmpstr;
    Colormap def_cmap;
    XColor color;
    int bitmask;
    XGCValues gcvalues;
    XSetWindowAttributes attr;
    
    /* 1. get command line options */
    for (i = 1; i < argc; i += 2)
    {
        for (j = 0; j < num_options; j++)
        {
            if (strcmp(argv[i], options[j].name) == 0)
            {
                *(options[j].p_value_string) = argv[i + 1];
                break;
            }
        }
        
        if (j >= num_options)
        {
            usage();
        }
    }
    
    /* 2. open display */
    /* get display_name from env DISPLAY if display_name == NULL */
    if ((p_display = XOpenDisplay(display_name)) == NULL)
    {
        fprintf(stderr, "Can't open display: %s\n", XDisplayName(display_name));
        return 1;
    }
    
    /* 3. load resources */
    app_name = argv[0];
    
    for (i = 0; i < num_resources; i++)
    {
        if ((tmpstr = XGetDefault(p_display, app_name, resources[i].name)) != NULL)
        {
            *(resources[i].p_value_string) = tmpstr;
        }
    }
    
    /* 4. setting font, color */
    if ((mfontstruct = XLoadQueryFont(p_display, mfont)) == NULL)
    {
        fprintf(stderr, "Can't load font: %s\n", mfont);
        return 1;
    }
    
    if ((efontstruct = XLoadQueryFont(p_display, efont)) == NULL)
    {
        fprintf(stderr, "Can't load font: %s\n", efont);
        return 1;
    }
    
    def_cmap = DefaultColormap(p_display, DefaultScreen(p_display));
    
    if (XParseColor(p_display, def_cmap, mbgcolor, &color) == 0 ||
        XAllocColor(p_display, def_cmap, &color) == 0)
    {
        mbgpixel = WhitePixel(p_display, DefaultScreen(p_display));
    }
    else
    {
        mbgpixel = color.pixel;
    }
    
    if (XParseColor(p_display, def_cmap, mfgcolor, &color) == 0 ||
        XAllocColor(p_display, def_cmap, &color) == 0)
    {
        mfgpixel = BlackPixel(p_display, DefaultScreen(p_display));
    }
    else
    {
        mfgpixel = color.pixel;
    }
    
    if (XParseColor(p_display, def_cmap, ebgcolor, &color) == 0 ||
        XAllocColor(p_display, def_cmap, &color) == 0)
    {
        ebgpixel = WhitePixel(p_display, DefaultScreen(p_display));
    }
    else
    {
        ebgpixel = color.pixel;
    }
    
    if (XParseColor(p_display, def_cmap, efgcolor, &color) == 0 ||
        XAllocColor(p_display, def_cmap, &color) == 0)
    {
        efgpixel = BlackPixel(p_display, DefaultScreen(p_display));
    }
    else
    {
        efgpixel = color.pixel;
    }
    
    /* 5 setting size of top-level window */
    if (etext_cline != NULL)
    {
        etext = etext_cline;
    }
    else if (etext_rsrc != NULL)
    {
        etext = etext_rsrc;
    }
    
    if (mtext_cline != NULL)
    {
        mtext = mtext_cline;
    }
    else if (mtext_rsrc != NULL)
    {
        mtext = mtext_rsrc;
    }
    
    extxt = efontstruct->max_bounds.width / 2;
    eytxt = efontstruct->max_bounds.ascent + efontstruct->max_bounds.descent;
    ewidth = extxt + XTextWidth(efontstruct, etext, strlen(etext)) + 4;
    eheight = eytxt + 4;
    
    sizehints.flags = PPosition | PSize | PMinSize;
    sizehints.height = mfontstruct->max_bounds.ascent + mfontstruct->max_bounds.descent + eheight + 10;
    sizehints.min_height = sizehints.height;
    sizehints.width = XTextWidth(mfontstruct, mtext, strlen(mtext)) + 2;
    sizehints.width = (sizehints.width > ewidth) ? sizehints.width : ewidth;
    sizehints.min_width = sizehints.width;
    sizehints.x = DisplayWidth(p_display, DefaultScreen(p_display)) / 2 - sizehints.width / 2;
    sizehints.y = DisplayHeight(p_display, DefaultScreen(p_display)) / 2 - sizehints.height / 2;
    
    sprintf(default_geom, "%dx%d+%d+%d", sizehints.width, sizehints.height, sizehints.x, sizehints.y);
    mgeom = default_geom;
    
    if (mgeom_cline != NULL)
    {
        mgeom = mgeom_cline;
    }
    else if (mgeom_rsrc != NULL)
    {
        mgeom = mgeom_rsrc;
    }
    
    /* change sizehints when mgeom is specified */
    bitmask = XGeometry(p_display,
                        DefaultScreen(p_display),
                        mgeom,
                        default_geom,
                        DEFAULT_BDWIDTH,
                        mfontstruct->max_bounds.width,
                        mfontstruct->max_bounds.ascent + mfontstruct->max_bounds.descent,
                        1,
                        1,
                        &sizehints.x,
                        &sizehints.y,
                        &sizehints.width,
                        &sizehints.height
                        );
    
    if (bitmask & (XValue | YValue))
    {
        sizehints.flags |= USPosition;
    }
    
    if (bitmask & (WidthValue | HeightValue))
    {
        sizehints.flags |= USSize;
    }
    
    /* 6. create top-level window */
    Main = XCreateSimpleWindow(p_display,
                               DefaultRootWindow(p_display),
                               sizehints.x,
                               sizehints.y,
                               sizehints.width,
                               sizehints.height,
                               DEFAULT_BDWIDTH,
                               mbgpixel,
                               mfgpixel
                               );
    
    /* 7. set properties to window manager */
    XSetStandardProperties(p_display,
                            Main,
                            app_name,
                            app_name,
                            None,
                            argv,
                            argc,
                            &sizehints
                            );
    
    /* 8. create window manager hints */
    wmhints.flags = InputHint | StateHint;
    wmhints.input = False;
    wmhints.initial_state = NormalState;
    XSetWMHints(p_display, Main, &wmhints);
    
    /* 9. create GC */
    gcvalues.font = mfontstruct->fid;
    gcvalues.foreground = mfgpixel;
    gcvalues.background = mbgpixel;
    main_gc = XCreateGC(p_display, Main, GCForeground | GCBackground | GCFont, &gcvalues);
    
    /* 11. set event */
    XSelectInput(p_display, Main, ExposureMask);
    
    /* 12. Mapping Window */
    XMapWindow(p_display, Main);
    
    
    /* 13. child window */
    ex = 1;
    ey = 1;
    Exit = XCreateSimpleWindow(p_display,
                               Main,
                               ex,
                               ey,
                               ewidth,
                               eheight,
                               DEFAULT_BDWIDTH,
                               ebgpixel,
                               efgpixel
                               );
    
    XSelectInput(p_display, Exit, ExposureMask | ButtonPressMask);
    
    XMapWindow(p_display, Exit);
    
    gcvalues.font = efontstruct->fid;
    gcvalues.foreground = efgpixel;
    gcvalues.background = ebgpixel;
    exit_gc = XCreateGC(p_display, Exit, GCForeground | GCBackground | GCFont, &gcvalues);
    
    /* 14. event loop */
    while (!Done)
    {
        XNextEvent(p_display, &event);
        
        if (event.xany.window == Main)
        {
            switch (event.type)
            {
            case Expose:
                if (event.xexpose.count == 0)
                {
                    int x, y, itemp;
                    unsigned int width, height, utemp;
                    Window wtemp;
                    
                    if (XGetGeometry(p_display, Main, &wtemp, &itemp, &itemp, &width, &height, &utemp, &utemp) == 0)
                    {
                        break;
                    }
                    
                    x = (width - XTextWidth(mfontstruct, mtext, strlen(mtext))) / 2;
                    y = eheight + (height - eheight + mfontstruct->max_bounds.ascent - mfontstruct->max_bounds.descent) / 2;
                    
                    XClearWindow(p_display, Main);
                    XDrawString(p_display, Main, main_gc, x, y, mtext, strlen(mtext));
                }
                break;
            }
        }
        else if (event.xany.window == Exit)
        {
            switch (event.type)
            {
            case Expose:
                if (event.xexpose.count == 0)
                {
                    XClearWindow(p_display, Exit);
                    XDrawString(p_display, Exit, exit_gc, extxt, eytxt, etext, strlen(etext));
                }
                break;
            case ButtonPress:
                Done = 1;
                break;
            }
        }
    }
    
    /* 15. clean up */
    XFreeGC(p_display, main_gc);
    XFreeGC(p_display, exit_gc);
    XDestroyWindow(p_display, Main);
    XCloseDisplay(p_display);
    
    return 0;
}

void usage()
{
    fprintf(stderr, "Usage: xhello [-display display] [-geometry geom] [-mtext text] [-etext text]\n");
    exit(1);
}