#ifndef MEOW_FUNCTIONS_H
#define MEOW_FUNCTIONS_H

#include "Meow.h"

#if MEOW_DEBUG == 1
/**
  Console Log.
**/
VOID
MeowConsoleLog (
  IN  CHAR16   *Text
  );
#endif

/**
  Draw Lines.
**/
EFI_STATUS
MeowDrawLines (
  IN   CHAR16                  *String,
  IN   EFI_FONT_DISPLAY_INFO   *FontDisplayInfo,
  OUT  EFI_IMAGE_OUTPUT        *ImageOutput,
  IN   UINTN                   PosX,
  IN   UINTN                   PosY
  );

/**
  Clear Screen.
**/
VOID
MeowClearScreen (
  IN  MEOW_ACTIVITY                   *Activity,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Color
  );

/**
  DevicePath to text.
**/
CHAR16 *
MeowPathToText (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Path
  );

/**
  Set Max Res.
**/
VOID
SetMaxRes (
  OUT  UINT32  *Width,
  OUT  UINT32  *Height,
  OUT  UINT32  *BestMode
  );

#endif // MEOW_FUNCTIONS_H
