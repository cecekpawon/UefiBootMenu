#ifndef MEOW_ACTIVITY_H
#define MEOW_ACTIVITY_H

#include <Uefi.h>

#include <Protocol/GraphicsOutput.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define BLACK_BG    { 0 }
#define COLOR_BG    { 0xDD, 0x10, 0x10, 0x00 }
#define COLOR_FG    { 0xFF, 0x79, 0x29, 0x00 }

// Typedefs, struct declares.

typedef struct RECT       RECT;
typedef struct ACTIVITY   ACTIVITY;

// Function declares.

EFI_STATUS
ActivityInitialize (
  IN OUT  ACTIVITY  *Activity,
  IN      UINT32    Width,
  IN      UINT32    Height
  );

VOID
ActivityInvalidate (
  IN OUT  ACTIVITY  *Activity,
  IN      RECT      Rect
  );


EFI_STATUS
ActivityRender (
  IN OUT  ACTIVITY                      *Activity,
  IN      EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphProtocol
  );

VOID
ClearScreen (
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Color
);

// Struct defines.

struct RECT {
  UINT32  PosX;
  UINT32  PosY;
  UINT32  Width;
  UINT32  Height;
};

struct ACTIVITY {
  //ACTIVITY                        *Parent;
  //BOOLEAN                         IsNonFullScreen;
  UINT32                          Width;
  UINT32                          Height;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Buffer;
  UINT8                           CountInvalid;
  RECT                            Invalids[64];
};

#endif // MEOW_ACTIVITY_H
