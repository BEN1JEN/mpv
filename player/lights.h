#ifndef MPLAYER_MP_LIGHTS_H
#define MPLAYER_MP_LIGHTS_H

#include "video/img_format.h"

void update_lights(int width, int height, enum mp_imgfmt imgfmt, uint8_t *yPlane, uint8_t *uPlane, uint8_t *vPlane);

#endif
