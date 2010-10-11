SO_EXT                    = dll
SO_LDFLAGS                = -shared
SO_CFLAGS                 = -shared
SO_LIBS                  :=

EXE_EXT                   = .exe

# TODO: config.h
CFLAGS                   += -DSHARED_EXT=\"$(SO_EXT)\"
CFLAGS                   += -DGETTEXT_STATIC
# Windows XP is the minimum we need
CFLAGS                   += -DWINVER=0x501

OPENGL_LIBS              ?= -lopengl32
OPENAL_LIBS              ?= -lOpenAL32
PKG_CONFIG               ?= $(CROSS)pkg-config
SDL_TTF_LIBS             ?= $(call PKG_LIBS,SDL_ttf) $(call PKG_LIBS,freetype2)
SDL_TTF_CFLAGS           ?= $(call PKG_CFLAGS,SDL_ttf) $(call PKG_CFLAGS,freetype2)
INTL_LIBS                ?= -lintl

ufo_LDFLAGS              += -lws2_32 -lwinmm -lgdi32 -lfreetype
ufoded_LDFLAGS           += -lws2_32 -lwinmm -lgdi32
testall_LDFLAGS          += -lws2_32 -lwinmm -lgdi32 -lfreetype
ufo2map_LDFLAGS          += -lwinmm
ufomodel_LDFLAGS         += -lwinmm
uforadiant_LDFLAGS       += -lglib-2.0 -lgtk-win32-2.0 -lgobject-2.0
