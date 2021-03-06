#ifndef MEOW_ACTIVITY_H
#define MEOW_ACTIVITY_H

#include "Meow.h"

#define BLACK_BG        { 0x00, 0x00, 0x00, 0x00 }
#define COLOR_BG        { 0xDD, 0x10, 0x10, 0x00 }
#define COLOR_FG        { 0xFF, 0x79, 0x29, 0x00 }

#define MAX_INVALIDS    (64)

// Typedefs, struct declares.

typedef struct _RECT             RECT;
typedef struct _MEOW_ACTIVITY    MEOW_ACTIVITY;

// Struct defines.

struct _RECT {
  UINT32  PosX;
  UINT32  PosY;
  UINT32  Width;
  UINT32  Height;
};

struct _MEOW_ACTIVITY {
  UINT32                          Width;
  UINT32                          Height;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Buffer;
  UINT8                           CountInvalid;
  RECT                            Invalids[MAX_INVALIDS];
};

typedef enum {
  EventReturnEnter,
  EventReturnError,
  EventReturnEscape,
  EventReturnNone,
  EventReturnRefresh
} MEOW_EVENT_RETURN;

// Function declares.

/**
  Init Activity.
**/
EFI_STATUS
ActivityInitialize (
  IN OUT  MEOW_ACTIVITY   *This,
  IN      UINT32          Width,
  IN      UINT32          Height
  );

/**
  Invalidate Activity.
**/
VOID
ActivityInvalidate (
  IN OUT  MEOW_ACTIVITY   *This,
  IN      RECT            Rect
  );

/**
  Render Activity.
**/
EFI_STATUS
ActivityRender (
  IN OUT  MEOW_ACTIVITY       *This,
  IN      MEOW_EVENT_RETURN   Ret
  );

#endif // MEOW_ACTIVITY_H
