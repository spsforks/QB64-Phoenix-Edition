
#include "libqb-common.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <list>
#include <queue>
#include <unordered_map>
#include "GL/glew.h"

// note: MacOSX uses Apple's GLUT not FreeGLUT
#ifdef QB64_MACOSX
#    include <GLUT/glut.h>
#else
#    define CORE_FREEGLUT
#    include "freeglut.h"
#endif

#include "mutex.h"
#include "thread.h"
#include "completion.h"
#include "gui.h"
#include "mac-key-monitor.h"
#include "glut-thread.h"

// FIXME: These extern variable and function definitions should probably go
// somewhere more global so that they can be referenced by libqb.cpp
extern uint8_t *window_title;
extern int32_t framebufferobjects_supported;
extern int32_t screen_hide;

void MAIN_LOOP(void *);
void GLUT_KEYBOARD_FUNC(unsigned char key, int x, int y);
void GLUT_DISPLAY_REQUEST();
void GLUT_KEYBOARDUP_FUNC(unsigned char key, int x, int y);
void GLUT_SPECIAL_FUNC(int key, int x, int y);
void GLUT_SPECIALUP_FUNC(int key, int x, int y);
void GLUT_MOUSE_FUNC(int glut_button, int state, int x, int y);
void GLUT_MOTION_FUNC(int x, int y);
void GLUT_PASSIVEMOTION_FUNC(int x, int y);
void GLUT_RESHAPE_FUNC(int width, int height);

#ifdef QB64_WINDOWS
void GLUT_TIMER_EVENT(int ignore);
#else
void GLUT_IDLEFUNC();
#endif

#ifdef CORE_FREEGLUT
void GLUT_MOUSEWHEEL_FUNC(int wheel, int direction, int x, int y);
#endif

static void glutWarning(const char *fmt, va_list lst) {
    // This keeps FreeGlut from dumping warnings to console
}

// Performs all of the FreeGLUT initialization except for calling glutMainLoop()
static void initialize_glut(int argc, char **argv) {
#    ifdef CORE_FREEGLUT
    glutInitWarningFunc(glutWarning);
    glutInitErrorFunc(glutWarning);
#    endif

    glutInit(&argc, argv);

    mac_register_key_handler();

#    ifdef QB64_WINDOWS
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
#    else
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
#    endif

    glutInitWindowSize(640, 400); // cannot be changed unless display_x(etc) are modified

    if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) // must be called on Linux or GLUT crashes
    {
        exit(1);
    }

    if (!window_title) {
        glutCreateWindow("Untitled");
    } else {
        glutCreateWindow((char *)window_title);
    }

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        gui_alert((char *)glewGetErrorString(err));
    }

    if (glewIsSupported("GL_EXT_framebuffer_object"))
        framebufferobjects_supported = 1;

    glutDisplayFunc(GLUT_DISPLAY_REQUEST);

#    ifdef QB64_WINDOWS
    glutTimerFunc(8, GLUT_TIMER_EVENT, 0);
#    else
    glutIdleFunc(GLUT_IDLEFUNC);
#    endif

    glutKeyboardFunc(GLUT_KEYBOARD_FUNC);
    glutKeyboardUpFunc(GLUT_KEYBOARDUP_FUNC);
    glutSpecialFunc(GLUT_SPECIAL_FUNC);
    glutSpecialUpFunc(GLUT_SPECIALUP_FUNC);
    glutMouseFunc(GLUT_MOUSE_FUNC);
    glutMotionFunc(GLUT_MOTION_FUNC);
    glutPassiveMotionFunc(GLUT_PASSIVEMOTION_FUNC);
    glutReshapeFunc(GLUT_RESHAPE_FUNC);

#    ifdef CORE_FREEGLUT
    glutMouseWheelFunc(GLUT_MOUSEWHEEL_FUNC);
#    endif
}

static bool glut_is_started;
static struct completion glut_thread_starter;
static struct completion *glut_thread_initialized;

void libqb_start_glut_thread() {
    if (glut_is_started)
        return;

    struct completion init;
    completion_init(&init);

    glut_thread_initialized = &init;

    completion_finish(&glut_thread_starter);

    completion_wait(&init);
    completion_clear(&init);
}

// Checks whether the GLUT thread is running
bool libqb_is_glut_up() {
    return glut_is_started;
}

void libqb_glut_presetup(int argc, char **argv) {
    if (!screen_hide) {
        initialize_glut(argc, argv); // Initialize GLUT if the screen isn't hidden
        glut_is_started = true;
    } else {
        completion_init(&glut_thread_starter);
    }
}

void libqb_start_main_thread(int argc, char **argv) {

    // Start the 'MAIN_LOOP' in a separate thread, as GLUT has to run on the
    // initial thread.
    struct libqb_thread *main_loop = libqb_thread_new();
    libqb_thread_start(main_loop, MAIN_LOOP, NULL);

    // This happens for $SCREENHIDE programs. This thread waits on the
    // `glut_thread_starter` completion, which will get completed if a
    // _ScreenShow is used.
    if (!glut_is_started) {
        completion_wait(&glut_thread_starter);

        initialize_glut(argc, argv);
        glut_is_started = true;

        if (glut_thread_initialized)
            completion_finish(glut_thread_initialized);
    }

    glutMainLoop();
}
