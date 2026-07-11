#ifndef DOOR_H
#define DOOR_H

#include <stdint.h>

#include "map.h"

typedef enum {
    DOOR_NORMAL,
    DOOR_LOCK_GOLD,
    DOOR_LOCK_SILVER,
    DOOR_LOCK_3,
    DOOR_LOCK_4,
    DOOR_ELEVATOR
} DoorKind;

typedef enum {
    DOOR_CLOSED,
    DOOR_OPENING,
    DOOR_OPEN,
    DOOR_CLOSING
} DoorAction;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t vertical;
    DoorKind kind;
    DoorAction action;
    double position;
    double open_seconds;
} Door;

int door_from_tile(uint16_t tile, int *vertical, DoorKind *kind);
unsigned int door_required_key(DoorKind kind);
int door_blocks_circle(const Door *door, double position,
                       double x, double y, double radius);

#endif
