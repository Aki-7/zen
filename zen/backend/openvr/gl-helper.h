#ifndef ZEN_OPENVR_BACKEND_GL_HELPER_H
#define ZEN_OPENVR_BACKEND_GL_HELPER_H

#include <GL/glew.h>
#include <stdio.h>

#include <string>

namespace zen {

static inline void
GlHelperPushDebug_(const char *file, const char *func)
{
  std::string str = std::string(file) + ":" + func;
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, str.c_str());
}

#define GlHelperPushDebug() GlHelperPushDebug_(__FILE__, __func__)

static inline void
GlHelperPopDebug()
{
  glPopDebugGroup();
}

}  // namespace zen

#endif  //  ZEN_OPENVR_BACKEND_GL_HELPER_H
