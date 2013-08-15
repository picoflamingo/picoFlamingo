# picoFlamingo: 3D Portable Presentation System
# Copyright (c) 2010, 2011 David Martínez Oliveira
#
# This file is part of picoFlamingo
#
# picoFlamingo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# picoFlamingo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with picoFlamingo.  If not, see <http://www.gnu.org/licenses/>.

#AM_CFLAGS=-m32 -Wall -D_GNU_SOURCE=1 -D_REENTRANT -I$(srcdir)/../include
#AM_CFLAGS+=-I. -I../include -I../${PLATFORM}/include -I./support -I/usr/include/freetype2
AM_CFLAGS=${BUILD_FLAGS} ${PF_FLAGS} -DPF_VERSION="\"${VNUM}\""
SOURCES_SYS=./${PLATFORM}/src/render_engine_sys.c

# mobiNET main application
bin_PROGRAMS=picoFlamingo

picoFlamingo_SOURCES=main.c render_engine.c pf_cube1.c \
        pf_mesh.c pf_render_item.c pf_quad.c pf_group.c \
        pf_text.c  pf_stext.c pf_font.c pf_image.c pf_3d.c \
        matrix.c shader.c  3ds.c pf_fx.c \
        input.c pfs_files.c pfs_cmd.c video_net.c \
	pf_tex_mng.c pf_sphere.c \
        support/stb_image.c ${SOURCES_SYS}

#picoFlamingo_LDADD=-L../${PLATFORM}/libs  -L/usr/lib -L/usr/lib32 -lEGL -lGLESv2 -lm -lfreetype -lz -lX11
picoFlamingo_LDADD=${PF_LIBS}


#EXTRA_DIST=
#include_HEADERS=

EXTRA_DIST= 3ds.h const.h.in input.h log.h matrix.h pf_3d.h pf_font.h \
pf_fx.h pf_image.h pf_mesh.h pf_quad.h pf_render_item.h pfs_cmd.h pfs_files.h pf_stext.h \
pf_text.h render_engine.h shader.h video_net.h pf_tex_mng.h pf_group.h \
support/stb_image.h \
x11/src/render_engine_sys.c fb/src/render_engine_sys.c rpi/src/render_engine_sys.c
