#include "rk_irfpa_log.h"
#include "rk_irfpa_version.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#define MAX_CALLBACKS 4

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

typedef struct {
  log_LogFn fn;
  void *udata;
  int level;
} Callback;

typedef struct {
    rk_irfpa_log_ctx_t ctx;

    //log_LockFn lock;
    //int level;
    //bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} log_priv_t;

static rk_irfpa_log_ctx_t *default_ctx;

rk_irfpa_log_ctx_t *rk_irfpa_log_get_default(void)
{
    return default_ctx;
}

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};


static void stdout_callback(log_priv_t *priv, log_Event *ev) {
  char buf[16];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
  if (priv->ctx.cfg.stdio_color) {
      fprintf(
        (FILE *)ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
        buf, level_colors[ev->level], level_strings[ev->level],
        ev->file, ev->line);
  } else {
      fprintf(
        (FILE *)ev->udata, "%s %-5s %s:%d: ",
        buf, level_strings[ev->level], ev->file, ev->line);
  }
  vfprintf((FILE *)ev->udata, ev->fmt, ev->ap);
  fprintf((FILE *)ev->udata, "\n");
  fflush((FILE *)ev->udata);
}


static void file_callback(log_Event *ev) {
  char buf[64];
  buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
  fprintf(
    (FILE *)ev->udata, "%s %-5s %s:%d: ",
    buf, level_strings[ev->level], ev->file, ev->line);
  vfprintf((FILE *)ev->udata, ev->fmt, ev->ap);
  fprintf((FILE *)ev->udata, "\n");
  fflush((FILE *)ev->udata);
}


/*
static void lock(void)   {
  if (L.lock) { L.lock(true, L.udata); }
}


static void unlock(void) {
  if (L.lock) { L.lock(false, L.udata); }
}
*/


const char* log_level_string(int level) {
  return level_strings[level];
}


/*
static void log_set_lock(log_LockFn fn, void *udata) {
  L.lock = fn;
  L.udata = udata;
}


static void log_set_level(int level) {
  L.level = level;
}


static void log_set_quiet(bool enable) {
  L.quiet = enable;
}


static int log_add_callback(log_LogFn fn, void *udata, int level) {
  for (int i = 0; i < MAX_CALLBACKS; i++) {
    if (!L.callbacks[i].fn) {
      L.callbacks[i] = (Callback) { fn, udata, level };
      return 0;
    }
  }
  return -1;
}


static int log_add_fp(FILE *fp, int level) {
  return log_add_callback(file_callback, fp, level);
}
*/


static void init_event(log_Event *ev, void *udata) {
  if (!ev->time) {
    time_t t = time(NULL);
    ev->time = localtime(&t);
  }
  ev->udata = udata;
}


void rk_irfpa_log(rk_irfpa_log_ctx_t * ctx, int level, const char *file, int line, const char *fmt, ...)
{
    log_priv_t *priv = (log_priv_t *) ctx;
    if (!priv)
        return;

    log_Event ev = {
        .fmt   = fmt,
        .file  = file,
        .line  = line,
        .level = level,
    };

    //lock();

    if (ctx->cfg.use_stdio && level >= ctx->cfg.level) {
        init_event(&ev, stderr);
        va_start(ev.ap, fmt);
        stdout_callback(priv, &ev);
        va_end(ev.ap);
    }

    /*
    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    //unlock();
    */
}

rk_irfpa_log_ctx_t* rk_irfpa_log_create_ctx(void)
{
    log_priv_t *priv = new log_priv_t;
    rk_irfpa_log_ctx_t *ctx = &priv->ctx;

    default_ctx = ctx;
    return ctx;
}


void rk_irfpa_log_deinit(rk_irfpa_log_ctx_t *ctx)
{
    log_priv_t *priv = (log_priv_t *)ctx;
    delete priv;
}

bool rk_irfpa_log_init(rk_irfpa_log_ctx_t *ctx)
{
    rk_irfpa_log_info(ctx, "IRPFA-ISP Rev: %s", RK_IRFPA_GIT_REV);
    return true;
}
