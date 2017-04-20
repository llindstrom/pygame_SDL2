"""The Window class
"""

from pygame cimport *
import pygame
from pygame.surface import Surface
include "doc/window_doc.pxi"
from cpython.object cimport PyObject, PyTypeObject

DEF WindowFlagsMask = 0xFFFFFFFF
DEF RenderFlagsOffset = 32

# Exported Pygame C API
#
cdef api PyTypeObject *pgWindowRenderer_Type;
cdef api PyTypeObject *pgWindowSurface_Type;


# Support code
#
cdef raise_sdl_error():
    msg = SDL_GetError()
    SDL_ClearError()
    raise pygame.error(msg)


cdef class WindowSurface(Surface):
    """A display surface returned by property SurfaceWindow.surface

    The associated window can be refreshed with by calling method update. If
    the surface is closed by calling method close() or method close_window()
    the surface in invalidated. ********
    """

    # Window mixin
    # Duplicate any changes here in class WindowRenderer
    #
    cdef SDL_Window *window_p

    cdef int open_window(self, char *title, loc, Uint32 flags) except -1:
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
        self.window_p = SDL_CreateWindow(title, x, y, w, h, flags)
        if self.window_p == NULL:
            raise_sdl_error()
        return 0

    cdef void close_window(self):
        # Destroy the SDL window
        #
        if self.window_p != NULL:
            SDL_DestroyWindow(self.window_p)
            self.window_p = NULL

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
        return False
    #
    # End Window mixin

    def __init__(self, char *title, loc, Uint32 flags):
        self.close()
        self.open_window(title, loc, flags)
        self.surf = SDL_GetWindowSurface(self.window_p)
        if self.surf == NULL:
            raise_sdl_error()

    def __nonzero__(self):
        return self.window_p != NULL

    def update(self, rects=None):
        ## incomplete
        """Update the window associated with this surface

        update(rects=None) => None

        rects: The parts of the image to update in the window, a sequence of
               rectangles. If None then the whole window is updated.

        A pygame.error exception is raised for any window update problems
        or if the surface is not associated with a window.
        """
        if self.surf == NULL:
            msg = "display surface quit"
            raise pygame.error(msg)
        if SDL_UpdateWindowSurface(self.window_p) < 0:
            raise_sdl_error()

    def close(self):
        ## What about locking?
        if self:
            self.close_window()
            self.surf = NULL

    def __dealloc__(self):
        self.close()


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

    # Window mixin
    # Duplicate any changes here in class WindowSurface
    #
    cdef SDL_Window *window_p

    cdef int open_window(self, char *title, loc, Uint32 flags) except -1:
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
        self.window_p = SDL_CreateWindow(title, x, y, w, h, flags)
        if self.window_p == NULL:
            raise_sdl_error()
        return 0

    cdef void close_window(self):
        # Destroy the SDL window
        #
        if self.window_p != NULL:
            SDL_DestroyWindow(self.window_p)
            self.window_p = NULL

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
        return False
    #
    # End Window mixin

    def __init__(self, char *title, loc, flags, int driver_index=-1):
        cdef Uint32 window_flags = flags & WindowFlagsMask
        cdef Uint32 renderer_flags = flags >> RenderFlagsOffset

        self.close()
        if renderer_flags != 0 and driver_index != -1:
            msg = "both renderer flags and a driver index were given"
            raise ValueError(msg)
        self.open_window(title, loc, window_flags)
        self.renderer_p = SDL_CreateRenderer(self.window_p,
                                             driver_index,
                                             renderer_flags)
        if self.renderer_p == NULL:
            self.close_window()
            raise_sdl_error()

    def close(self):
        """Close the renderer and window

        close() => None
        """
        Renderer.close(self)
        self.close_window()

    # Maybe update applies to surface renderers as well,
    # so should belong in the Renderer class
    def update(self):
        if not self:
            msg = "The renderer is closed"
            raise pygame.error(msg)
        SDL_RenderPresent(self.renderer_p)

    def __dealloc__(self):
        self.close()


# Module initialization
#
import_pygame_base()
import_pygame_surface()
pgWindowRenderer_Type = <PyTypeObject *>WindowRenderer
pgWindowSurface_Type= <PyTypeObject *>WindowSurface
