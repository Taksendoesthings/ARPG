#ifndef STATE_ISOMETRIC_H
#define STATE_ISOMETRIC_H

#include <gbdk/platform.h>

void isometric_init(void) BANKED;
void isometric_update(void) BANKED;

extern UBYTE isometric_grid;

#endif
