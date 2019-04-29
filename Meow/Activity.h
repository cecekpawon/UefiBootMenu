#ifndef __MEOW_ACTIVITY_H__
#define __MEOW_ACTIVITY_H__

#include <Uefi.h>

#include <Protocol/GraphicsOutput.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

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

VOID
FreeActivity (
  VOID
  );

// Struct defines.

struct RECT {
  UINT32  PosX;
  UINT32  PosY;
  UINT32  Width;
  UINT32  Height;
};

struct ACTIVITY {
  ACTIVITY                        *Parent;
  //BOOLEAN                         IsNonFullScreen;
  UINT32                          Width;
  UINT32                          Height;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Buffer;
  UINT8                           CountInvalid;
  RECT                            Invalids[64];
};

#endif
