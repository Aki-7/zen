#ifndef ZEN_CAMERA_H
#define ZEN_CAMERA_H

#include <GL/glew.h>
#include <cglm/cglm.h>

struct zn_camera {
  GLuint framebuffer_id;
  mat4 view_matrix;
  mat4 projection_matrix;

  struct {
    GLint x, y;
    GLsizei width, height;
  } viewport;
};

struct zn_camera* zn_camera_create(GLuint framebuffer_id);

void zn_camera_destroy(struct zn_camera* self);

#endif  //  ZEN_CAMERA_H
