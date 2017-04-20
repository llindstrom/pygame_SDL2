/*
  pygame - Python Game Library
  Copyright (C) 2000-2001  Pete Shinners

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Pete Shinners
  pete@shinners.org
*/

/*
 *  pygame display module
 */
#define PYGAMEAPI_DISPLAY_INTERNAL
#include "pygame.h"
#include "pgcompat.h"
#include "doc/display_doc.h"

#ifndef SDL2

#include <SDL_syswm.h>
static PyTypeObject pgVidInfo_Type;
static PyObject* pgVidInfo_New (const SDL_VideoInfo* info);
static void pg_do_set_icon (PyObject *surface);
static PyObject* pgDisplaySurfaceObject = NULL;
static int icon_was_set = 0;

#else /* SDL2 */
#include "pygame.window_api.h"

typedef struct _display_state_s {
    char* title;
    PyObject* icon;
    Uint16 *gamma_ramp;
} _DisplayState;

#if PY3
static struct PyModuleDef _module;
#define DISPLAY_MOD_STATE(mod) ((_DisplayState*)PyModule_GetState(mod))
#define DISPLAY_STATE \
    DISPLAY_MOD_STATE(PyState_FindModule(&_module))
#else /* PY3 */
static _DisplayState _modstate;
#define DISPLAY_MOD_STATE(mod) (&_modstate)
#define DISPLAY_STATE DISPLAY_MOD_STATE(0)
#endif /* PY3 */

#endif /* SDL2 */

#if (!defined(darwin))
static char* icon_defaultname = "pygame_icon.bmp";
static char* pkgdatamodule_name = "pygame.pkgdata";
static char* imagemodule_name = "pygame.image";
static char* resourcefunc_name = "getResource";
static char* load_basicfunc_name = "load_basic";

static void
pg_close_file (PyObject *fileobj)
{
    PyObject* result = PyObject_CallMethod(fileobj, "close", NULL);
    if (result) {
        Py_DECREF (result);
    }
    else {
        PyErr_Clear ();
    }
}

static PyObject*
pg_display_resource (char *filename)
{
    PyObject* imagemodule = NULL;
    PyObject* load_basicfunc = NULL;
    PyObject* pkgdatamodule = NULL;
    PyObject* resourcefunc = NULL;
    PyObject* fresult = NULL;
    PyObject* result = NULL;
#if PY3
    PyObject* name = NULL;
#endif

    pkgdatamodule = PyImport_ImportModule(pkgdatamodule_name);
    if (!pkgdatamodule)
        goto display_resource_end;

    resourcefunc = PyObject_GetAttrString(pkgdatamodule, resourcefunc_name);
    if (!resourcefunc)
        goto display_resource_end;

    imagemodule = PyImport_ImportModule(imagemodule_name);
    if (!imagemodule)
        goto display_resource_end;

    load_basicfunc = PyObject_GetAttrString(imagemodule, load_basicfunc_name);
    if (!load_basicfunc)
        goto display_resource_end;

    fresult = PyObject_CallFunction(resourcefunc, "s", filename);
    if (!fresult)
        goto display_resource_end;

#if PY3
    name = PyObject_GetAttrString (fresult, "name");
    if (name != NULL) {
        if (Text_Check(name)) {
            pg_close_file(fresult);
            Py_DECREF(fresult);
            fresult = name;
            name = NULL;
        }
    }
    else {
        PyErr_Clear ();
    }
#else
    if (PyFile_Check (fresult)) {
        PyObject *tmp = PyFile_Name(fresult);
        Py_INCREF(tmp);
        pg_close_file(fresult);
        Py_DECREF(fresult);
        fresult = tmp;
    }
#endif

    result = PyObject_CallFunction(load_basicfunc, "O", fresult);
    if (!result) goto display_resource_end;

display_resource_end:
    Py_XDECREF (pkgdatamodule);
    Py_XDECREF (resourcefunc);
    Py_XDECREF (imagemodule);
    Py_XDECREF (load_basicfunc);
    Py_XDECREF (fresult);
#if PY3
    Py_XDECREF (name);
#endif
    return result;
}
#endif

/* init routines */
static void
pg_display_autoquit (void)
{
#ifdef SDL2
    if (Py_GetDefaultWindowSurface ()) {
        _DisplayState* state = DISPLAY_STATE;

        Py_SetDefaultWindowSurface(NULL);

        if (state->title) {
            free (state->title);
            state->title = NULL;
        }

        Py_XDECREF(state->icon);
        state->icon = NULL;

        if (state->gamma_ramp)
            free (state->gamma_ramp);
        state->gamma_ramp = NULL;

        Py_SetDefaultWindow(NULL);
    }
#else
    if (pgDisplaySurfaceObject) {
        PySurface_AsSurface(pgDisplaySurfaceObject) = NULL;
        Py_DECREF(pgDisplaySurfaceObject);
        pgDisplaySurfaceObject = NULL;
        icon_was_set = 0;
    }
#endif
}

static PyObject*
pg_display_autoinit (PyObject* self, PyObject* arg)
{
    PyGame_RegisterQuit(pg_display_autoquit);
    return PyInt_FromLong (1);
}

static PyObject*
pg_quit (PyObject* self, PyObject* arg)
{
    PyGame_Video_AutoQuit();
    pg_display_autoquit();
    Py_RETURN_NONE;
}

static PyObject*
pg_init (PyObject* self)
{
    if (!PyGame_Video_AutoInit())
        return RAISE (PyExc_SDLError, SDL_GetError());
    if (!pg_display_autoinit(NULL, NULL))
        return NULL;
    Py_RETURN_NONE;
}

static PyObject*
pg_get_init (PyObject* self)
{
    return PyInt_FromLong(SDL_WasInit(SDL_INIT_VIDEO) != 0);
}

static PyObject*
pg_get_active (PyObject* self)
{
#ifdef SDL2
    Uint32 flags = SDL_GetWindowFlags (Py_GetDefaultWindow ());
    return PyInt_FromLong((flags & SDL_WINDOW_SHOWN) != 0);
#else
    return PyInt_FromLong((SDL_GetAppState() & SDL_APPACTIVE) != 0);
#endif
}

/* vidinfo object */
#ifdef SDL2
#warning "The vidinfo object is not used in SDL2 so far."
#else
static void
pg_vidinfo_dealloc (PyObject* self)
{
    PyObject_DEL (self);
}

static PyObject*
pg_vidinfo_getattr (PyObject *self, char *name)
{
    SDL_VideoInfo* info = &((PyVidInfoObject*) self)->info;

    int current_w = -1;
    int current_h = -1;

    SDL_version versioninfo;
    SDL_VERSION (&versioninfo);

    if (versioninfo.major >= 1 && versioninfo.minor >= 2
        && versioninfo.patch >= 10 ) {
        current_w = info->current_w;
        current_h = info->current_h;
    }

    if (!strcmp (name, "hw"))
        return PyInt_FromLong(info->hw_available);
    else if (!strcmp (name, "wm"))
        return PyInt_FromLong(info->wm_available);
    else if (!strcmp (name, "blit_hw"))
        return PyInt_FromLong(info->blit_hw);
    else if (!strcmp (name, "blit_hw_CC"))
        return PyInt_FromLong(info->blit_hw_CC);
    else if (!strcmp (name, "blit_hw_A"))
        return PyInt_FromLong(info->blit_hw_A);
    else if (!strcmp (name, "blit_sw"))
        return PyInt_FromLong(info->blit_hw);
    else if (!strcmp (name, "blit_sw_CC"))
        return PyInt_FromLong(info->blit_hw_CC);
    else if (!strcmp (name, "blit_sw_A"))
        return PyInt_FromLong(info->blit_hw_A);
    else if (!strcmp (name, "blit_fill"))
        return PyInt_FromLong(info->blit_fill);
    else if (!strcmp (name, "video_mem"))
        return PyInt_FromLong(info->video_mem);
    else if (!strcmp (name, "bitsize"))
        return PyInt_FromLong(info->vfmt->BitsPerPixel);
    else if (!strcmp (name, "bytesize"))
        return PyInt_FromLong(info->vfmt->BytesPerPixel);
    else if (!strcmp (name, "masks"))
        return Py_BuildValue("(iiii)", info->vfmt->Rmask, info->vfmt->Gmask,
                             info->vfmt->Bmask, info->vfmt->Amask);
    else if (!strcmp (name, "shifts"))
        return Py_BuildValue("(iiii)", info->vfmt->Rshift, info->vfmt->Gshift,
                             info->vfmt->Bshift, info->vfmt->Ashift);
    else if (!strcmp (name, "losses"))
        return Py_BuildValue("(iiii)", info->vfmt->Rloss, info->vfmt->Gloss,
                             info->vfmt->Bloss, info->vfmt->Aloss);
    else if (!strcmp (name, "current_h"))
        return PyInt_FromLong(current_h);
    else if (!strcmp (name, "current_w"))
        return PyInt_FromLong(current_w);

    return RAISE(PyExc_AttributeError, "does not exist in vidinfo");
}

PyObject*
pg_vidinfo_str (PyObject* self)
{
    char str[1024];
    int current_w = -1;
    int current_h = -1;
    SDL_VideoInfo* info = &((PyVidInfoObject*) self)->info;

    SDL_version versioninfo;
    SDL_VERSION(&versioninfo);

    if(versioninfo.major >= 1 && versioninfo.minor >= 2
       && versioninfo.patch >= 10 ) {
        current_w = info->current_w;
        current_h = info->current_h;
    }

    sprintf (str, "<VideoInfo(hw = %d, wm = %d,video_mem = %d\n"
             "         blit_hw = %d, blit_hw_CC = %d, blit_hw_A = %d,\n"
             "         blit_sw = %d, blit_sw_CC = %d, blit_sw_A = %d,\n"
             "         bitsize  = %d, bytesize = %d,\n"
             "         masks =  (%d, %d, %d, %d),\n"
             "         shifts = (%d, %d, %d, %d),\n"
             "         losses =  (%d, %d, %d, %d),\n"
             "         current_w = %d, current_h = %d\n"
             ">\n",
             info->hw_available, info->wm_available, info->video_mem,
             info->blit_hw, info->blit_hw_CC, info->blit_hw_A,
             info->blit_sw, info->blit_sw_CC, info->blit_sw_A,
             info->vfmt->BitsPerPixel, info->vfmt->BytesPerPixel,
             info->vfmt->Rmask, info->vfmt->Gmask, info->vfmt->Bmask,
             info->vfmt->Amask,
             info->vfmt->Rshift, info->vfmt->Gshift, info->vfmt->Bshift,
             info->vfmt->Ashift,
             info->vfmt->Rloss, info->vfmt->Gloss, info->vfmt->Bloss,
             info->vfmt->Aloss,
             current_w, current_h);
    return Text_FromUTF8(str);
}

static PyTypeObject pgVidInfo_Type =
{
    TYPE_HEAD (NULL, 0)
    "VidInfo",               /*name*/
    sizeof(PyVidInfoObject), /*basic size*/
    0,                       /*itemsize*/
    pg_vidinfo_dealloc,      /*dealloc*/
    0,                       /*print*/
    pg_vidinfo_getattr,      /*getattr*/
    NULL,                    /*setattr*/
    NULL,                    /*compare*/
    pg_vidinfo_str,          /*repr*/
    NULL,                    /*as_number*/
    NULL,                    /*as_sequence*/
    NULL,                    /*as_mapping*/
    (hashfunc)NULL,          /*hash*/
    (ternaryfunc)NULL,       /*call*/
    (reprfunc)NULL,          /*str*/
};

static PyObject*
pgVidInfo_New (const SDL_VideoInfo* i)
{
    PyVidInfoObject* info;
    if (!i)
        return RAISE (PyExc_SDLError, SDL_GetError ());
    info = PyObject_NEW (PyVidInfoObject, &pgVidInfo_Type);
    if (!info)
        return NULL;
    memcpy (&info->info, i, sizeof (SDL_VideoInfo));
    return (PyObject*)info;
}

static PyObject*
pg_get_wm_info (PyObject* self)
{
    PyObject *dict;
    PyObject *tmp;
    SDL_SysWMinfo info;

    VIDEO_INIT_CHECK();

    SDL_VERSION(&(info.version))
    dict = PyDict_New();
    if (!dict || !SDL_GetWMInfo(&info))
        return dict;

/*scary #ifdef's match SDL_syswm.h*/
#if (defined(unix) || defined(__unix__) ||              \
     defined(_AIX) || defined(__OpenBSD__)) &&          \
    (defined(SDL_VIDEO_DRIVER_X11) && !defined(__CYGWIN32__) && \
     !defined(ENABLE_NANOX) && !defined(__QNXNTO__))

    tmp = PyInt_FromLong(info.info.x11.window);
    PyDict_SetItemString(dict, "window", tmp);
    Py_DECREF(tmp);

    tmp = PyCapsule_New(info.info.x11.display, "display", NULL);
    PyDict_SetItemString(dict, "display", tmp);
    Py_DECREF(tmp);

    tmp = PyCapsule_New(info.info.x11.lock_func, "lock_func", NULL);
    PyDict_SetItemString(dict, "lock_func", tmp);
    Py_DECREF(tmp);

    tmp = PyCapsule_New(info.info.x11.unlock_func, "unlock_func", NULL);
    PyDict_SetItemString(dict, "unlock_func", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(info.info.x11.fswindow);
    PyDict_SetItemString(dict, "fswindow", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(info.info.x11.wmwindow);
    PyDict_SetItemString(dict, "wmwindow", tmp);
    Py_DECREF(tmp);

#elif defined(ENABLE_NANOX)
    tmp = PyInt_FromLong(info.window);
    PyDict_SetItemString(dict, "window", tmp);
    Py_DECREF(tmp);
#elif defined(WIN32)
    tmp = PyInt_FromLong((long)info.window);
    PyDict_SetItemString(dict, "window", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong((long)info.hglrc);
    PyDict_SetItemString(dict, "hglrc", tmp);
    Py_DECREF(tmp);
#elif defined(__riscos__)
    tmp = PyInt_FromLong(info.window);
    PyDict_SetItemString(dict, "window", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(info.wimpVersion);
    PyDict_SetItemString(dict, "wimpVersion", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(info.taskHandle);
    PyDict_SetItemString(dict, "taskHandle", tmp);
    Py_DECREF(tmp);
#elif (defined(__APPLE__) && defined(__MACH__))
    /* do nothing. */
#else
    tmp = PyInt_FromLong (info.data);
    PyDict_SetItemString (dict, "data", tmp);
    Py_DECREF(tmp);
#endif

    return dict;
}

static PyObject*
pgInfo (PyObject* self)
{
    const SDL_VideoInfo* info;
    VIDEO_INIT_CHECK();
    info = SDL_GetVideoInfo();
    return pgVidInfo_New(info);
}
#endif /* pgInfo stuff is not defined in SDL2 */

/* display functions */
static PyObject*
pg_get_driver (PyObject* self)
{
#ifdef SDL2
    const char* name = NULL;
    name = SDL_GetCurrentVideoDriver();
    if (!name)
        Py_RETURN_NONE;
    return Text_FromUTF8(name);
#else
    char buf[256];
    VIDEO_INIT_CHECK ();
    if (!SDL_VideoDriverName(buf, sizeof(buf)))
        Py_RETURN_NONE;
    return Text_FromUTF8(buf);
#endif
}

static PyObject*
pg_get_surface (PyObject* self)
{
#ifdef SDL2
    PyObject* surface = Py_GetDefaultWindowSurface();
    if (!surface)
        Py_RETURN_NONE;
    Py_INCREF (surface);
    return surface;
#else
    if (!pgDisplaySurfaceObject)
        Py_RETURN_NONE;
    Py_INCREF (pgDisplaySurfaceObject);
    return pgDisplaySurfaceObject;
#endif
}

static PyObject*
pg_gl_set_attribute (PyObject* self, PyObject* arg)
{
    int flag, value, result;
    VIDEO_INIT_CHECK();
    if (!PyArg_ParseTuple(arg, "ii", &flag, &value))
        return NULL;
    if (flag == -1) /*an undefined/unsupported val, ignore*/
        Py_RETURN_NONE;
    result = SDL_GL_SetAttribute(flag, value);
    if (result == -1)
        return RAISE(PyExc_SDLError, SDL_GetError());
    Py_RETURN_NONE;
}

static PyObject*
pg_gl_get_attribute (PyObject* self, PyObject* arg)
{
    int flag, value, result;
    VIDEO_INIT_CHECK();
    if (!PyArg_ParseTuple (arg, "i", &flag))
        return NULL;
    result = SDL_GL_GetAttribute(flag, &value);
    if (result == -1)
        return RAISE (PyExc_SDLError, SDL_GetError());
    return PyInt_FromLong (value);
}

static PyObject*
pg_set_mode (PyObject* self, PyObject* arg)
{
#ifndef SDL2
    SDL_Surface* surf;
#else /* SDL2 */
    static const char* const DefaultTitle = "pygame window";

    _DisplayState* state = DISPLAY_MOD_STATE (self);
    SDL_Window* win = Py_GetDefaultWindow ();
    PyObject* surface = Py_GetDefaultWindowSurface ();
    SDL_Surface* surf = NULL;
#endif /* SDL2 */
    int depth = 0;
#ifndef SDL2
    int flags = SDL_SWSURFACE;
#else /* SDL2 */
    int flags = 0;
#endif /* SDL2 */
    int w = 0;
    int h = 0;
#ifndef SDL2
    int hasbuf;
    char *title, *icontitle;
#else /* SDL2 */
    char *title = state->title;
#endif /* SDL2 */

    if (!PyArg_ParseTuple (arg, "|(ii)ii", &w, &h, &flags, &depth))
        return NULL;

    if (w < 0 || h < 0)
        return RAISE (PyExc_SDLError, "Cannot set negative sized display mode");

    if (w == 0 || h == 0)
    {
        SDL_version versioninfo;
        SDL_VERSION (&versioninfo);
#ifndef SDL2
        if (!(versioninfo.major > 1 ||
#else /* SDL2 */
        if (!(versioninfo.major != 1 ||
#endif /* SDL2 */
             (versioninfo.major == 1 && versioninfo.minor > 2) ||
             (versioninfo.major == 1 && versioninfo.minor == 2 && versioninfo.patch >= 10 )))
        {
            return RAISE (PyExc_SDLError, "Cannot set 0 sized display mode");
        }
    }

    if (!SDL_WasInit (SDL_INIT_VIDEO))
    {
#ifndef SDL2
        /*note SDL works special like this too*/
#else /* SDL2 */
        /*note SDL works special like thir too*/
#endif /* SDL2 */
        if (!pg_init (NULL))
            return NULL;
    }

#ifndef SDL2
    if (flags & SDL_OPENGL)
#else /* SDL2 */
    if (!state->title)
#endif /* SDL2 */
    {
#ifndef SDL2
        if (flags & SDL_DOUBLEBUF)
        {
            flags &= ~SDL_DOUBLEBUF;
            SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
#else /* SDL2 */
        state->title = malloc ((strlen(DefaultTitle) + 1) * sizeof(char *));
        if (!state->title)
            return PyErr_NoMemory ();
        strcpy (state->title, DefaultTitle);
        title = state->title;
    }

    if (win)
    {
        /*change existing window*/
        SDL_SetWindowTitle (win, title);
        SDL_SetWindowSize (win, w, h);
#warning Add mode stuff.
        surf = SDL_GetWindowSurface (win);
        if (!surf) {
            PyErr_SetString (PyExc_SDLError, SDL_GetError ());
            Py_SetDefaultWindow (NULL);
            return NULL;
#endif /* SDL2 */
        }
#ifndef SDL2
        else
            SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 0);
        if (depth)
            SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, depth);
        surf = SDL_SetVideoMode (w, h, depth, flags);
        if (!surf)
            return RAISE (PyExc_SDLError, SDL_GetError ());

        SDL_GL_GetAttribute (SDL_GL_DOUBLEBUFFER, &hasbuf);
        if (hasbuf)
            surf->flags |= SDL_DOUBLEBUF;
#else /* SDL2 */
        assert (surface);
        PySurface_AsSurface (surface) = surf;
#endif /* SDL2 */
    }
    else
    {
#ifndef SDL2
        if (!depth)
            flags |= SDL_ANYFORMAT;
        Py_BEGIN_ALLOW_THREADS;
        surf = SDL_SetVideoMode (w, h, depth, flags);
        Py_END_ALLOW_THREADS;
        if  (!surf)
#else /* SDL2 */
        /*open window*/
        Uint32 sdl_flags = 0;
        if (flags & PGS_FULLSCREEN)
            sdl_flags |= SDL_WINDOW_FULLSCREEN;
        if (flags & PGS_OPENGL)
            sdl_flags |= SDL_WINDOW_OPENGL;
        if (flags & PGS_NOFRAME)
            sdl_flags |= SDL_WINDOW_BORDERLESS;
        if (flags & PGS_RESIZABLE)
            sdl_flags |= SDL_WINDOW_RESIZABLE;
        if (flags & PGS_SHOWN)
            sdl_flags |= SDL_WINDOW_SHOWN;
        if (flags & PGS_HIDDEN)
            sdl_flags |= SDL_WINDOWEVENT_HIDDEN;
        if (!(sdl_flags & SDL_WINDOWEVENT_HIDDEN))
            sdl_flags |= SDL_WINDOW_SHOWN;
        win = SDL_CreateWindow (title,
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                w,
                                h,
                                sdl_flags);
        if (!win)
#endif /* SDL2 */
            return RAISE (PyExc_SDLError, SDL_GetError ());
#ifndef SDL2
    }
    SDL_WM_GetCaption (&title, &icontitle);
    if (!title || !*title)
        SDL_WM_SetCaption ("pygame window", "pygame");
#endif /* ! SDL2 */

#ifndef SDL2
    /*probably won't do much, but can't hurt, and might help*/
    SDL_PumpEvents ();
#else /* SDL2 */
        if (state->gamma_ramp)
        {
            int result = SDL_SetWindowGammaRamp (win,
                                                 state->gamma_ramp,
                                                 state->gamma_ramp + 256,
                                                 state->gamma_ramp + 512);
            if (result) /* SDL Error? */
            {
                /* Discard a possibly faulty gamma ramp. */
                free (state->gamma_ramp);
                state->gamma_ramp = NULL;

                /* Recover error, then destroy the window */
                RAISE (PyExc_SDLError, SDL_GetError ());
                SDL_DestroyWindow (win);
                return NULL;
            }
        }
        surf = SDL_GetWindowSurface (win);
        if (!surf)
        {
            PyErr_SetString (PyExc_SDLError, SDL_GetError ());
            SDL_DestroyWindow (win);
            return NULL;
        }
        surface = PySurface_NewNoOwn (surf);
        if (!surface)
        {
            SDL_DestroyWindow (win);
            return 0;
        }
#endif /* SDL2 */

#ifndef SDL2
    if (pgDisplaySurfaceObject)
        ((PySurfaceObject*) pgDisplaySurfaceObject)->surf = surf;
    else
        pgDisplaySurfaceObject = PySurface_New (surf);
#else /* SDL2 */
        /*no errors; make the window available*/
        Py_SetDefaultWindow (win);
        Py_SetDefaultWindowSurface (surface);
        Py_DECREF (surface);
    }
#endif /* SDL2 */

#if !defined(darwin)
#ifndef SDL2
    if (!icon_was_set)
#else /* SDL2 */
    /*set the window icon*/
    if (!state->icon)
#endif /* SDL2 */
    {
#ifndef SDL2
        PyObject* iconsurf = pg_display_resource(icon_defaultname);
        if (!iconsurf)
#else /* SDL2 */
        state->icon = pg_display_resource(icon_defaultname);
        if (!state->icon)
#endif /* SDL2 */
            PyErr_Clear ();
        else
        {
#ifndef SDL2
            SDL_SetColorKey (PySurface_AsSurface (iconsurf), SDL_SRCCOLORKEY,
#else /* SDL2 */
            SDL_SetColorKey (PySurface_AsSurface (state->icon),
                             SDL_TRUE,
#endif /* SDL2 */
                             0);
#ifndef SDL2
            pg_do_set_icon (iconsurf);
            Py_DECREF (iconsurf);
#endif /* ! SDL2 */
        }
    }
#ifdef SDL2
    if (state->icon)
        SDL_SetWindowIcon (win, PySurface_AsSurface (state->icon));
#endif /* SDL2 */
#endif
#ifndef SDL2
    Py_INCREF (pgDisplaySurfaceObject);
    return pgDisplaySurfaceObject;
#else /* SDL2 */

    /*probably won't do much, but can't hurt, and might help*/
    SDL_PumpEvents ();

    /*return the window's surface (screen)*/
    Py_INCREF (surface);
    return surface;
#endif /* SDL2 */
}

static PyObject*
pg_mode_ok (PyObject* self, PyObject* args)
{
#ifdef SDL2
#warning "implement pg_mode_ok for SDL2"
    return PyInt_FromLong ((long)0);
#else
    int depth = 0;
    int w, h;
    int flags = SDL_SWSURFACE;

    VIDEO_INIT_CHECK();

    if (!PyArg_ParseTuple (args, "(ii)|ii", &w, &h, &flags, &depth))
        return NULL;
    if (!depth)
        depth = SDL_GetVideoInfo()->vfmt->BitsPerPixel;

    return PyInt_FromLong(SDL_VideoModeOK (w, h, depth, flags));
#endif
}

static PyObject*
pg_list_modes (PyObject* self, PyObject* args)
{
#ifdef SDL2
#warning "implement pg_list_modes for SDL2"
    Py_RETURN_NONE;
#else
    SDL_PixelFormat format;
    SDL_Rect** rects;
    int flags=SDL_FULLSCREEN;
    PyObject *list, *size;

    format.BitsPerPixel = 0;
    if (PyTuple_Size (args)!=0
        && !PyArg_ParseTuple (args, "|bi", &format.BitsPerPixel, &flags))
        return NULL;

    VIDEO_INIT_CHECK();

    if (!format.BitsPerPixel)
        format.BitsPerPixel = SDL_GetVideoInfo()->vfmt->BitsPerPixel;

    rects = SDL_ListModes(&format, flags);

    if(rects == (SDL_Rect**)-1)
        return PyInt_FromLong (-1);

    if(!(list = PyList_New(0)))
        return NULL;
    if (!rects)
        return list;

    for (; *rects; ++rects) {
        if (!(size = Py_BuildValue("(ii)", (*rects)->w, (*rects)->h))) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_Append(list, size);
        Py_DECREF(size);
    }
    return list;
#endif
}

static PyObject*
pg_flip (PyObject* self)
{
#ifdef SDL2
    SDL_Window* win = Py_GetDefaultWindow();
    int status = 0;

    VIDEO_INIT_CHECK();

    if (!win)
        return RAISE (PyExc_SDLError, "Display mode not set");

    Py_BEGIN_ALLOW_THREADS;
    if (SDL_GetWindowFlags(win) & SDL_WINDOW_OPENGL)
        SDL_GL_SwapWindow(win);
    else
        status = SDL_UpdateWindowSurface (win) == -1;
    Py_END_ALLOW_THREADS;

    if (status == -1)
        return RAISE (PyExc_SDLError, SDL_GetError ());
    Py_RETURN_NONE;
#else
    SDL_Surface* screen;
    int status = 0;

    VIDEO_INIT_CHECK();

    screen = SDL_GetVideoSurface();
    if (!screen)
        return RAISE (PyExc_SDLError, "Display mode not set");

    Py_BEGIN_ALLOW_THREADS;
    if (screen->flags & SDL_OPENGL)
        SDL_GL_SwapBuffers();
    else
        status = SDL_Flip(screen) == -1;
    Py_END_ALLOW_THREADS;

    if (status == -1)
        return RAISE(PyExc_SDLError, SDL_GetError());
    Py_RETURN_NONE;
#endif
}

/*BAD things happen when out-of-bound rects go to updaterect*/
static SDL_Rect*
pg_screencroprect (GAME_Rect* r, int w, int h, SDL_Rect* cur)
{
    if (r->x > w || r->y > h || (r->x + r->w) <= 0 || (r->y + r->h) <= 0)
        return 0;
    else {
        int right = MIN(r->x + r->w, w);
        int bottom = MIN(r->y + r->h, h);
        cur->x = (short) MAX(r->x, 0);
        cur->y = (short) MAX(r->y, 0);
        cur->w = (unsigned short) right - cur->x;
        cur->h = (unsigned short) bottom - cur->y;
    }
    return cur;
}

static PyObject*
pg_update (PyObject* self, PyObject* arg)
{
#ifdef SDL2
    SDL_Window* win = Py_GetDefaultWindow();
#else
    SDL_Surface* screen;
#endif
    GAME_Rect *gr, temp = { 0 };
    int wide, high;
    PyObject* obj;

    VIDEO_INIT_CHECK();

#ifdef SDL2
    if (!win)
        return RAISE(PyExc_SDLError, "Display mode not set");
    SDL_GetWindowSize(win, &wide, &high);
    if (SDL_GetWindowFlags(win) & SDL_WINDOW_OPENGL)
        return RAISE (PyExc_SDLError, "Cannot update an OPENGL display");
#else
    screen = SDL_GetVideoSurface();
    if (!screen)
        return RAISE(PyExc_SDLError, SDL_GetError());
    wide = screen->w;
    high = screen->h;
    if (screen->flags & SDL_OPENGL)
        return RAISE(PyExc_SDLError, "Cannot update an OPENGL display");
#endif

    /*determine type of argument we got*/
    if (PyTuple_Size (arg) == 0) {

#ifdef SDL2
        SDL_UpdateWindowSurface(win);
#else
        SDL_UpdateRect(screen, 0, 0, 0, 0);
#endif

        Py_RETURN_NONE;
    } else {
        obj = PyTuple_GET_ITEM(arg, 0);
        if (obj == Py_None)
            gr = &temp;
        else {
            gr = GameRect_FromObject(arg, &temp);
            if (!gr)
                PyErr_Clear();
            else if (gr != &temp) {
                memcpy(&temp, gr, sizeof(temp));
                gr = &temp;
            }
        }
    }

    if (gr) {
        SDL_Rect sdlr;

#ifdef SDL2
        if (pg_screencroprect(gr, wide, high, &sdlr))
            SDL_UpdateWindowSurfaceRects(win, &sdlr, 1);
#else
        if (pg_screencroprect(gr, wide, high, &sdlr))
            SDL_UpdateRect(screen, sdlr.x, sdlr.y, sdlr.w, sdlr.h);
#endif

    } else {
        PyObject* seq;
        PyObject* r;
        int loop, num, count;
        SDL_Rect* rects;
        if (PyTuple_Size(arg) != 1)
            return RAISE
                (PyExc_ValueError,
                 "update requires a rectstyle or sequence of recstyles");
        seq = PyTuple_GET_ITEM(arg, 0);
        if (!seq || !PySequence_Check(seq))
            return RAISE
                (PyExc_ValueError,
                 "update requires a rectstyle or sequence of recstyles");

        num = PySequence_Length(seq);
        rects = PyMem_New(SDL_Rect, num);
        if (!rects)
            return NULL;
        count = 0;
        for (loop = 0; loop < num; ++loop) {
            SDL_Rect* cur_rect = (rects + count);

            /*get rect from the sequence*/
            r = PySequence_GetItem(seq, loop);
            if (r == Py_None) {
                Py_DECREF(r);
                continue;
            }
            gr = GameRect_FromObject(r, &temp);
            Py_XDECREF(r);
            if (!gr) {
                PyMem_Free((char*)rects);
                return RAISE(PyExc_ValueError,
                             "update_rects requires a single list of rects");
            }

            if (gr->w < 1 && gr->h < 1)
                continue;

            /*bail out if rect not onscreen*/
            if (!pg_screencroprect(gr, wide, high, cur_rect))
                continue;

            ++count;
        }

        if (count) {
            Py_BEGIN_ALLOW_THREADS;
#ifndef SDL2
            SDL_UpdateRects(screen, count, rects);
#else /* SDL2 */
            SDL_UpdateWindowSurfaceRects(win, rects, count);
#endif /* SDL2 */
            Py_END_ALLOW_THREADS;
        }

        PyMem_Free((char*)rects);
    }
    Py_RETURN_NONE;
}

static PyObject*
pg_set_palette (PyObject* self, PyObject* args)
{
#ifdef SDL2
    PyObject* surface = Py_GetDefaultWindowSurface();
#endif /* SDL2 */
    SDL_Surface* surf;
    SDL_Palette* pal;
    SDL_Color* colors;
    PyObject* list, *item = NULL;
    int i, len;
    int r, g, b;

    VIDEO_INIT_CHECK();
    if (!PyArg_ParseTuple (args, "|O", &list))
        return NULL;
#ifndef SDL2
    surf = SDL_GetVideoSurface();
    if (!surf)
#else /* SDL2 */
    if (!surface)
#endif /* SDL2 */
        return RAISE(PyExc_SDLError, "No display mode is set");
#ifdef SDL2
    Py_INCREF(surface);
    surf = PySurface_AsSurface(surface);
#endif /* SDL2 */
    pal = surf->format->palette;
    if (surf->format->BytesPerPixel != 1 || !pal)
#ifdef SDL2
    {
        Py_DECREF(surface);
#endif /* SDL2 */
        return RAISE(PyExc_SDLError, "Display mode is not colormapped");
#ifdef SDL2
    }
#endif /* SDL2 */

    if (!list)
    {
#ifndef SDL2
        colors = pal->colors;
        len = pal->ncolors;
        SDL_SetPalette(surf, SDL_PHYSPAL, colors, 0, len);
#else /* SDL2 */
        Py_DECREF(surface);
#endif /* SDL2 */
        Py_RETURN_NONE;
    }

    if (!PySequence_Check(list))
#ifdef SDL2
    {
        Py_DECREF(surface);
#endif /* SDL2 */
        return RAISE (PyExc_ValueError, "Argument must be a sequence type");
#ifdef SDL2
    }
#endif /* SDL2 */

    len = MIN (pal->ncolors, PySequence_Length(list));

    colors = (SDL_Color*)malloc(len * sizeof(SDL_Color));
    if (!colors)
#ifndef SDL2
        return NULL;
#else /* SDL2 */
    {
        Py_DECREF(surface);
        return PyErr_NoMemory();
    }
#endif /* SDL2 */

    for (i = 0; i < len; i++)
    {
        item = PySequence_GetItem(list, i);
        if (!PySequence_Check(item) || PySequence_Length(item) != 3)
        {
            Py_DECREF(item);
            free((char*)colors);
#ifdef SDL2
            Py_DECREF(surface);
#endif /* SDL2 */
            return RAISE(PyExc_TypeError,
                         "takes a sequence of sequence of RGB");
        }
        if(!IntFromObjIndex(item, 0, &r) || !IntFromObjIndex(item, 1, &g)
           || !IntFromObjIndex(item, 2, &b))
#ifdef SDL2
        {
            Py_DECREF(item);
            free((char*)colors);
            Py_DECREF(surface);
#endif /* SDL2 */
            return RAISE(PyExc_TypeError,
                         "RGB sequence must contain numeric values");
#ifdef SDL2
        }
#endif /* SDL2 */

        colors[i].r = (unsigned char)r;
        colors[i].g = (unsigned char)g;
        colors[i].b = (unsigned char)b;
#ifdef SDL2
        colors[i].a = SDL_ALPHA_OPAQUE;
#endif /* SDL2 */

        Py_DECREF (item);
    }

#ifndef SDL2
    SDL_SetPalette (surf, SDL_PHYSPAL, colors, 0, len);
#else /* SDL2 */
    pal = SDL_AllocPalette(len);
    if (!pal)
    {
        free((char*)colors);
        Py_DECREF(surface);
        return RAISE(PyExc_SDLError, SDL_GetError ());
    }
    if (!SDL_SetPaletteColors(pal, colors, 0, len))
    {
        SDL_FreePalette(pal);
        free ((char*)colors);
        Py_DECREF(surface);
        return RAISE(PyExc_SDLError, SDL_GetError ());
    }
#endif /* SDL2 */

#ifdef SDL2
    SDL_SetSurfacePalette(surf, pal);
    SDL_FreePalette(pal);
#endif /* SDL2 */
    free ((char*)colors);
#ifdef SDL2
    Py_DECREF(surface);
#endif /* SDL2 */
    Py_RETURN_NONE;
}

static PyObject*
pg_set_gamma (PyObject* self, PyObject* arg)
{
#ifdef SDL2
    float r, g, b;
    int result = 0;
    _DisplayState* state = DISPLAY_MOD_STATE(self);
    SDL_Window* win = Py_GetDefaultWindow();
    Uint16* gamma_ramp;

    if (!PyArg_ParseTuple (arg, "f|ff", &r, &g, &b))
        return NULL;
    if (PyTuple_Size (arg) == 1)
        g = b = r;
    VIDEO_INIT_CHECK();

    gamma_ramp = (Uint16 *) malloc((3 * 256) * sizeof(Uint16));
    if (!gamma_ramp)
        return PyErr_NoMemory();
    SDL_CalculateGammaRamp(r, gamma_ramp);
    SDL_CalculateGammaRamp(g, gamma_ramp + 256);
    SDL_CalculateGammaRamp(b, gamma_ramp + 256);
    if (win) {
        result = SDL_SetWindowGammaRamp(win,
                                        gamma_ramp,
                                        gamma_ramp + 256,
                                        gamma_ramp + 512);
        if (result) {
            /* Discard a possibly faulty gamma ramp */
            free(gamma_ramp);
            gamma_ramp = NULL;
        }
    }
    if (gamma_ramp) {
        if (state->gamma_ramp)
            free(state->gamma_ramp);
        state->gamma_ramp = gamma_ramp;
    }
    return PyInt_FromLong(result == 0);
#else
    float r, g, b;
    int result = 0;
    if (!PyArg_ParseTuple(arg, "f|ff", &r, &g, &b))
        return NULL;
    if (PyTuple_Size(arg) == 1)
        g = b = r;
    VIDEO_INIT_CHECK();
    result = SDL_SetGamma(r, g, b);
    return PyInt_FromLong(result == 0);
#endif
}

static int
pg_convert_to_uint16 (PyObject* python_array, Uint16* c_uint16_array)
{
    int i;
    PyObject* item;

    if (!c_uint16_array) {
        RAISE(PyExc_RuntimeError, "Memory not allocated for c_uint16_array.");
        return 0;
    }
    if (!PySequence_Check(python_array)) {
        RAISE(PyExc_TypeError, "Array must be sequence type");
        return 0;
    }
    if (PySequence_Size(python_array) != 256) {
        RAISE(PyExc_ValueError, "gamma ramp must be 256 elements long");
        return 0;
    }
    for (i = 0; i < 256; i++) {
        item = PySequence_GetItem(python_array, i);
        if(!PyInt_Check(item)) {
            RAISE(PyExc_ValueError,
                  "gamma ramp must contain integer elements");
            return 0;
        }
        c_uint16_array[i] = (Uint16)PyInt_AsLong(item);
        Py_XDECREF(item);
    }
    return 1;
}

static PyObject*
pg_set_gamma_ramp (PyObject* self, PyObject* arg)
{
#ifdef SDL2
    _DisplayState* state = DISPLAY_MOD_STATE (self);
    SDL_Window* win = Py_GetDefaultWindow();
    Uint16* gamma_ramp = (Uint16 *) malloc((3 * 256) * sizeof(Uint16));
    Uint16 *r, *g, *b;
    int result;
    if (!gamma_ramp)
        return PyErr_NoMemory();
    r = gamma_ramp;
    g = gamma_ramp + 256;
    b = gamma_ramp + 512;
    if(!PyArg_ParseTuple(arg, "O&O&O&",
                         pg_convert_to_uint16, r,
                         pg_convert_to_uint16, g,
                         pg_convert_to_uint16, b)) {
        free(gamma_ramp);
        return NULL;
    }
    VIDEO_INIT_CHECK ();
    if (win) {
        result = SDL_SetWindowGammaRamp(win,
                                        gamma_ramp,
                                        gamma_ramp + 256,
                                        gamma_ramp + 512);
        if (result) {
            /* Discard a possibly faulty gamma ramp */
            free(gamma_ramp);
            gamma_ramp = NULL;
        }
    }
    if (gamma_ramp) {
        if (state->gamma_ramp)
            free(state->gamma_ramp);
        state->gamma_ramp = gamma_ramp;
    }
    return PyInt_FromLong (result == 0);
#else
    Uint16 *r, *g, *b;
    int result;
    r = (Uint16 *) malloc(256 * sizeof(Uint16));
    if (!r)
        return NULL;
    g = (Uint16 *) malloc(256 * sizeof(Uint16));
    if (!g) {
        free(r);
        return NULL;
    }
    b = (Uint16 *) malloc(256 * sizeof(Uint16));
    if (!b) {
        free(r);
        free(g);
        return NULL;
    }
    if(!PyArg_ParseTuple(arg, "O&O&O&",
                         pg_convert_to_uint16, r,
                         pg_convert_to_uint16, g,
                         pg_convert_to_uint16, b)) {
        free(r);
        free(g);
        free(b);
        return NULL;
    }
    VIDEO_INIT_CHECK ();
    result = SDL_SetGammaRamp(r, g, b);
    free((char*)r);
    free((char*)g);
    free((char*)b);
    return PyInt_FromLong(result == 0);
#endif
}

static PyObject*
pg_set_caption (PyObject* self, PyObject* arg)
{
#ifdef SDL2
    _DisplayState* state = DISPLAY_MOD_STATE(self);
    SDL_Window* win = Py_GetDefaultWindow();
    char *title, *icontitle=NULL;
    if (!PyArg_ParseTuple (arg, "s|s", &title, &icontitle))
        return NULL;
    if (state->title)
        free(state->title);
    state->title = (char *)malloc((strlen(title) + 1) * sizeof(char *));
    if (!state->title)
        return PyErr_NoMemory();
    strcpy(state->title, title);
    if (win)
        SDL_SetWindowTitle(win, title);
    Py_RETURN_NONE;
#else
    char* title, *icontitle=NULL;
    if (!PyArg_ParseTuple(arg, "s|s", &title, &icontitle))
        return NULL;
    if (!icontitle)
        icontitle = title;
    SDL_WM_SetCaption(title, icontitle);
    Py_RETURN_NONE;
#endif
}

static PyObject*
pg_get_caption (PyObject* self)
{
#ifdef SDL2
    _DisplayState* state = DISPLAY_MOD_STATE(self);
    SDL_Window* win = Py_GetDefaultWindow();
    const char* title = win ? SDL_GetWindowTitle(win) : state->title;

    if (title && title[0]) /* Conditional && */
        return Py_BuildValue ("(ss)", title, title);
    return Py_BuildValue ("()");
#else
    char* title, *icontitle;
    SDL_WM_GetCaption(&title, &icontitle);
    if (title && *title)
        return Py_BuildValue("(ss)", title, icontitle);
    return Py_BuildValue("()");
#endif
}


#ifndef SDL2
static void
pg_do_set_icon (PyObject *surface)
{
    SDL_Surface* surf = PySurface_AsSurface(surface);
    SDL_WM_SetIcon(surf, NULL);
    icon_was_set = 1;
}
#endif /* ! SDL2 */

static PyObject*
pg_set_icon (PyObject* self, PyObject* arg)
{
#ifdef SDL2
    _DisplayState* state = DISPLAY_MOD_STATE(self);
    SDL_Window* win = Py_GetDefaultWindow();
    PyObject* surface;
    if (!PyArg_ParseTuple(arg, "O!", &PySurface_Type, &surface))
        return NULL;
    if (!PyGame_Video_AutoInit())
        return RAISE(PyExc_SDLError, SDL_GetError());
#if (!defined(darwin))
    Py_INCREF(surface);
    Py_XDECREF(state->icon);
    state->icon = surface;
    if (win)
        SDL_SetWindowIcon(win, PySurface_AsSurface(surface));
#endif
    Py_RETURN_NONE;
#else
    PyObject* surface;
    if (!PyArg_ParseTuple(arg, "O!", &PySurface_Type, &surface))
        return NULL;
    if (!PyGame_Video_AutoInit())
        return RAISE(PyExc_SDLError, SDL_GetError());
    pg_do_set_icon(surface);
    Py_RETURN_NONE;
#endif
}

static PyObject*
pg_iconify (PyObject* self)
{
#ifdef SDL2
    SDL_Window* win = Py_GetDefaultWindow();
    VIDEO_INIT_CHECK();
    if (!win)
        return RAISE (PyExc_SDLError, "No open window");
    SDL_MinimizeWindow(win);
#warning "Does this send the app an SDL_ActiveEvent loss event?";
    return PyInt_FromLong(1);
#else
    int result;
    VIDEO_INIT_CHECK();
    result = SDL_WM_IconifyWindow();

    /* If the application is running in a window managed environment SDL attempts to
       iconify/minimise it. If SDL_WM_IconifyWindow is successful, the application will
       receive a SDL_APPACTIVE loss event (see SDL_ActiveEvent).
    */
    return PyInt_FromLong(result != 0);
#endif
}

static PyObject*
pg_toggle_fullscreen (PyObject* self)
{
#ifdef SDL2
    SDL_Window* win = Py_GetDefaultWindow();
    int result;
    VIDEO_INIT_CHECK();
    if (!win)
        return RAISE (PyExc_SDLError, "No open window");
    if (SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN)
        result = SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN);
    else
        result = SDL_SetWindowFullscreen(win, 0);
    return PyInt_FromLong (result != 0);
#else
    SDL_Surface* screen;
    int result;
    VIDEO_INIT_CHECK();
    screen = SDL_GetVideoSurface();
    if (!screen)
        return RAISE(PyExc_SDLError, SDL_GetError());

    result = SDL_WM_ToggleFullScreen(screen);
    return PyInt_FromLong(result != 0);
#endif
}

static PyMethodDef _pg_display_methods[] =
{
    { "__PYGAMEinit__", pg_display_autoinit, 1,
      "auto initialize function for display." },
    { "init", (PyCFunction) pg_init, METH_NOARGS, DOC_PYGAMEDISPLAYINIT },
    { "quit", (PyCFunction) pg_quit, METH_NOARGS, DOC_PYGAMEDISPLAYQUIT },
    { "get_init", (PyCFunction) pg_get_init, METH_NOARGS,
      DOC_PYGAMEDISPLAYGETINIT },
    { "get_active", (PyCFunction) pg_get_active, METH_NOARGS,
      DOC_PYGAMEDISPLAYGETACTIVE },

/*    { "set_driver", set_driver, 1, doc_set_driver },*/
    { "get_driver", (PyCFunction) pg_get_driver, METH_NOARGS,
      DOC_PYGAMEDISPLAYGETDRIVER },
#ifndef SDL2
    { "get_wm_info", (PyCFunction) pg_get_wm_info, METH_NOARGS,
      DOC_PYGAMEDISPLAYGETWMINFO },
    { "Info", (PyCFunction) pgInfo, METH_NOARGS, DOC_PYGAMEDISPLAYINFO },
#endif /* ! SDL2 */
    { "get_surface", (PyCFunction) pg_get_surface, METH_NOARGS,
      DOC_PYGAMEDISPLAYGETSURFACE },

    { "set_mode", pg_set_mode, METH_VARARGS, DOC_PYGAMEDISPLAYSETMODE },
    { "mode_ok", pg_mode_ok, METH_VARARGS, DOC_PYGAMEDISPLAYMODEOK },
    { "list_modes", pg_list_modes, METH_VARARGS, DOC_PYGAMEDISPLAYLISTMODES },

    { "flip", (PyCFunction) pg_flip, METH_NOARGS, DOC_PYGAMEDISPLAYFLIP },
    { "update", pg_update, METH_VARARGS, DOC_PYGAMEDISPLAYUPDATE },

    { "set_palette", pg_set_palette, METH_VARARGS, DOC_PYGAMEDISPLAYSETPALETTE },
    { "set_gamma", pg_set_gamma, METH_VARARGS, DOC_PYGAMEDISPLAYSETGAMMA },
    { "set_gamma_ramp", pg_set_gamma_ramp, METH_VARARGS,
      DOC_PYGAMEDISPLAYSETGAMMARAMP },

    { "set_caption", pg_set_caption, METH_VARARGS, DOC_PYGAMEDISPLAYSETCAPTION },
    { "get_caption", (PyCFunction) pg_get_caption, METH_NOARGS,
      DOC_PYGAMEDISPLAYGETCAPTION },
    { "set_icon", pg_set_icon, METH_VARARGS, DOC_PYGAMEDISPLAYSETICON },

    { "iconify", (PyCFunction) pg_iconify, METH_NOARGS, DOC_PYGAMEDISPLAYICONIFY },
    { "toggle_fullscreen", (PyCFunction) pg_toggle_fullscreen, METH_NOARGS,
      DOC_PYGAMEDISPLAYTOGGLEFULLSCREEN },

    { "gl_set_attribute", pg_gl_set_attribute, METH_VARARGS,
      DOC_PYGAMEDISPLAYGLSETATTRIBUTE },
    { "gl_get_attribute", pg_gl_get_attribute, METH_VARARGS,
      DOC_PYGAMEDISPLAYGLGETATTRIBUTE },

    { NULL, NULL, 0, NULL }
};

#ifdef SDL2
#if PY3
static struct PyModuleDef _module = {
    PyModuleDef_HEAD_INIT,
    "display",
    DOC_PYGAMEDISPLAY,
    sizeof(_DisplayState),
    _pg_display_methods,
#warning At some point should add GC slot functions.
    NULL, NULL, NULL, NULL
};
#endif
#endif /* SDL2 */

MODINIT_DEFINE (display)
{
#ifdef SDL2
    PyObject* module;
    _DisplayState* state;
#else
    PyObject *module, *dict, *apiobj;
    int ecode;
    static void* c_api[PYGAMEAPI_DISPLAY_NUMSLOTS];
#if PY3
    static struct PyModuleDef _module = {
        PyModuleDef_HEAD_INIT,
        "display",
        DOC_PYGAMEDISPLAY,
        -1,
        _pg_display_methods,
        NULL, NULL, NULL, NULL
    };
#endif /* PY3 */
#endif


    /* imported needed apis; Do this first so if there is an error
       the module is not loaded.
    */
    import_pygame_base ();
    if (PyErr_Occurred ()) {
        MODINIT_ERROR;
    }
    import_pygame_rect ();
    if (PyErr_Occurred ()) {
        MODINIT_ERROR;
    }
    import_pygame_surface ();
    if (PyErr_Occurred ()) {
        MODINIT_ERROR;
    }
#ifdef SDL2
    if (import_pygame__window ()) {
        MODINIT_ERROR;
    }
#endif /* SDL2 */

#ifndef SDL2
    /* type preparation */
    if (PyType_Ready (&pgVidInfo_Type) < 0) {
        MODINIT_ERROR;
    }
#endif /* ! SDL2 */


    /* create the module */
#if PY3
    module = PyModule_Create (&_module);
#else
    module = Py_InitModule3 (MODPREFIX "display",
                             _pg_display_methods,
                             DOC_PYGAMEDISPLAY);
#endif
    if (module == NULL) {
        MODINIT_ERROR;
    }
#ifndef SDL2
    dict = PyModule_GetDict (module);

    /* export the c api */
    c_api[0] = &pgVidInfo_Type;
    c_api[1] = pgVidInfo_New;
    apiobj = encapsulate_api (c_api, "display");
    if (apiobj == NULL) {
        DECREF_MOD (module);
        MODINIT_ERROR;
    }
    ecode = PyDict_SetItemString (dict, PYGAMEAPI_LOCAL_ENTRY, apiobj);
    Py_DECREF (apiobj);
    if (ecode) {
        DECREF_MOD (module);
        MODINIT_ERROR;
    }
#else /* SDL2 */
    Py_INCREF (pgWindow_Type);
    if (PyModule_AddObject (module,
                            "Window",
                            (PyObject *)pgWindow_Type)) {
        Py_DECREF (pgWindow_Type);
        DECREF_MOD (module);
        MODINIT_ERROR;
    }
    Py_INCREF (pgWindowSurface_Type);
    if (PyModule_AddObject (module,
                            "WindowSurface",
                            (PyObject *)pgWindowSurface_Type)) {
        Py_DECREF (pgWindowSurface_Type);
        DECREF_MOD (module);
        MODINIT_ERROR;
    }
    Py_INCREF (pgWindowSurface_Type);
    if (PyModule_AddObject (module,
                            "WindowRenderer",
                            (PyObject *)pgWindowRenderer_Type)) {
        Py_DECREF (pgWindowRenderer_Type);
        DECREF_MOD (module);
        MODINIT_ERROR;
    }

    state = DISPLAY_MOD_STATE (module);
    state->title = NULL;
    state->icon = NULL;
    state->gamma_ramp = NULL;
#endif /* SDL2 */
    MODINIT_RETURN (module);
}
