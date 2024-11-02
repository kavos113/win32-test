#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

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
char *geom = NULL;

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
char default_geom[32];

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
    
    return 0;
}