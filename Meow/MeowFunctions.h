#ifndef MEOW_MEOWFUNCTIONS_H
#define MEOW_MEOWFUNCTIONS_H

#include <Protocol/DevicePath.h>

#include <Library/UefiLib.h>

EFI_STATUS
DrawLines (
  IN   CHAR16                  *String,
  IN   EFI_FONT_DISPLAY_INFO   *FontDisplayInfo,
  OUT  EFI_IMAGE_OUTPUT        *ImageOutput,
  IN   UINTN                   PosX,
  IN   UINTN                   PosY
  );

VOID
Log (
  IN  CHAR16   *Text
  );

UINT32
SprintUint (
  IN   UINT32   Decimal,
  OUT  CHAR16   *Buffer,
  IN   UINT32   BufferSize,
  IN   UINT32   Offset
  );

UINT32
Max (
  IN  UINT32  One,
  IN  UINT32  Another
  );

CHAR16 *
MeowPathToText (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Path
  );

#endif // MEOW_MEOWFUNCTIONS_H
