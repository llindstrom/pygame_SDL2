"""SDL 2 definitions
"""

cdef extern from "SDL.h":

    ctypedef unsigned int Uint32
    ctypedef unsigned short Uint16
    ctypedef int SDL_bool

    struct SDL_Rect:
        int x
        int y
        int w
        int h

    struct SDL_PixelFormat:
        Uint32 format

    struct SDL_Surface:
        SDL_PixelFormat *format
        int w
        int h
        int pitch
        void *pixels

    struct SDL_Window:
        pass

    struct SDL_Renderer:
        pass
    
    struct SDL_Texture:
        pass

    struct SDL_SysWMinfo:
        pass

    int SDL_WINDOWPOS_UNDEFINED
    int SDL_WINDOW_SHOWN
    int SDL_RENDERER_ACCELERATED
    int SDL_RENDERER_PRESENTVSYNC
   
    SDL_Window *SDL_CreateWindow(char *t, int x, int y, int w, int h, Uint32 f)
    void SDL_DestroyWindow(SDL_Window *window)
    int SDL_GetWindowBorderSize(SDL_Window win, int *t, int *l, int *b, int *r)
    float SDL_GetWindowBrightness(SDL_Window *window)
    int SDL_GetWindowDisplayIndex(SDL_Window *window)
    Uint32 SDL_GetWindowFlags(SDL_Window *window)
    int SDL_GetWindowGammaRamp(SDL_Window *win, Uint16 *r, Uint16 *g, Uint16 *b)
    SDL_bool SDL_GetWindowGrab(SDL_Window *window)
    void SDL_GetWindowMaximumSize(SDL_Window *win, int *w, int *h)
    void SDL_GetWindowMinimumSize(SDL_Window *win, int *w, int *h)
    int SDL_GetWindowOpacity(SDL_Window *win, float *opacity)
    Uint32 SDL_GetWindowPixelFormat(SDL_Window *win)
    void SDL_GetWindowPosition(SDL_Window *win, int *x, int *y)
    void SDL_GetWindowSize(SDL_Window *win, int *w, int *h)
    SDL_Surface *SDL_GetWindowSurface(SDL_Window *win)
    char *SDL_GetWindowTitle(SDL_Window *win)
    int SDL_UpdateWindowSurface(SDL_Window *win)

    SDL_Renderer *SDL_CreateRenderer(SDL_Window *win, int i, Uint32 f)
    void SDL_DestroyRenderer(SDL_Renderer *r)
    int SDL_RenderCopy(SDL_Renderer *r, SDL_Texture *t, SDL_Rect *s, SDL_Rect *d)
    int SDL_RenderClear(SDL_Renderer *r)
    void SDL_RenderPresent(SDL_Renderer *r)

    SDL_Texture *SDL_CreateTextureFromSurface(SDL_Renderer *r, SDL_Surface *s)
    void SDL_DestroyTexture(SDL_Texture *t)

    char *SDL_GetError()
    void SDL_ClearError()
    int SDL_SetError(char *fmt, ...)
