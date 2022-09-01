#include "camera.h"

#include "gl-helper.h"

namespace zen {

bool
Camera::Init(uint32_t width, uint32_t height)
{
  GLenum status;

  zn_camera_ = static_cast<struct zn_camera*>(zalloc(sizeof *zn_camera_));
  zn_camera_->viewport.width = width;
  zn_camera_->viewport.height = height;
  zn_camera_->viewport.x = 0;
  zn_camera_->viewport.y = 0;

  GlHelperPushDebug();

  glGenFramebuffers(1, &framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);

  glGenRenderbuffers(1, &depth_buffer_id_);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_id_);
  glRenderbufferStorageMultisample(
      GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, width, height);
  glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_id_);

  glGenTextures(1, &color_buffer_id_);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color_buffer_id_);
  glTexImage2DMultisample(
      GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_2D_MULTISAMPLE, color_buffer_id_, 0);

  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    zn_error("Failed to create camera framebuffer");
    goto err_framebuffer;
  }

  glGenTextures(1, &resolve_texture_id_);
  glBindTexture(GL_TEXTURE_2D, resolve_texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
      GL_UNSIGNED_BYTE, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenFramebuffers(1, &resolve_framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, resolve_framebuffer_id_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      resolve_texture_id_, 0);

  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    zn_error("Failed to create camera resolve framebuffer");
    goto err_resolve_framebuffer;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  GlHelperPopDebug();

  zn_camera_->framebuffer_id = framebuffer_id_;

  return true;

err_resolve_framebuffer:
  glDeleteTextures(1, &resolve_texture_id_);
  glDeleteFramebuffers(1, &resolve_framebuffer_id_);

err_framebuffer:
  glDeleteTextures(1, &color_buffer_id_);
  glDeleteRenderbuffers(1, &depth_buffer_id_);
  glDeleteFramebuffers(1, &framebuffer_id_);

  GlHelperPopDebug();

  return false;
}

void
Camera::BlitFramebuffer()
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_id_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_framebuffer_id_);
  glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(),
      GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

}  // namespace zen
