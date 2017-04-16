"""The Window class
"""

from pygame cimport *
import pygame
from pygame.surface import Surface
include "doc/window_doc.pxi"
from cpython.object cimport PyObject, PyTypeObject


# Exported Pygame C API
#
cdef api PyTypeObject *pgRendererWindow_Type;
cdef api PyTypeObject *pgSurfaceWindow_Type;


# Support code
#
cdef raise_sdl_error():
    msg = SDL_GetError()
    SDL_ClearError()
    raise pygame.error(msg)


# Class declarations
#
cdef class Window:
    """A display window

    Window(title, location, flags)

    title: the window title, a string
    location: either a (width, height) size or a (x, y, width, height)
              rectangle of position and size, a sequence of integers
              (particularly a Rect)
    flags: SDL window flags. An additional Pygame flag is WINDOW_CENTERED.
           If flags is None then the default ... flags are used.

    A pygame.error exception is raised for any window creation problems.
    """
    POS_UNDEFINED = SDL_WINDOWPOS_UNDEFINED
    SHOWN = SDL_WINDOW_SHOWN

    cdef SDL_Window *window_p

    def close(self):
        close_window(self)

    def __nonzero__(self):
        return self.window_p != NULL

    def __dealloc__(self):
        close_window(self)

cdef int open_window(Window win, char *title, loc, Uint32 flags) except -1:
    cdef int x
    cdef int y
    cdef int w
    cdef int h

    ## Put Rect specific code here. Does not need special error checking.
    if len(loc) == 2:
        x = SDL_WINDOWPOS_UNDEFINED
        y = SDL_WINDOWPOS_UNDEFINED
        w, h = loc
    elif len(loc) == 4:
        x, y, w, h = loc
    else:
        msg = "expect a size or rectangle for location"
        raise ValueError(msg)
    win.window_p = SDL_CreateWindow(title, x, y, w, h, flags)
    if not win.window_p:
        raise_sdl_error()
    return 0

cdef void close_window(Window win):
    # Destroy the SDL window
    #
    if win.window_p:
        SDL_DestroyWindow(win.window_p)
        win.window_p = NULL

cdef SDL_Window *get_sdl_window(Window win):
    return win.window_p

cdef class WindowSurface(Surface):
    """A display surface returned by property SurfaceWindow.surface

    The associated window can be refreshed with by calling method update. If
    the surface is closed by calling method close() or method close_window()
    the surface in invalidated. ********
    """
    cdef PyObject *window_o

    def __init__(self, *args, **kwds):
        self.close()
        Surface.__init__(self, *args, **kwds)

    def update(self, rects=None):
        ## incomplete
        """Update the window associated with this surface

        update(rects=None) => None

        rects: The parts of the image to update in the window, a sequence of
               rectangles. If None then the whole window is updated.

        A pygame.error exception is raised for any window update problems
        or if the surface is not associated with a window.
        """
        if not self.window_o:
            msg = "no associated window to update"
            raise pygame.error(msg)
        if SDL_UpdateWindowSurface((<Window>self.window_o).window_p) < 0:
            raise_sdl_error()

    @property
    def window(self):
        if self.window_o:
            return <object>self.window_o
        return None

    def close(self):
        ## What about locking?
        if self.window_o:
            self.surf = NULL
            self.window_o = NULL

    def close_window(self):
        """Close the associated window, if any

        close_window() => None

        If a window is closed, the surface is invalidated. Always succeeds.
        """
        if self.window_o:
            (<object>self.window_o).close()


cdef object new_window_surface(subclass, win):
    cdef SDL_Surface *surf_p
    cdef WindowSurface surf

    surf = subclass.__new__(subclass)
    surf_p = SDL_GetWindowSurface((<Window>win).window_p)
    if not surf_p:
        raise_sdl_error()
    surf.surf = surf_p
    surf.window_o = <PyObject *>win
    return surf


cdef class SurfaceWindow(Window):
    cdef readonly object surface

    def __cinit__(self,
                  char *title,
                  location,
                  Uint32 flags=0,
                  surface_subclass=None):
        if surface_subclass is None:
            surface_subclass = WindowSurface
        elif not issubclass(surface_subclass, WindowSurface):
            msg = "argument surface_subclass not a subclass of WindowSurface"
            raise TypeError(msg)
        
        open_window(self, title, location, flags)
        self.surface = new_window_surface(surface_subclass, self)

    def close(self):
        if self.surface is not None:
            self.surface.close()
            self.surface = None
        Window.close(self)

    def __dealloc__(self):
        if self:
            self.surface.close()


cdef class Renderer:
    """The renderer base class
    
    This is an abstract class.
    """
    ACCELERATED = SDL_RENDERER_ACCELERATED
    PRESENTVSYNC = SDL_RENDERER_PRESENTVSYNC

    cdef SDL_Renderer *renderer_p
    cdef object textures

    def __cinit__(self, *args, **kwds):
        self.textures = set()

    def __nonzero__(self):
        return self.renderer_p != NULL

    def close(self):
        close_renderer(self)

    def new_texture(self, Uint32 format, int access, size, subclass=None):
        ## incomplete
        pass

    def new_texture_from_surface(self, surface, subclass=None):
        cdef SDL_Texture *texture_p
        cdef SDL_Surface *surface_p

        if not self:
            msg = "The renderer is closed"
            raise pygame.error(msg)
        if not isinstance(surface, Surface):
            msg = "surface argument not an actual Surface"
            raise TypeError(msg)
        if subclass is None:
            subclass = Texture
        elif not issubclass(subclass, Texture):
            raise TypeError("argument subclass not a subclass of Texture")
        surface_p = (<Surface>surface).surf
        if not surface_p:
            raise pygame.error("display Surface quit")
        texture_p = SDL_CreateTextureFromSurface(self.renderer_p, surface_p)
        if not texture_p:
            raise_sdl_error()
        try:
            texture = new_texture(subclass, self, texture_p)
        except:
            SDL_DestroyTexture(texture_p)
            raise
        self.textures.add(texture)
        return texture

    def get_textures(self):
        """Return a list of accociated textures

        The list will be empty if the renderer is closed.
        """
        textures = self.textures
        if textures is None:
            textures = []
        else:
            textures = list(textures)
        return textures

    def clear(self):
        if not self:
            msg = "The renderer is closed"
            raise pygame.error(msg)
        if SDL_RenderClear(self.renderer_p) < 0:
            raise_sdl_error()

    def __dealloc__(self):
        close_renderer(self)

cdef object new_renderer(SDL_Renderer *renderer_p, subclass):
    cdef Renderer renderer

    renderer = subclass.__new__(subclass)
    renderer.renderer_p = renderer_p
    renderer.textures = set()
    return renderer

cdef close_renderer(Renderer r):
    # Destroy all SDL textures and the SDL renderer
    #
    if r.renderer_p:
        for texture in r.get_textures():
            texture.close()
        SDL_DestroyRenderer(r.renderer_p)
        r.renderer_p = NULL
        
cdef void renderer_remove_texture(Renderer r, texture):
    r.textures.remove(texture)


cdef class Texture:
    cdef SDL_Texture *texture_p
    cdef PyObject *renderer_o

    def __nonzero__(self):
        """True if valid, False if closed
        """
        return self.texture_p != NULL

    def close(self):
        """Release this texture's resources
        """
        close_texture(self)

    def copy_to_renderer(self, src_rect=None, dst_rect=None):
        ## incomplete: want src and dst rects.
        if not self.texture_p:
            raise pygame.error("The texture is closed")
        renderer_p = (<Renderer>self.renderer_o).renderer_p
        if SDL_RenderCopy(renderer_p, self.texture_p, NULL, NULL) < 0:
            raise_sdl_error()

    def __dealloc__(self):
        close_texture(self)

cdef object new_texture(subclass, renderer, SDL_Texture *texture_p): 
    cdef Texture texture

    texture = subclass.__new__(subclass)
    texture.texture_p = texture_p
    texture.renderer_o = <PyObject *>renderer
    return texture

cdef void close_texture(Texture t):
    # Destroy the SDL texture and disassociate the object from its renderer
    #
    if t:
        SDL_DestroyTexture(t.texture_p)
        t.texture_p = NULL
        renderer_remove_texture(<WindowRenderer>t.renderer_o, t)
        t.renderer_o = NULL


cdef class WindowRenderer(Renderer):
    DOC_WINDOWWINDOWRENDERER

    cdef PyObject *window_o

    def open(self, Uint32 flags=0, int driver_index=-1):
        cdef SDL_Renderer *renderer_p
        cdef SDL_Window *window_p

        if not self.window_o:
            raise pygame.error("window closed")
        self.close()
        if flags != 0 and driver_index != -1:
            msg = "flags and driver_index arguments were both defined"
            raise ValueError(msg)
        window_p = get_sdl_window(<Window>self.window_o)
        renderer_p = SDL_CreateRenderer(window_p, driver_index, flags)
        if not renderer_p:
            raise_sdl_error()
        self.renderer_p = renderer_p

    def close(self):
        """Disconnect from an associated window

        close() => None

        The renderer becomes invalidated. The window remains open.
        """
        Renderer.close(self)

    def close_window(self):
        if self:
            (<object>self.window_o).close()

    def update(self):
        if not self:
            msg = "The renderer is closed"
            raise pygame.error(msg)
        SDL_RenderPresent(self.renderer_p)

cdef object new_win_renderer(subclass, win):
    cdef WindowRenderer obj

    obj = subclass.__new__(subclass)
    obj.window_o = <PyObject *>win
    return obj

cdef void release_win_renderer(WindowRenderer r):
    r.close()
    r.window_o = NULL


cdef class RendererWindow(Window):
    cdef readonly object renderer

    def __cinit__(self,
                  char *title,
                  location,
                  Uint32 flags=0,
                  renderer_subclass=None,
                  *args, **kwds):
        cdef WindowRenderer renderer

        if renderer_subclass is None:
            renderer_subclass = WindowRenderer
        elif not issubclass(renderer_subclass, WindowRenderer):
            msg = "argument renderer_subclass not a subclass of WindowRenderer"
            raise TypeError(msg)
        open_window(self, title, location, flags)
        try:
            self.renderer = new_win_renderer(renderer_subclass, self)
        except:
            Window.close(self)
            raise

    def close(self):
        if self:
            release_win_renderer(self.renderer)
            Window.close(self)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
        return False

    def __dealloc__(self):
        if self:
            release_win_renderer(<Renderer>self.renderer)


# Module initialization
#
import_pygame_base()
import_pygame_surface()
pgRendererWindow_Type = <PyTypeObject *>RendererWindow
pgSurfaceWindow_Type= <PyTypeObject *>SurfaceWindow
