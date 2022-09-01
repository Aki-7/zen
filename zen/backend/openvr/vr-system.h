#ifndef ZEN_OPENVR_BACKEND_VR_SYSTEM_H
#define ZEN_OPENVR_BACKEND_VR_SYSTEM_H

#include <wayland-server-core.h>

#include <functional>
#include <memory>

#include "camera.h"
#include "openvr.h"
#include "zen-common.h"
#include "zen/scene/camera.h"

namespace zen {
class VrSystem
{
 public:
  VrSystem() = default;
  ~VrSystem() = default;
  DISABLE_MOVE_AND_COPY(VrSystem)

  bool Init(struct wl_event_loop *loop);
  bool Connect();
  void Disconnect();
  void StartRepaintLoop();

  struct {
    std::function<void()> Disconnected;
    std::function<void(struct zn_camera *camera)> NewCamera;
  } callbacks;

 private:
  void HandlePollResult(OpenVr::PollResult *result);

  OpenVr openvr_;
  bool is_repaint_loop_running_ = false;

  std::unique_ptr<Camera> left_eye_;
  std::unique_ptr<Camera> right_eye_;
};

}  // namespace zen

#endif  //  ZEN_OPENVR_BACKEND_VR_SYSTEM_H
