#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <epoxy/egl.h>

#include <libportal/portal.h>
#include <libportal/remote.h>
#include <glib.h>
#include "xdg-shell.h"


struct context {
  GMainLoop *loop;

  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;

  struct xdg_wm_base *xdg_wm_base;
  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_egl_window *egl_window;
  struct wl_pointer *pointer;
  struct wl_keyboard *keyboard;

  EGLDisplay egl_display;
  EGLConfig egl_config;
  EGLContext egl_context;
  EGLSurface egl_surface;

  int32_t width;
  int32_t height;
  uint8_t running;

  XdpPortal     *portal;
  XdpSession    *session;
  GCancellable  *cancellable;
};


/** XGD SURFACE */

static void xdg_surface_configure(void *data, struct xdg_surface *surface, uint32_t serial) {
  struct context *ctx = data;
  xdg_surface_ack_configure(surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
  .configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
  .ping = xdg_wm_base_ping,
};

/** XGD SURFACE END */

/** POINTER */

static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
                           struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy) {
  // Handle pointer enter event
  printf("Pointer entered the surface\n");
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial,
                           struct wl_surface *surface) {
  // Handle pointer leave event
  printf("Pointer left the surface\n");
}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time,
                            wl_fixed_t sx, wl_fixed_t sy) {
  // Handle pointer motion event
  printf("Pointer moved to (%f, %f)\n", wl_fixed_to_double(sx), wl_fixed_to_double(sy));
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial,
                            uint32_t time, uint32_t button, uint32_t state) {
  // Handle pointer button event
  printf("Button %u %s at %u\n", button, (state == WL_POINTER_BUTTON_STATE_PRESSED) ? "pressed" : "released", time);
}

const struct wl_pointer_listener pointer_listener = {
  pointer_handle_enter,
  pointer_handle_leave,
  pointer_handle_motion,
  pointer_handle_button,
  // Add other event handlers if needed
};

/** POINTER END **/

/** KEYBOARD **/

static  void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size) {
  // Handle keymap event
  printf("Received keymap event\n");
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                                  struct wl_surface *surface, struct wl_array *keys) {
  // Handle keyboard enter event
  printf("Keyboard entered the surface\n");
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {
    // Handle keyboard leave event
    printf("Keyboard left the surface\n");
}

static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time,
                                 uint32_t key, uint32_t state) {
  struct context *ctx = data;
  // Handle key event
  printf("Key %u %s at %u\n", key, (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? "pressed" : "released", time);

  // 45 == x
  if (key == 45 && (state = WL_KEYBOARD_KEY_STATE_PRESSED && ctx->session)) {
    printf("MOVING FAKE MOUSE\n");
    xdp_session_pointer_motion(ctx->session, 10, 10);
  } else if (key == 16 && (state == WL_KEYBOARD_KEY_STATE_PRESSED)) {
    g_main_loop_quit(ctx->loop);
  }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed,
                                      uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
  // Handle modifier keys event
  printf("Modifiers: Depressed: %u, Latched: %u, Locked: %u, Group: %u\n", mods_depressed, mods_latched, mods_locked, group);
}

static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *keyboard, int32_t rate, int32_t delay) {
  // Handle repeat rate and delay information
  printf("Repeat rate: %d, Delay: %d\n", rate, delay);
}

const struct wl_keyboard_listener keyboard_listener = {
  keyboard_handle_keymap,
  keyboard_handle_enter,
  keyboard_handle_leave,
  keyboard_handle_key,
  keyboard_handle_modifiers,
  keyboard_handle_repeat_info,
};

/** KEYBOARD END **/

/** OUTPUT / MONITOR **/
static void output_handle_geometry(void *data, struct wl_output *output, int32_t x, int32_t y,
                                   int32_t physical_width, int32_t physical_height,
                                   int32_t subpixel, const char *make, const char *model,
                                   int32_t transform) {
  // Handle output geometry information
  printf("MONITOR GEOMETRY is %d,%d %dX%d, maker %s, model %s\n", x, y, physical_width, physical_height, make, model);
}

static void output_handle_mode(void *data, struct wl_output *output, uint32_t flags, int32_t width,
                              int32_t height, int32_t refresh) {
  // Handle output mode information
  printf("MONITOR MODE is %dX%d, refresh %d\n", width, height, refresh);
}

static void output_handle_done(void *data, struct wl_output *wl_output) {}


static void output_handle_scale(void* data, struct wl_output* wl_output, int factor) {
  // Handle output mode information
  printf("SCALE is %d\n", factor);
}
/** OUTPUT / MONITOR END **/

const struct wl_output_listener output_listener = {
  output_handle_geometry,
  output_handle_mode,
  output_handle_done,
  output_handle_scale,
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface,
                                   uint32_t version) {

  printf("interface: '%s', version: %d, name: %d\n", interface, version, name);

  struct context *ctx = data;
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    ctx->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    ctx->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(ctx->xdg_wm_base, &xdg_wm_base_listener, ctx);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    struct wl_seat *seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    ctx->pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(ctx->pointer, &pointer_listener, ctx);

    ctx->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(ctx->keyboard, &keyboard_listener, ctx);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    printf("OUTPUT/MONITOR");
    struct wl_output *output = wl_registry_bind(registry, name, &wl_output_interface, 2);
    wl_output_add_listener(output, &output_listener, ctx);
  }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {

}

static const struct wl_registry_listener registry_listener = {
  .global = registry_handle_global,
  .global_remove = registry_handle_global_remove,
};


/***
 * EGL
 */

static void egl_init(struct context *ctx) {
  EGLint major;
  EGLint minor;
  EGLint num_configs;
  EGLint attribs[] = {
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_BIT,
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,
        EGL_RED_SIZE,             8,
        EGL_GREEN_SIZE,           8,
        EGL_BLUE_SIZE,            8,
        EGL_ALPHA_SIZE,           8,
        EGL_NONE,
    };

    ctx->egl_window = wl_egl_window_create(ctx->surface, ctx->width, ctx->height);
    ctx->egl_display = eglGetDisplay((EGLNativeDisplayType) ctx->display);
    eglBindAPI(EGL_OPENGL_API);

  if (ctx->egl_display == EGL_NO_DISPLAY) {
	  fprintf(stderr, "Couldn't get EGL display\n");
  }

  if (eglInitialize(ctx->egl_display, &major, &minor) != EGL_TRUE) {
	  fprintf(stderr, "Couldnt initialize EGL\n");
  }

  fprintf(stderr, "EGL ver: %d.%d\n", major, minor);


  if (eglChooseConfig(ctx->egl_display, attribs, &ctx->egl_config, 1, &num_configs) != EGL_TRUE) {
	  fprintf(stderr, "Couldnt find matching EGL config\n");
  }

  ctx->egl_surface = eglCreateWindowSurface(ctx->egl_display,
						  ctx->egl_config, (EGLNativeWindowType) ctx->egl_window, NULL);

  if (ctx->egl_surface == EGL_NO_SURFACE) {
	  fprintf(stderr, "Couldn't create EGL surface\n");
  }

  ctx->egl_context = eglCreateContext(ctx->egl_display, ctx->egl_config, EGL_NO_CONTEXT, NULL);

  if (ctx->egl_context == EGL_NO_CONTEXT) {
	  fprintf(stderr, "Couldn't create EGL context %d\n", eglGetError());
  }

  if (!eglMakeCurrent(ctx->egl_display, ctx->egl_surface, ctx->egl_surface, ctx->egl_context)) {
	  fprintf(stderr, "Couldn't make EGL context current\n");
  }
}

static void draw(struct context *ctx) {
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  eglSwapBuffers(ctx->egl_display, ctx->egl_surface);
}

static void on_remote_session_started(XdpSession   *session,
                                      GAsyncResult *res,
                                      gpointer      data) {
  struct context *ctx = data;
  printf("Remote session started\n");
  if (!(xdp_session_get_devices (session) & XDP_DEVICE_POINTER)
    || !(xdp_session_get_devices (session) & XDP_DEVICE_KEYBOARD)) {

    printf("FAILED TO GET DEVICES.\n");
  }
}

static void on_remote_session_created(XdpPortal    *portal,
                                      GAsyncResult *result,
                                      gpointer      data) {
  g_autoptr (GError) error = NULL;
  struct context *ctx = data;
  printf("Remote session created\n");

  ctx->session = xdp_portal_create_remote_desktop_session_finish(portal, result, &error);

  xdp_session_start(ctx->session,
                     NULL,
                     NULL,
                     (GAsyncReadyCallback)on_remote_session_started,
                     ctx);
}

gboolean continuous_callback(gpointer data) {
  struct context *ctx = data;
  wl_display_dispatch_pending(ctx->display);
  draw(ctx);
  return TRUE;
}

int main(int argc, char *argv[]) {
  struct context ctx = { 0 };

  ctx.display = wl_display_connect(NULL);
  ctx.registry = wl_display_get_registry(ctx.display);
  wl_registry_add_listener(ctx.registry, &registry_listener, &ctx);
  wl_display_roundtrip(ctx.display);

  ctx.surface = wl_compositor_create_surface(ctx.compositor);
  ctx.xdg_surface = xdg_wm_base_get_xdg_surface(ctx.xdg_wm_base, ctx.surface);
  xdg_surface_add_listener(ctx.xdg_surface, &xdg_surface_listener, &ctx);
  ctx.xdg_toplevel = xdg_surface_get_toplevel(ctx.xdg_surface);
  xdg_toplevel_set_title(ctx.xdg_toplevel, "Example client");

  wl_surface_commit(ctx.surface);
  ctx.width = 300;
  ctx.height = 300;
  ctx.running = 1;
  egl_init(&ctx);

  ctx.loop = g_main_loop_new (NULL, FALSE);

  GSettings *mouseSettings = g_settings_new("org.gnome.desktop.peripherals.mouse");
  gint doubleClick = g_settings_get_int(mouseSettings, "double-click");
  printf("Mouse DC Speed: %d\n", doubleClick);
  g_object_unref(mouseSettings);


  ctx.portal = xdp_portal_new();
  printf("Will create remote desktop session...");
  xdp_portal_create_remote_desktop_session(ctx.portal,
                                        (XDP_DEVICE_KEYBOARD | XDP_DEVICE_POINTER),
                                        XDP_OUTPUT_MONITOR,
                                        XDP_REMOTE_DESKTOP_FLAG_NONE,
                                        XDP_CURSOR_MODE_HIDDEN,
                                        ctx.cancellable,
                                        (GAsyncReadyCallback)on_remote_session_created,
                                        &ctx);

  g_timeout_add(16, continuous_callback, &ctx);

  g_main_loop_run(ctx.loop);
  g_main_loop_unref(ctx.loop);

  xdp_session_close(ctx.session);
  //g_free(ctx.portal);

  eglDestroySurface(ctx.egl_display, ctx.egl_surface);
  eglDestroyContext(ctx.egl_display, ctx.egl_context);
  wl_egl_window_destroy(ctx.egl_window);
  xdg_toplevel_destroy(ctx.xdg_toplevel);
  xdg_surface_destroy(ctx.xdg_surface);
  wl_surface_destroy(ctx.surface);
  wl_display_disconnect(ctx.display);

  return 0;
}
