"""The Window class
"""

from pygame cimport *
import pygame
from pygame.surface import Surface
include "doc/window_doc.pxi"
from cpython.object cimport PyObject, PyTypeObject


# Exported Pygame C API
#
cdef api PyTypeObject *pgWindow_Type
cdef api PyTypeObject *pgWindowSurface_Type
cdef api PyTypeObject *pgWindowRenderer_Type


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
    cdef SDL_Window *window_p
    cdef readonly object renderer
    cdef readonly object surface

    def __cinit__(self, char *title, loc, Uint32 flags=SDL_WINDOW_SHOWN):
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
        if not self.window_p:
            raise_sdl_error()

    def close(self):
        """Destroy the SDL window
        """
        if self:
            if self.renderer is not None:
                self.renderer.close()
            if self.surface is not None:
                self.surface.close()
            SDL_DestroyWindow(self.window_p)
            self.window_p = NULL

    def create_surface(self, subclass=None):
        """Create a display surface object for the window
        """
        cdef SDL_Surface *surface_p

        if not self:
            msg = "Window closed"
            raise pygame.error(msg)
        if self.surface is not None:
            msg = "Window already has a surface object"
            raise AttributeError(msg)
        if subclass is None:
            subclass = WindowSurface
        elif not issubclass(subclass, WindowSurface):
            msg = "{} not a subclass of WindowSurface".format(subclass)
            raise TypeError(msg)       
        surface_p = SDL_GetWindowSurface(self.window_p)
        if surface_p == NULL:
            raise_sdl_error()
        self.surface = subclass.__new__(subclass)
        (<Surface>self.surface).surf = surface_p
        (<WindowSurface>self.surface).window_o = <PyObject *>self
        return self.surface

    def create_renderer(self, Uint32 flags=0, int index=-1, subclass=None):
        """Create a renderer object for the window
        """
        cdef SDL_Renderer *surface_p

        if not self:
            msg = "Window closed"
            raise pygame.error(msg)
        if self.renderer is not None:
            msg = "Window already has a renderer object"
            raise AttributeError(msg)
        if flags != 0 and index != -1:
            msg = "both renderer flags and a driver index were given"
            raise ValueError(msg)
        if subclass is None:
            subclass = WindowRenderer
        elif not issubclass(subclass, WindowRenderer):
            msg = "{} not a subclass of WindowRenderer".format(subclass)
            raise TypeError(msg)       
        renderer_p = SDL_CreateRenderer(self.window_p, index, flags)
        if renderer_p == NULL:
            raise_sdl_error()
        try:
            self.renderer = subclass.__new__(subclass)
        except:
            SDL_DestroyRenderer(renderer_p)
            raise
        (<Renderer>self.renderer).renderer_p = renderer_p
        (<WindowRenderer>self.renderer).window_o = <PyObject *>self
        return self.renderer

    def __nonzero__(self):
        return self.window_p != NULL

    def __dealloc__(self):
        self.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
        return False


cdef class WindowSurface(Surface):
    """A display surface returned by property SurfaceWindow.surface

    The associated window can be refreshed by calling method update. If
    the surface is closed by method close() or its window is closed,
    the surface in invalidated. ********
    """
    cdef PyObject *window_o

    def __nonzero__(self):
        return self.surf != NULL

    def update(self, rects=None):
        ## incomplete
        """Update the window associated with this surface

        update(rects=None) => None

        rects: The parts of the image to update in the window, a sequence of
               rectangles. If None then the whole window is updated.

        A pygame.error exception is raised for any window update problems
        or if the surface is not associated with a window.
        """
        if not self:
            msg = "display Surface quit"
            raise pygame.error(msg)
        if not self.window_o:
            msg = "no associated window to update"
            raise pygame.error(msg)
        if SDL_UpdateWindowSurface((<Window>self.window_o).window_p) < 0:
            raise_sdl_error()

    @property
    def window(self):
        return <object>self.window_o if self.window_o != NULL else None

    def close(self):
        ## What about locking?
        if self:
            self.surf = NULL
            (<Window>self.window_o).surface = None
            self.window_o = NULL


cdef class Renderer:
    """The renderer base class
    
    This is an abstract class.
    """

    cdef SDL_Renderer *renderer_p
    cdef object textures

    def __cinit__(self, *args, **kwds):
        self.textures = set()

    def __nonzero__(self):
        return self.renderer_p != NULL

    def close(self):
        """Destroy all SDL textures and the SDL renderer
        """
        if self:
            for texture in self.get_textures():
                texture.close()
            SDL_DestroyRenderer(self.renderer_p)
            self.renderer_p = NULL

    def new_texture(self, Uint32 format, int access, size, subclass=None):
        ## incomplete
        pass

    def create_texture_from_surface(self, surface, subclass=None):
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
        surface_p = PySurface_AsSurface(surface)
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

        The list is empty if the renderer is closed.
        """
        textures = self.textures
        return list(textures) if textures is not None else []

    def clear(self):
        if not self:
            msg = "The renderer is closed"
            raise pygame.error(msg)
        if SDL_RenderClear(self.renderer_p) < 0:
            raise_sdl_error()

    def __dealloc__(self):
        self.close()


cdef class Texture:
    cdef SDL_Texture *texture_p
    cdef PyObject *renderer_o

    def __nonzero__(self):
        """True if valid, False if closed
        """
        return self.texture_p != NULL

    @property
    def renderer(self):
        return <object>self.renderer_o if self.renderer_o != NULL else None 

    def close(self):
        """Release this texture's resources
        """
        # Destroy the SDL texture and disassociate from its renderer
        #
        if self:
            SDL_DestroyTexture(self.texture_p)
            self.texture_p = NULL
            (<Renderer>self.renderer_o).textures.remove(self)
            self.renderer_o = NULL

    def copy_to_renderer(self, src_rect=None, dst_rect=None):
        ## incomplete: want src and dst rects.
        if not self:
            raise pygame.error("The texture is closed")
        renderer_p = (<Renderer>self.renderer_o).renderer_p
        if SDL_RenderCopy(renderer_p, self.texture_p, NULL, NULL) < 0:
            raise_sdl_error()

    def __dealloc__(self):
        self.close()

cdef object new_texture(subclass, renderer, SDL_Texture *texture_p): 
    cdef Texture texture

    texture = subclass.__new__(subclass)
    texture.texture_p = texture_p
    texture.renderer_o = <PyObject *>renderer
    return texture


cdef class WindowRenderer(Renderer):
    DOC_WINDOWWINDOWRENDERER

    cdef PyObject *window_o

    def close(self):
        """Disconnect from an associated window

        close() => None

        The renderer becomes invalidated. The window remains open.
        """
        if self.window_o != NULL:
            (<Window>self.window_o).renderer = None
            self.window_o = NULL
        Renderer.close(self)

    # Do surface renderers Present?
    # Maybe update belongs in the Renderer class instead
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
pgWindow_Type = <PyTypeObject *>Window
pgWindowSurface_Type = <PyTypeObject *>WindowSurface
pgWindowRenderer_Type = <PyTypeObject *>WindowRenderer
