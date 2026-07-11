#include "door.h"

#include <math.h>

enum { DOOR_THICKNESS_PERCENT = 4 };

static double clamp(double value, double minimum, double maximum)
{
    if (value < minimum)
        return minimum;
    if (value > maximum)
        return maximum;
    return value;
}

int door_from_tile(uint16_t tile, int *vertical, DoorKind *kind)
{
    if (tile < 90 || tile > 101)
        return 0;
    *vertical = !(tile & 1u);
    *kind = (DoorKind)((tile - 90) / 2);
    return 1;
}

unsigned int door_required_key(DoorKind kind)
{
    if (kind >= DOOR_LOCK_GOLD && kind <= DOOR_LOCK_4)
        return 1u << (kind - DOOR_LOCK_GOLD);
    return 0;
}

int door_blocks_circle(const Door *door, double position,
                       double x, double y, double radius)
{
    if (position >= 1.0)
        return 0;
    const double half = DOOR_THICKNESS_PERCENT / 200.0;
    double left, right, top, bottom;
    if (door->vertical) {
        left = door->x + 0.5 - half;
        right = door->x + 0.5 + half;
        top = door->y + position;
        bottom = door->y + 1.0;
    } else {
        left = door->x + position;
        right = door->x + 1.0;
        top = door->y + 0.5 - half;
        bottom = door->y + 0.5 + half;
    }
    const double nearest_x = clamp(x, left, right);
    const double nearest_y = clamp(y, top, bottom);
    const double dx = x - nearest_x;
    const double dy = y - nearest_y;
    return dx * dx + dy * dy < radius * radius;
}
