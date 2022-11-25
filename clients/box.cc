#include <GLES3/gl32.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <zigen-protocol.h>

#include <cstring>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <vector>

#include "application.h"
#include "bounded.h"
#include "box.vert.h"
#include "buffer.h"
#include "color.fragment.h"
#include "fd.h"
#include "gl-base-technique.h"
#include "gl-buffer.h"
#include "gl-program.h"
#include "gl-shader.h"
#include "gl-texture.h"
#include "gl-vertex-array.h"
#include "rendering-unit.h"
#include "shm-pool.h"
#include "virtual-object.h"

using namespace zen::client;

typedef struct {
  float x, y, z;
} Vertex;

/** Draw the rotating cube so that it always fits within the boundary */
class Box final : public Bounded
{
 public:
  Box() = delete;
  Box(Application* app) : Bounded(app), app_(app) {}

  void SetUniformVariables()
  {
    float rad_2 = M_PI * animation_seed_ * 4;  // rad / 2
    float cos = cosf(rad_2);
    float sin = sinf(rad_2);
    glm::vec4 quaternion = {0, sin, 0, cos};
    float min = glm::min(glm::min(half_size_.x, half_size_.y), half_size_.z);
    glm::vec3 size(min / sqrt(3));
    technique_->Uniform(0, "half_size", size);
    technique_->Uniform(0, "quaternion", quaternion);
  }

  void Frame(uint32_t time) override
  {
    animation_seed_ = (float)(time % 12000) / 12000.0f;

    SetUniformVariables();
    NextFrame();
    Commit();
  }

  void Configure(glm::vec3 half_size, uint32_t serial) override
  {
    half_size_ = half_size;

    SetUniformVariables();

    AckConfigure(serial);

    if (!committed_) {
      NextFrame();
      Commit();
      committed_ = true;
    }
  }

  bool Init()
  {
    if (!Bounded::Init(glm::vec3(0.25, 0.25, 0.50))) return false;

    unit_ = CreateRenderingUnit(app_, this);
    if (!unit_) return false;

    technique_ = CreateGlBaseTechnique(app_, unit_.get());
    if (!technique_) return false;

    // clang-format off
    vertices_ = {
      {-1, -1, +1}, {+1, -1, +1},
      {+1, -1, +1}, {+1, -1, -1},
      {+1, -1, -1}, {-1, -1, -1},
      {-1, -1, -1}, {-1, -1, +1},
      {-1, +1, +1}, {+1, +1, +1},
      {+1, +1, +1}, {+1, +1, -1},
      {+1, +1, -1}, {-1, +1, -1},
      {-1, +1, -1}, {-1, +1, +1},
      {-1, -1, +1}, {-1, +1, +1},
      {+1, -1, +1}, {+1, +1, +1},
      {+1, -1, -1}, {+1, +1, -1},
      {-1, -1, -1}, {-1, +1, -1},
    };
    // clang-format on

    ssize_t vertex_buffer_size =
        sizeof(decltype(vertices_)::value_type) * vertices_.size();
    vertex_buffer_fd_ = create_anonymous_file(vertex_buffer_size);
    if (vertex_buffer_fd_ < 0) return false;

    {
      auto v = mmap(nullptr, vertex_buffer_size, PROT_WRITE, MAP_SHARED,
          vertex_buffer_fd_, 0);
      std::memcpy(v, vertices_.data(), vertex_buffer_size);
      munmap(v, vertex_buffer_size);
    }

    pool_ = CreateShmPool(app_, vertex_buffer_fd_, vertex_buffer_size);
    if (!pool_) return false;

    vertex_buffer_ = CreateBuffer(pool_.get(), 0, vertex_buffer_size);
    if (!vertex_buffer_) return false;

    gl_vertex_buffer_ = CreateGlBuffer(app_);
    if (!gl_vertex_buffer_) return false;

    texture_ = CreateGlTexture(app_);
    if (!texture_) return false;

    vertex_array_ = CreateGlVertexArray(app_);
    if (!vertex_array_) return false;

    vertex_shader_ =
        CreateGlShader(app_, GL_VERTEX_SHADER, box_vertex_shader_source);
    if (!vertex_shader_) return false;

    fragment_shader_ =
        CreateGlShader(app_, GL_FRAGMENT_SHADER, color_fragment_shader_source);
    if (!fragment_shader_) return false;

    program_ = CreateGlProgram(app_);
    if (!program_) return false;

    program_->AttachShader(vertex_shader_.get());
    program_->AttachShader(fragment_shader_.get());
    program_->Link();

    gl_vertex_buffer_->Data(
        GL_ARRAY_BUFFER, vertex_buffer_.get(), GL_STATIC_DRAW);

    vertex_array_->Enable(0);
    vertex_array_->VertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 0, 0, gl_vertex_buffer_.get());

    technique_->Bind(vertex_array_.get());
    technique_->Bind(program_.get());
    technique_->Bind(0, "", texture_.get(), GL_TEXTURE_2D);

    technique_->DrawArrays(GL_LINES, 0, vertices_.size());

    return true;
  }

 private:
  Application* app_;
  std::unique_ptr<RenderingUnit> unit_;
  std::unique_ptr<GlBaseTechnique> technique_;
  std::vector<Vertex> vertices_;
  int vertex_buffer_fd_;
  std::unique_ptr<ShmPool> pool_;
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<GlBuffer> gl_vertex_buffer_;
  std::unique_ptr<GlVertexArray> vertex_array_;
  std::unique_ptr<GlShader> vertex_shader_;
  std::unique_ptr<GlShader> fragment_shader_;
  std::unique_ptr<GlProgram> program_;
  std::unique_ptr<GlTexture> texture_;

  float animation_seed_;  // 0 to 1
  glm::vec3 half_size_;
  bool committed_ = false;
};

int
main(void)
{
  Application app;

  if (!app.Init()) return EXIT_FAILURE;
  if (!app.Connect()) return EXIT_FAILURE;

  Box box(&app);
  if (!box.Init()) return EXIT_FAILURE;

  return app.Run();
}
