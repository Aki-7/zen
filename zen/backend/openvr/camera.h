#ifndef ZEN_OPENVR_BACKEND_CAMERA_H
#define ZEN_OPENVR_BACKEND_CAMERA_H

#include <GL/glew.h>
#include <wlr/render/egl.h>

#include "zen-common.h"
#include "zen/scene/camera.h"

namespace zen {

class Camera
{
 public:
  Camera() = default;
  ~Camera() = default;
  DISABLE_MOVE_AND_COPY(Camera)

  bool Init(uint32_t width, uint32_t height);  // return NULL if failed
  void BlitFramebuffer();

  inline uint32_t width();
  inline uint32_t height();
  inline GLuint framebuffer();
  inline GLuint resolve_texture();
  inline struct zn_camera* zn_camera();

 private:
  GLuint framebuffer_id_;
  GLuint color_buffer_id_;
  GLuint depth_buffer_id_;
  GLuint resolve_framebuffer_id_;
  GLuint resolve_texture_id_;
  struct zn_camera* zn_camera_;
};

inline uint32_t
Camera::width()
{
  return zn_camera_->viewport.width;
}

inline uint32_t
Camera::height()
{
  return zn_camera_->viewport.height;
}

inline GLuint
Camera::framebuffer()
{
  return framebuffer_id_;
}

inline GLuint
Camera::resolve_texture()
{
  return resolve_texture_id_;
}

inline zn_camera*
Camera::zn_camera()
{
  return zn_camera_;
}

}  // namespace zen

#endif  //  ZEN_OPENVR_BACKEND_CAMERA_H
