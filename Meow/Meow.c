#include <Uefi.h>

#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/HiiFont.h>
#include <Protocol/DevicePathToText.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "Activity.h"
#include "MainActivity.h"
#include "MeowFunctions.h"

///////////////

STATIC ACTIVITY                          *gTopActivity       = NULL;

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL      *gGraphProtocol     = NULL;

STATIC EFI_HII_FONT_PROTOCOL             *gFontProtocol      = NULL;

STATIC EFI_DEVICE_PATH_TO_TEXT_PROTOCOL  *gPathConvProtocol  = NULL;

///////////////

UINT32
SprintUint (
  IN   UINT32   Decimal,
  OUT  CHAR16   *Buffer,
  IN   UINT32   BufferSize,
  IN   UINT32   Offset
  )
{
  CHAR16  Tmp[10];
  UINT32  MaxVal;
  UINT32  i;

  //

  MaxVal = 0;

  if (Buffer == NULL) {
    return MaxVal;
  }

  for (i = 0; 0 != Decimal; i++) {

    UINT32  Digit;

    //

    Digit = Decimal % 10;

    Tmp[i] = (CHAR16)('0' + Digit);
    Decimal = Decimal / 10;

    // Trust the compiler it would optimize :)

    if (Digit != 0) {
      MaxVal = i + 1;
    }
  }

  if (MaxVal >= (BufferSize - Offset - 1)) {
    MaxVal = BufferSize - Offset - 1;
  }

  for (i = 0; i < MaxVal; i++) {
    Buffer[Offset + i] = Tmp[MaxVal - i - 1];
  }

  if (MaxVal == 0) {
    Buffer[Offset] = L'\0';
    return 1;
  }

  return MaxVal;
}

UINT32
Max (
  IN  UINT32  One,
  IN  UINT32  Another
  )
{
  return (One > Another) ? One : Another;
}

VOID
Log (
  IN  CHAR16   *Text
  )
{
  gST->ConOut->OutputString (gST->ConOut, Text);
}

EFI_STATUS
DrawLines (
  IN   CHAR16                  *String,
  IN   EFI_FONT_DISPLAY_INFO   *FontDisplayInfo,
  OUT  EFI_IMAGE_OUTPUT        *ImageOutput,
  IN   UINTN                   PosX,
  IN   UINTN                   PosY
  )
{
  return gFontProtocol->StringToImage (
                          gFontProtocol,
                          EFI_HII_OUT_FLAG_TRANSPARENT,
                          String,
                          FontDisplayInfo,
                          &ImageOutput,
                          PosX,
                          PosY,
                          NULL,
                          NULL,
                          NULL
                          );
}

CHAR16 *
MeowPathToText (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Path
  )
{
  if (gPathConvProtocol != NULL) {
    return gPathConvProtocol->ConvertDevicePathToText (Path, TRUE, TRUE);
  }

  return NULL;
}

VOID
ClearScreen (
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Color
  )
{
  if (gGraphProtocol != NULL) {

    gGraphProtocol->Blt (
                      gGraphProtocol,
                      &Color,
                      EfiBltVideoFill,
                      0,
                      0,
                      0,
                      0,
                      gGraphProtocol->Mode->Info->HorizontalResolution,
                      gGraphProtocol->Mode->Info->VerticalResolution,
                      0
                      );
  }
}

EFI_STATUS
Loop (
  VOID
  )
{
  EFI_STATUS  Status;
  UINTN       IsRunning;

  while (TRUE) {

    if (gTopActivity == NULL) {
      Log (L"gTopActivity == NULL\n");
      return EFI_SUCCESS;
    }

    IsRunning = MainActivityOnEvent (gTopActivity);

    // ESC pressed

    if (IsRunning != 0) {
      break;
    }

    Status = ActivityRender (gTopActivity, gGraphProtocol);

    if (EFI_ERROR (Status)) {
      Log (L"Cannot render Activity.\n");
      return Status;
    }
  }

  return EFI_SUCCESS;
}

///////////////

EFI_STATUS
EFIAPI
UefiMain (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  UINT32      Width;
  UINT32      Height;
  //CHAR16      Line[32];
  //UINT32      Tmp;

  //

  // Graphic access.

  Status = gBS->LocateProtocol (
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  (VOID **)&gGraphProtocol
                  );

  if (EFI_ERROR (Status)) {
    // Well, if graphic's not available, where comes the shell?
    Log (L"Cannot locate graphics output protocol.\n");
    goto Done;
  }

  // Assuming screen size won't change.
  // After all, this is just a boot menu.

  Width  = gGraphProtocol->Mode->Info->HorizontalResolution;
  Height = gGraphProtocol->Mode->Info->VerticalResolution;

  /*
  Log (L"Got resolution ");

  Tmp = SprintUint (Width, Line, 0);
  Line[Tmp] = 'x';
  Tmp++ ;
  Tmp += SprintUint (Height, Line, Tmp);
  Log (Line);
  Log (L".\n");
  */

  // Locate font protocol, we use it to draw strings.

  Status = gBS->LocateProtocol (&gEfiHiiFontProtocolGuid, NULL, (VOID **)&gFontProtocol);

  if (EFI_ERROR (Status)) {
    Log (L"Cannot locate font protocol.\n");
    goto Done;
  }

  // Create the Main Activity.
  Status = NewMainActivity (Width, Height, &gTopActivity);

  if (EFI_ERROR (Status)) {
    Log (L"Cannot allocate MainActivity.\n");
    goto Done;
  }

  Status = gBS->LocateProtocol (
                  &gEfiDevicePathToTextProtocolGuid,
                  NULL,
                  (VOID **)&gPathConvProtocol
                  );

  if (EFI_ERROR (Status)) {
    Log (L"Cannot locate device path to text protocol.");
    gPathConvProtocol = NULL;
    goto Done;
  }

  // Start the Activity.
  MainActivityOnStart (gTopActivity);

  // Enter event loop.

  //Log (L"-> loop.\n");
  Status = Loop ();
  //Log (L"<- loop.\n");

  // Log previous exception from event loop.

  if (EFI_ERROR (Status)) {
    Log (L"Cannot draw frame.\n");
  }

  FreeActivity (gTopActivity);

  Done:

  #if defined(MEOW_MODE) && (MEOW_MODE == APPLICATION)
    Status = EFI_SUCCESS;
  #endif

  // Bye.
  return Status;
}
