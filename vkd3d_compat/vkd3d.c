#include <windows.h>

#include <vkd3d_shader.h>

#include "private/vkd3d_common.h"

enum vkd3d_dbg_level vkd3d_dbg_get_level(const char *env_name)
{
  return VKD3D_DBG_LEVEL_TRACE;
}

void vkd3d_dbg_printf(const char *vkd3d_dbg_env_name, enum vkd3d_dbg_level level,
                      const char *function, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

void vkd3d_dbg_set_log_callback(PFN_vkd3d_log callback)
{
  STUBBED();
}

const char *vkd3d_dbg_sprintf(const char *fmt, ...)
{
  static char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  return buffer;
}

const char *debugstr_a(const char *str)
{
  return str ? str : "(null)";
}

const char *debugstr_an(const char *str, size_t n)
{
  // TODO: I suspect this is what it's meant for,
  //  but it will have to run first, so I can tell for sure.
  static char buffer[1024];
  size_t len = strnlen((const char *)str, n);
  if (len >= sizeof(buffer))
    len = sizeof(buffer) - 1;
  memcpy(buffer, str, len);
  buffer[len] = '\0';
  return buffer;
}

uint64_t vkd3d_parse_debug_options(const char *string,
                                   const struct vkd3d_debug_option *options, unsigned int option_count)
{
  STUBBED();
  return 0;
}