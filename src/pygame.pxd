from SDL cimport *

cdef extern from "pygame.h":
    # base
    void import_pygame_base() except *

    # Surface
    struct SubSurface_Data:
        int pixeloffset
        int offsetx
        int offsety

    ctypedef class pygame.surface.Surface [object PySurfaceObject]:
        cdef SDL_Surface *surf
        cdef SubSurface_Data *subsurface

    object PySurface_New(SDL_Surface *surface)
    object PySurface_NewNoOwn(SDL_Surface *surface)
    void import_pygame_surface() except *
