#include "Meow.h"

#if MEOW_DEBUG == 1
/**
  Console Log.
**/
VOID
MeowConsoleLog (
  IN  CHAR16   *Text
  )
{
  gST->ConOut->OutputString (gST->ConOut, Text);
}
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

/**
  Clear Screen.
**/
VOID
MeowClearScreen (
  IN  MEOW_ACTIVITY                   *Activity,
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
                      Activity->Width,
                      Activity->Height,
                      0
                      );
  }
}

/**
  DevicePath to text.
**/
CHAR16 *
MeowPathToText (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Path
  )
{
  if (Path != NULL) {

    #if MEOW_DEVPATH_LIB == 1
      CHAR16                *FileName;
      FILEPATH_DEVICE_PATH  *ThisFilePath;

      //

      // Get FileName only.

      FileName      = NULL;
      ThisFilePath  = (FILEPATH_DEVICE_PATH *)Path;

      while (!IsDevicePathEnd (ThisFilePath)) {
        if ((DevicePathType (&ThisFilePath->Header) == MEDIA_DEVICE_PATH)
          && (DevicePathSubType (&ThisFilePath->Header) == MEDIA_FILEPATH_DP)
          )
        {
          FileName = &(ThisFilePath->PathName)[0];
          break;
        }

        ThisFilePath = (FILEPATH_DEVICE_PATH *)NextDevicePathNode (ThisFilePath);
      }

      if (FileName != NULL) {

        return CatSPrint (NULL, L"%s", FileName);
      }

      return ConvertDevicePathToText (Path, TRUE, TRUE);
    #else
      if (gPathConvProtocol != NULL) {

        return gPathConvProtocol->ConvertDevicePathToText (Path, TRUE, TRUE);
      }
    #endif
  }

  return NULL;
}

/**
  Set Max Res.
**/
VOID
SetMaxRes (
  OUT  UINT32  *Width,
  OUT  UINT32  *Height,
  OUT  UINT32  *BestMode
  )
{
  if ((Width!= NULL) && (Height!= NULL) && (BestMode!= NULL)) {

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    *Info;
    UINTN                                   SizeOfInfo;
    UINT32                                  Index;

    //

    for (Index = 0; Index < gGraphProtocol->Mode->MaxMode; Index++) {

      EFI_STATUS    Status;

      //

      Status = gGraphProtocol->QueryMode (gGraphProtocol, Index, &SizeOfInfo, &Info);
      if (Status == EFI_SUCCESS) {
        if ((*Width > Info->HorizontalResolution) || (*Height > Info->VerticalResolution)) {
          continue;
        }

        *Width    = Info->HorizontalResolution;
        *Height   = Info->VerticalResolution;
        *BestMode = Index;
      }
    }
  }
}
