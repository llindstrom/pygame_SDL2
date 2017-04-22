"""SDL 2 definitions
"""

cdef extern from "SDL.h":

    # Type and constant declarations
    #

    # SDL_stdinc.h
    ctypedef unsigned char Uint8
    ctypedef unsigned short Uint16
    ctypedef unsigned int Uint32
    ctypedef int SDL_bool
    SDL_bool SDL_TRUE
    SDL_bool SDL_FALSE

    # SDL_rect.h
    struct SDL_Point:
        int x
        int y

    struct SDL_Rect:
        int x
        int y
        int w
        int h

    # SDL_pixels.h
    Uint32 SDL_PIXELFORMAT_UNKNOWN
    Uint32 SDL_PIXELFORMAT_INDEX1LSB
    Uint32 SDL_PIXELFORMAT_INDEX1MSB
    Uint32 SDL_PIXELFORMAT_INDEX4LSB
    Uint32 SDL_PIXELFORMAT_INDEX4MSB
    Uint32 SDL_PIXELFORMAT_INDEX8
    Uint32 SDL_PIXELFORMAT_RGB332
    Uint32 SDL_PIXELFORMAT_RGB444
    Uint32 SDL_PIXELFORMAT_RGB555
    Uint32 SDL_PIXELFORMAT_BGR555
    Uint32 SDL_PIXELFORMAT_ARGB4444
    Uint32 SDL_PIXELFORMAT_RGBA4444
    Uint32 SDL_PIXELFORMAT_ABGR4444
    Uint32 SDL_PIXELFORMAT_BGRA4444
    Uint32 SDL_PIXELFORMAT_ARGB1555
    Uint32 SDL_PIXELFORMAT_RGBA5551
    Uint32 SDL_PIXELFORMAT_ABGR1555
    Uint32 SDL_PIXELFORMAT_BGRA5551
    Uint32 SDL_PIXELFORMAT_RGB565
    Uint32 SDL_PIXELFORMAT_BGR565
    Uint32 SDL_PIXELFORMAT_RGB24
    Uint32 SDL_PIXELFORMAT_BGR24
    Uint32 SDL_PIXELFORMAT_RGB888
    Uint32 SDL_PIXELFORMAT_RGBX8888
    Uint32 SDL_PIXELFORMAT_BGR888
    Uint32 SDL_PIXELFORMAT_BGRX8888
    Uint32 SDL_PIXELFORMAT_ARGB8888
    Uint32 SDL_PIXELFORMAT_RGBA8888
    Uint32 SDL_PIXELFORMAT_ABGR8888
    Uint32 SDL_PIXELFORMAT_BGRA8888
    Uint32 SDL_PIXELFORMAT_ARGB2101010
    Uint32 SDL_PIXELFORMAT_RGBA32
    Uint32 SDL_PIXELFORMAT_ARGB32
    Uint32 SDL_PIXELFORMAT_BGRA32
    Uint32 SDL_PIXELFORMAT_ABGR32
    Uint32 SDL_PIXELFORMAT_YV12
    Uint32 SDL_PIXELFORMAT_IYUV
    Uint32 SDL_PIXELFORMAT_YUY2
    Uint32 SDL_PIXELFORMAT_UYVY
    Uint32 SDL_PIXELFORMAT_YVYU
    Uint32 SDL_PIXELFORMAT_NV12
    Uint32 SDL_PIXELFORMAT_NV21

    Uint32 SDL_PIXELTYPE_UNKNOWN
    Uint32 SDL_PIXELTYPE_INDEX1
    Uint32 SDL_PIXELTYPE_INDEX4
    Uint32 SDL_PIXELTYPE_INDEX8
    Uint32 SDL_PIXELTYPE_PACKED8
    Uint32 SDL_PIXELTYPE_PACKED16
    Uint32 SDL_PIXELTYPE_PACKED32
    Uint32 SDL_PIXELTYPE_ARRAYU8
    Uint32 SDL_PIXELTYPE_ARRAYU16
    Uint32 SDL_PIXELTYPE_ARRAYU32
    Uint32 SDL_PIXELTYPE_ARRAYF16
    Uint32 SDL_PIXELTYPE_ARRAYF32

    Uint32 SDL_BITMAPORDER_NONE
    Uint32 SDL_BITMAPORDER_4321
    Uint32 SDL_BITMAPORDER_1234
    Uint32 SDL_PACKEDORDER_NONE
    Uint32 SDL_PACKEDORDER_XRGB
    Uint32 SDL_PACKEDORDER_RGBX
    Uint32 SDL_PACKEDORDER_ARGB
    Uint32 SDL_PACKEDORDER_RGBA
    Uint32 SDL_PACKEDORDER_XBGR
    Uint32 SDL_PACKEDORDER_BGRX
    Uint32 SDL_PACKEDORDER_ABGR
    Uint32 SDL_PACKEDORDER_BGRA
    Uint32 SDL_ARRAYORDER_NONE
    Uint32 SDL_ARRAYORDER_RGB
    Uint32 SDL_ARRAYORDER_RGBA
    Uint32 SDL_ARRAYORDER_ARGB
    Uint32 SDL_ARRAYORDER_BGR
    Uint32 SDL_ARRAYORDER_BGRA
    Uint32 SDL_ARRAYORDER_ABGR

    Uint32 SDL_PACKEDLAYOUT_NONE
    Uint32 SDL_PACKEDLAYOUT_332
    Uint32 SDL_PACKEDLAYOUT_4444
    Uint32 SDL_PACKEDLAYOUT_1555
    Uint32 SDL_PACKEDLAYOUT_5551
    Uint32 SDL_PACKEDLAYOUT_565
    Uint32 SDL_PACKEDLAYOUT_8888
    Uint32 SDL_PACKEDLAYOUT_2101010
    Uint32 SDL_PACKEDLAYOUT_1010102

    struct SDL_Color:
        Uint8 r
        Uint8 g
        Uint8 b
        Uint8 a

    struct SDL_Palette:
        int ncolors
        SDL_Color *colors

    struct SDL_PixelFormat:
        Uint32 format
        SDL_Palette *palette
        Uint8 BitsPerPixel
        Uint8 BytesPerPixel
        Uint32 Rmask
        Uint32 Gmask
        Uint32 Bmask
        Uint32 Amask

    # SDL_surface.h
    struct SDL_Surface:
        SDL_PixelFormat *format
        int w
        int h
        int pitch
        void *pixels

    # SDL_video.h
    Uint32 SDL_WINDOW_FULLSCREEN
    Uint32 SDL_WINDOW_FULLSCREEN_DESKTOP
    Uint32 SDL_WINDOW_OPENGL
    Uint32 SDL_WINDOW_SHOWN
    Uint32 SDL_WINDOW_HIDDEN
    Uint32 SDL_WINDOW_BORDERLESS
    Uint32 SDL_WINDOW_RESIZE
    Uint32 SDL_WINDOW_MINIMIZE
    Uint32 SDL_WINDOW_MAXIMIZE
    Uint32 SDL_WINDOW_INPUT_GRABBED
    Uint32 SDL_WINDOW_INPUT_FOCUS
    Uint32 SDL_WINDOW_MOUSE_FOCUS
    Uint32 SDL_WINDOW_FOREIGN
    Uint32 SDL_WINDOW_ALLOW_HIGHDPI
    Uint32 SDL_WINDOW_MOUSE_CAPTURE
    Uint32 SDL_WINDOW_ALWAYS_ON_TOP  # 2.0.5
    Uint32 SDL_WINDOW_SKIP_TASKBAR   # 2.0.5
    Uint32 SDL_WINDOW_UTILITY        # 2.0.5
    Uint32 SDL_WINDOW_TOOLTIP        # 2.0.5
    Uint32 SDL_WINDOW_POPUP_MENU     # 2.0.5
   
    int SDL_WINDOWPOS_CENTERED
    int SDL_WINDOWPOS_UNDEFINED

    struct SDL_DisplayMode:
        Uint32 format
        int w
        int h
        int refresh_rate
        void *driverdata

    struct SDL_MessageBoxData:
        pass

    struct SDL_Window:
        pass

    ctypedef int SDL_GLContext
    ctypedef int SDL_GLattr

    ctypedef int SDL_HitTestResult
    ctypedef SDL_HitTestResult (*SDL_HitTest)(SDL_Window *, SDL_Point *, void *)

    # SDL_version.h
    struct SDL_version:
        Uint8 major
        Uint8 minor
        Uint8 patch

    # SDL_syswm.h
    ctypedef int SDL_SYSWM_TYPE
    SDL_SYSWM_TYPE SDL_SYSWM_UNKNOWN
    SDL_SYSWM_TYPE SDL_SYSWM_WINDOWS
    SDL_SYSWM_TYPE SDL_SYSWM_DIRECTFB
    SDL_SYSWM_TYPE SDL_SYSWM_COCOA
    SDL_SYSWM_TYPE SDL_SYSWM_UIKIT
    SDL_SYSWM_TYPE SDL_SYSWM_WAYLAND
    SDL_SYSWM_TYPE SDL_SYSWM_MIR
    SDL_SYSWM_TYPE SDL_SYSWM_WINRT
    SDL_SYSWM_TYPE SDL_SYSWM_ANDROID
    SDL_SYSWM_TYPE SDL_SYSWM_VIVANTE

    struct _pg_SysWMinfo_windows:
        int window
        int hdc
        int hinstance  # 2.0.6
    ctypedef int IInspectable
    struct _pg_SysWMinfo_winrt:
        IInspectable *window
    struct Display:
        pass
    ctypedef int Window
    struct _pg_SysWMinfo_x11:
        Display *display
        Window window
    struct IDirectFB:
        pass
    struct IDirectFBWindow:
        pass
    struct IDirectFBSurface:
        pass
    struct _pg_SysWMinfo_directfb:
        IDirectFB *dfb
        IDirectFBWindow *window
        IDirectFBSurface *surface
    struct NSWindow:
        pass
    struct _pg_SysWMinfo_cocoa:
        NSWindow *window
    struct UIWindow:
        pass
    struct _pg_SysWMinfo_uikit:
        NSWindow *window
        unsigned framebuffer
        unsigned colorbuffer
        unsigned resolveFramebuffer
    struct wl_display:
        pass
    struct wl_surface:
        pass
    struct wl_shell_surface:
        pass
    struct _pg_SysWMinfo_wayland:
        wl_display *display
        wl_surface *surface
        wl_shell_surface *shell_surface
    struct MirConnection:
        pass
    struct MirSurface:
        pass
    struct _pg_SysWMinfo_mir:
        MirConnection *connection
        MirSurface *surface
    struct ANativeWindow:
        pass
    ctypedef int EGLSurface
    struct _pg_SysWMinfo_android:
        ANativeWindow *window
        EGLSurface surface
    ctypedef int EGLNativeDisplayType
    ctypedef int EGLNativeWindowType
    struct _pg_SysWMinfo_vivante:
        EGLNativeDisplayType display
        EGLNativeWindowType window
    struct SDL_SysWMinfo:
        SDL_version version
        SDL_SYSWM_TYPE subsystem
        _pg_SysWMinfo_windows win
        _pg_SysWMinfo_winrt winrt
        _pg_SysWMinfo_directfb dfb
        _pg_SysWMinfo_uikit uikit
        _pg_SysWMinfo_wayland wl
        _pg_SysWMinfo_mir mir
        _pg_SysWMinfo_android android
        _pg_SysWMinfo_vivante vivante

    # SDL_render.h
    Uint32 SDL_RENDERER_SOFTWARE
    Uint32 SDL_RENDERER_ACCELERATED
    Uint32 SDL_RENDERER_PRESENTVSYNC
    Uint32 SDL_RENDERER_TARGETTEXTURE

    struct SDL_RendererInfo:
        char *name
        Uint32 flags
        Uint32 num_texture_formats
        Uint32 texture_formats
        int max_texture_width
        int max_texture_height

    struct SDL_Renderer:
        pass

    struct SDL_Texture:
        pass


    # Function declarations
    #

    # SDL_error.h
    char *SDL_GetError()
    void SDL_ClearError()
    int SDL_SetError(char *, ...)

    # SDL_pixels.h
    Uint32 SDL_PIXELTYPE(Uint32)
    Uint32 SDL_PIXELORDER(Uint32)
    Uint32 SDL_PIXELLAYOUT(Uint32)
    Uint32 SDL_BITSPERPIXEL(Uint32)
    Uint32 SDL_BYTESPERPIXEL(Uint32)
    Uint32 SDL_ISPIXELFORMAT_INDEXED(Uint32)
    Uint32 SDL_ISPIXELFORMAT_ALPHA(Uint32)
    Uint32 SDL_ISPIXELFORMAT_FOURCC(Uint32)

    # SDL_video.h
    SDL_Window *SDL_CreateWindow(char *, int , int , int , int , Uint32)
    int SDL_CreateWindowAndRenderer(int , int , Uint32 , SDL_Window **win, SDL_Renderer **r)
    SDL_Window *SDL_CreateWindowFrom(void *)
    void SDL_DestroyWindow(SDL_Window *)
    void SDL_DisableScreenSaver()
    void SDL_EnableScreenSaver()
    SDL_GLContext SDL_GL_CreateContext(SDL_Window* win)
    SDL_GL_DeleteContext(SDL_GLContext)
    SDL_bool SDL_GL_ExtensionSupported(char *)
    int SDL_GL_GetAttribute(SDL_GLattr , int *)
    void SDL_GL_GetDrawableSize(SDL_Window *, int *, int *)
    void *SDL_GL_GetProcAddress(char *)
    int SDL_GL_GetSwapInterval()
    int SDL_GL_LoadLibrary(char *)
    int SDL_GL_MakeCurrent(SDL_Window *, SDL_GLContext)
    void SDL_GL_ResetAttributes()
    int SDL_GL_SetAttribute(SDL_GLattr , int)
    int SDL_GL_SetSwapInterval(int)
    void SDL_GL_SwapWindow(SDL_Window *)
    void SDL_GL_UnloadLibrary()
    SDL_DisplayMode *SDL_GetClosestDisplayMode(int , SDL_DisplayMode *, SDL_DisplayMode *)
    const char *SDL_GetCurrentVideoDriver()
    int SDL_GetDesktopDisplayMode(int , SDL_DisplayMode *)
    int SDL_GetDisplayBounds(int , SDL_Rect *)
    int SDL_GetDisplayDPI(int , float *, float *, float *)
    const char *SDL_GetDisplayName(int)
    int SDL_GetDisplayUsableBounds(int , SDL_Rect *)
    SDL_Window *SDL_GetGrabbedWindow()
    int SDL_GetNumDisplayModes(int)
    int SDL_GetNumVideoDisplays()
    int SDL_GetNumVideoDrivers()
    const char *SDL_GetVideoDriver(int)
    int SDL_GetWindowBorderSize(SDL_Window , int *, int *, int *, int *)
    float SDL_GetWindowBrightness(SDL_Window *)
    void *SDL_GetWindowData(SDL_Window *, char *)
    int SDL_GetWindowDisplayIndex(SDL_Window *)
    int SDL_GetWindowDisplayMode(SDL_Window *, SDL_DisplayMode *)
    Uint32 SDL_GetWindowFlags(SDL_Window *)
    SDL_Window *SDL_GetWindowFromID(Uint32)
    int SDL_GetWindowGammaRamp(SDL_Window *, Uint16 *, Uint16 *, Uint16 *)
    SDL_bool SDL_GetWindowGrab(SDL_Window *)
    Uint32 SDL_GetWindowID(SDL_Window *)
    void SDL_GetWindowMaximumSize(SDL_Window *, int *, int *)
    void SDL_GetWindowMinimumSize(SDL_Window *, int *, int *)
    int SDL_GetWindowOpacity(SDL_Window *, float *)
    Uint32 SDL_GetWindowPixelFormat(SDL_Window *)
    void SDL_GetWindowPosition(SDL_Window *, int *, int *)
    void SDL_GetWindowSize(SDL_Window *, int *, int *)
    SDL_Surface *SDL_GetWindowSurface(SDL_Window *)
    char *SDL_GetWindowTitle(SDL_Window *)
    SDL_bool SDL_GetWindowWMInfo(SDL_Window *, SDL_SysWMinfo *)
    void SDL_HideWindow(SDL_Window *)
    SDL_bool SDL_IsScreenSaverEnabled()
    void SDL_MaximizeWindow(SDL_Window *)
    void SDL_MinimizeWindow(SDL_Window *)
    void SDL_RaiseWindow(SDL_Window *)
    void SDL_RestoreWindow(SDL_Window *)
    void SDL_SetWindowBordered(SDL_Window *, SDL_bool)
    int SDL_SetWindowBrightness(SDL_Window *, float)
    void *SDL_SetWindowData(SDL_Window *, char *, void *)
    int SDL_SetWindowDisplayMode(SDL_Window *, SDL_DisplayMode *)
    int SDL_SetWindowFullscreen(SDL_Window *, Uint32)
    int SDL_SetWindowGammaRamp(SDL_Window *, Uint16 *, Uint16 *, Uint16 *)
    void SDL_SetWindowGrab(SDL_Window *, SDL_bool)
    int SDL_SetWindowHitTest(SDL_Window *, SDL_HitTest , void *)
    void SDL_SetWindowIcon(SDL_Window *, SDL_Surface *)
    int SDL_SetWindowInputFocus(SDL_Window *)
    void SDL_SetWindowMaximumSize(SDL_Window *, int , int)
    void SDL_SetWindowMinimumSize(SDL_Window *, int , int)
    int SDL_SetWindowModalFor(SDL_Window *, SDL_Window *)
    int SDL_SetWindowOpacity(SDL_Window *, float)
    void SDL_SetWindowPosition(SDL_Window *, int , int)
    void SDL_SetWindowResizable(SDL_Window *, SDL_bool)
    void SDL_SetWindowSize(SDL_Window *, int , int)
    void SDL_SetWindowTitle(SDL_Window *, char *)
    int SDL_ShowMessageBox(SDL_MessageBoxData *, int *)
    int SDL_ShowSimpleMessageBox(Uint32, char *, char *, SDL_Window *)
    void SDL_ShowWindow(SDL_Window *)
    int SDL_UpdateWindowSurface(SDL_Window *)
    int SDL_UpdateWindowSurfaceRect(SDL_Window *, SDL_Rect *, int)
    int SDL_VideoInit(char *)
    void SDL_VideoQuit()

    # SDL_render.h
    SDL_Renderer *SDL_CreateRenderer(SDL_Window *, int , Uint32)
    void SDL_DestroyRenderer(SDL_Renderer *)
    int SDL_RenderCopy(SDL_Renderer *, SDL_Texture *, SDL_Rect *, SDL_Rect *)
#    SDL_RenderCopyEx
    int SDL_RenderClear(SDL_Renderer *)
    void SDL_RenderPresent(SDL_Renderer *)
#    SDL_SetRenderTarget
#    SDL_SetRenderDrawColor

    SDL_Texture *SDL_CreateTexture(SDL_Renderer *, Uint32 , int , int , int)
    SDL_Texture *SDL_CreateTextureFromSurface(SDL_Renderer *, SDL_Surface *)
    void SDL_DestroyTexture(SDL_Texture *)
#    SDL_SetTextureBlendMode
#    SDL_SetTextureColorMod
#    SDL_SetTextureAlphaMod
#    SDL_LockTexture
#    SDL_UnlockTexture
