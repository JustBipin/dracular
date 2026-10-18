/* Defines if force the use DRM in okular */
#define OKULAR_FORCE_DRM ${_OKULAR_FORCE_DRM}

/* Defines if the purpose framework is available */
#cmakedefine01 HAVE_PURPOSE

/* Defines if qtspeech is available */
#cmakedefine01 HAVE_SPEECH

/* Defines if kwallet is available */
#cmakedefine01 HAVE_KWALLET

/* Defines if QJSEngine is available*/
#cmakedefine01 HAVE_JS

/* Defines whether the malloc_trim method from malloc.h is available */
#cmakedefine01 HAVE_MALLOC_TRIM

/* Defines whether we are building with X11 support */
#cmakedefine01 HAVE_X11


/* Defines if qtmultimedia is available */
#cmakedefine01 HAVE_MULTIMEDIA

/* Defines whether Poppler's SplashOutputDev constructor still takes a reverseVideo argument */
#cmakedefine01 HAVE_POPPLER_SPLASH_NO_REVERSE_VIDEO

/* Defines whether we are building with dbus enabled */
#cmakedefine01 HAVE_DBUS
