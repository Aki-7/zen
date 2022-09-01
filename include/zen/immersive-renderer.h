#ifndef ZEN_IMMERSIVE_RENDERER_H
#define ZEN_IMMERSIVE_RENDERER_H

#include "zen/scene/camera.h"
#include "zen/scene/scene.h"

void zn_immersive_renderer_render(
    struct zn_scene *scene, struct zn_camera *camera);

#endif  //  ZEN_IMMERSIVE_RENDERER_H
