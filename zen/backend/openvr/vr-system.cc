#include "vr-system.h"

#include <fcntl.h>
#include <openvr/openvr.h>
#include <unistd.h>

namespace zen {

int
CompileShader(
    const char *vertex_shader_source, const char *fragment_shader_source)
{
  GLuint id = glCreateProgram();

  // compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    zn_error("opengl shader compiler: failed to compile vertex shader\n");
    glDeleteProgram(id);
    glDeleteShader(vertex_shader);
    return -1;
  }
  glAttachShader(id, vertex_shader);
  glDeleteShader(vertex_shader);

  // compile fragment shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);

  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    zn_error("opengl shader compiler: failed to compile fragment shader\n");
    glDeleteProgram(id);
    glDeleteShader(fragment_shader);
    return -1;
  }
  glAttachShader(id, fragment_shader);
  glDeleteShader(vertex_shader);

  glLinkProgram(id);

  GLint program_success = GL_FALSE;
  glGetProgramiv(id, GL_LINK_STATUS, &program_success);
  if (program_success != GL_TRUE) {
    zn_error("opengl shader compiler: failed to link program\n");
    glDeleteProgram(id);
    return -1;
  }

  glUseProgram(id);
  glUseProgram(0);

  return id;
}

void
Render()
{
  int32_t shader_id = CompileShader(
      // vertex shader
      "#version 410 core\n"
      "layout(location = 0) in vec4 position;\n"
      "void main()\n"
      "{\n"
      "	gl_Position = position;\n"
      "}\n",

      // fragment shader
      "#version 410 core\n"
      "out vec4 outputColor;\n"
      "void main()\n"
      "{\n"
      "  outputColor = vec4(0, 1, 0, 1);\n"
      "}\n");

  float vertices[2][3] = {
      {-0.5, -0.5, 0.5},
      {+0.5, +0.5, 0.5},
  };

  uint32_t vao;
  uint32_t vbo;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glUseProgram(shader_id);

  glDrawArrays(GL_LINES, 0, 2);

  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(shader_id);
}

bool
VrSystem::Init(struct wl_event_loop *loop)
{
  return openvr_.Init(loop);
}

bool
VrSystem::Connect()
{
  uint32_t display_width, display_height;

  if (openvr_.Connect() == false) {
    goto err;
  }

  vr::VRSystem()->GetRecommendedRenderTargetSize(
      &display_width, &display_height);
  // display_width = 300;
  // display_height = 300;

  left_eye_ = std::make_unique<Camera>();
  if (left_eye_->Init(display_width, display_height) == false) {
    goto err_left_eye;
  }

  right_eye_ = std::make_unique<Camera>();
  if (right_eye_->Init(display_width, display_height) == false) {
    goto err_right_eye;
  }

  callbacks.NewCamera(left_eye_->zn_camera());
  callbacks.NewCamera(right_eye_->zn_camera());

  return true;

err_right_eye:
  right_eye_.reset();

err_left_eye:
  left_eye_.reset();
  openvr_.Disconnect();

err:
  return false;
}

void
VrSystem::Disconnect()
{
  openvr_.Disconnect();
  is_repaint_loop_running_ = false;

  left_eye_.reset();
  right_eye_.reset();

  if (callbacks.Disconnected) callbacks.Disconnected();
}

void
VrSystem::StartRepaintLoop()
{
  auto callback =
      std::bind(&VrSystem::HandlePollResult, this, std::placeholders::_1);

  if (is_repaint_loop_running_) return;

  openvr_.RequestPoll(callback);

  is_repaint_loop_running_ = true;
}

void
VrSystem::HandlePollResult(OpenVr::PollResult *result)
{
  auto callback =
      std::bind(&VrSystem::HandlePollResult, this, std::placeholders::_1);

  if (is_repaint_loop_running_ == false) return;

  if (result->should_quit) {
    Disconnect();
    return;
  }

  {
    glBindFramebuffer(GL_FRAMEBUFFER, left_eye_->framebuffer());
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, left_eye_->width(), left_eye_->height());

    Render();

    glBindFramebuffer(GL_FRAMEBUFFER, right_eye_->framebuffer());
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, right_eye_->width(), right_eye_->height());

    Render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    left_eye_->BlitFramebuffer();
    right_eye_->BlitFramebuffer();

    glFinish();

    openvr_.Submit(left_eye_->resolve_texture(), right_eye_->resolve_texture());
  }

  // {
  //   uint32_t width = right_eye_->width();
  //   uint32_t height = right_eye_->height();

  //   auto d =
  //       static_cast<GLubyte *>(malloc(sizeof(GLubyte) * width * height * 4));
  //   for (uint y = 0; y < height; y++) {
  //     for (uint x = 0; x < width; x++) {
  //       d[(y * width + x) * 4 + 0] = 0xff;
  //       d[(y * width + x) * 4 + 1] = 0x00;
  //       d[(y * width + x) * 4 + 2] = 0x00;
  //       d[(y * width + x) * 4 + 3] = 0xff;
  //     }
  //   }

  //   GLuint rt, lt;
  //   glGenTextures(1, &rt);
  //   glBindTexture(GL_TEXTURE_2D, rt);
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
  //       GL_UNSIGNED_BYTE, d);

  //   glGenTextures(1, &lt);
  //   glBindTexture(GL_TEXTURE_2D, lt);
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
  //       GL_UNSIGNED_BYTE, d);

  //   glBindTexture(GL_TEXTURE_2D, 0);

  //   glFinish();

  //   openvr_.Submit(lt, rt);

  //   glFinish();

  //   glDeleteTextures(1, &rt);
  //   glDeleteTextures(1, &lt);
  //   free(d);
  // }

  openvr_.RequestPoll(callback);
}

}  // namespace zen
