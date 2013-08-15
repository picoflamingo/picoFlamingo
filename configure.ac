dnl picoFlamingo
dnl Author: David Martinez Oliveira <dmo@papermint-designs.com>
dnl
dnl __LICENSE__
dnl


m4_include([version.m4])
AC_INIT([picoFlamingo], VERSION_NUMBER, [dmo@papermint-designs.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])


# Checks for programs
AC_PROG_CC
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

# Checks for libraries
# FIXME: Here we should check for OpenScenegraph, xine, etc...
AC_CHECK_LIB(dl, main)

# Checks for header files
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

# Cheks for typedefs, structures and compiler characteristics
# Checks for library functions

dnl AC_CHECK_LIB([freetype], [FT_Init_FreeType], [FREETYPE_FOUND="yes"], [FREETYPE_FOUND="no"])

dnl if test $FREETYPE_FOUND = no; then
dnl	AC_MSG_ERROR([--- Freetype not found.... Freetype is required])
dnl fi

# Configure options
AC_ARG_ENABLE(debug,
              [  --enable-debug          Enable debugging information],
              USE_DEBUG="$enableval", USE_DEBUG="no")


if test $USE_DEBUG = yes ; then 
  DEBUG=1
  BUILD_FLAGS="-Wall -g -O0 -DDEBUG "
else
  BUILD_FLAGS="-Wall -O3 "
  DEBUG=0
fi

PF_FLAGS="${PF_FLAGS} -Wall -D_GNU_SOURCE=1 -D_REENTRANT "
dnl Process configure parameters

AC_ARG_WITH(target-dir,
	    AC_HELP_STRING([--with-target-dir=path_to_OpenGL_libs],
			   [Select OpenGL ES dir [default=/usr/local/]]),
	    [], [with_target_dir="/usr/local/"])

PF_FLAGS="${PF_FLAGS}  -I. -I../include -I.support -I$with_target_dir/include "
PF_FLAGS="${PF_FLAGS}  -I/usr/include/freetype2 "

PF_LIBS="${PF_LIBS} -L$with_target_dir/lib "
PF_LIBS="${PF_LIBS} -lEGL -lGLESv2 -lm -lfreetype -lz "

AC_ARG_WITH(target,
	    AC_HELP_STRING([--with-target=@<:@x11/fb/rpi@:>@],
			   [Select picoFlamingo target [default=x11]]),
	    [], [with_target="x11"])

case "$with_target" in
	x11) PF_LIBS="${PF_LIBS} -lX11" ;;
	fb) ;;
	rpi) PF_FLAGS="-DRPI ${PF_FLAGS} -I/opt/vc/include/interface/vcos/pthreads" 
	PF_LIBS="${PF_LIBS} -lbcm_host -lvcos -lvchiq_arm"
;;
	*) AC_MSG_ERROR([Invalid target: must be x11 or fb]) ;;
esac

PLATFORM="$with_target"

VNUM="VERSION_NUMBER"
AC_SUBST(VNUM)

CFLAGS=" ${BUILD_FLAGS}"
AC_SUBST(BUILD_FLAGS)

MY_TOP_DIR="`pwd`"


AC_SUBST(MY_TOP_DIR)

case "$build_os" in
        linux*)
                AC_MSG_RESULT([--------------------------])
                AC_MSG_RESULT([Building on Linux Platform])
                AC_MSG_RESULT([--------------------------])
                ;;

        *)
                AC_MSG_WARN([------------------------------------])
                AC_MSG_WARN([Unknow target OS. Build could fail])
                AC_MSG_WARN([Falling back to UNIX default options])
                AC_MSG_WARN([------------------------------------])
        ;;

esac

case "$build_cpu" in
        x86_64*)
                AC_MSG_RESULT([--------------------------])
                AC_MSG_RESULT([Building on PC 64 bits. Forcing 32 bits])
                AC_MSG_RESULT([--------------------------])
		PF_FLAGS="-m32 ${PF_FLAGS} "
                ;;

        *)
                AC_MSG_WARN([picoF:: Platform is OK])
        ;;

esac
dnl Uncomment for special management of prefix variable
dnl 
if test "$prefix" = "NONE"; then
	PF_BASEDIR="/usr/local"
else
	PF_BASEDIR="$prefix"
fi

dnl if test "$PLATFORM" = ""; then
dnl 	PLATFORM=x11
dnl	AC_SUBST(PLATFORM)
dnl fi

PF_DATADIR="$PF_BASEDIR/share/pf"

AC_SUBST(PF_BASEDIR)
AC_SUBST(PF_DATADIR)
AC_SUBST(PF_LIBS)
AC_SUBST(PF_FLAGS)
AC_SUBST(PLATFORM)

# Output Files
AC_CONFIG_FILES([Makefile \
		include/Makefile \
		apps/Makefile\
		apps/video_server/Makefile \
		doc/Makefile \
		shaders/Makefile \
		fonts/Makefile \
		slides/Makefile \
		slides/pf/Makefile \
		src/const.h \
		src/Makefile])
AC_OUTPUT


AC_MSG_RESULT(--- Building for $build_cpu in a $build_os)
AC_MSG_RESULT(--- OS          : ${build_os})
AC_MSG_RESULT(--- HOST        : ${host})
AC_MSG_RESULT(--- VERSION     : ${VNUM})
AC_MSG_RESULT(--- Platform    : ${PLATFORM})
AC_MSG_RESULT(--- Prefix      : ${PF_BASEDIR})
AC_MSG_RESULT(--- Data dir    : ${PF_DATADIR})
AC_MSG_RESULT(--- BUILD_FLAGS : ${BUILD_FLAGS})
AC_MSG_RESULT(--- PF_FLAGS    : ${PF_FLAGS})
AC_MSG_RESULT(--- PF_LIBS     : ${PF_LIBS})
