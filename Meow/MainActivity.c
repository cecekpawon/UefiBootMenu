#include "Meow.h"

STATIC CHAR16   *StaticTexts[] = {
  L"Meow Boot Menu",
  L"ENTER = Boot | ESC = Exit | F5 = Refresh",
  L"System efi boot options:",
  L"Boot failed.",
  L"Auto booting in",
  L"second(s).",
  NULL
};

/**
  Free Activity resources.
**/
VOID
FreeActivity (
  IN OUT  MEOW_ACTIVITY   *Activity,
  IN      BOOLEAN         Reinit
  )
{
  MeowClearScreen (Activity, (Reinit ? (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG : (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)BLACK_BG));

  if (Activity != NULL) {

    VOID   *Buffer;

    //

    Buffer = (VOID *)((MEOW_MAIN_ACTIVITY *)Activity)->BgBuffer;

    if (Buffer != NULL) {

      FreePool (Buffer);
    }

    Buffer = (VOID *)((MEOW_MAIN_ACTIVITY *)Activity)->Activity.Buffer;

    if (Buffer != NULL) {

      FreePool (Buffer);
    }

    Buffer = (VOID *)((MEOW_MAIN_ACTIVITY *)Activity)->PathBuffer;

    if (Buffer != NULL) {

      FreePool (Buffer);
    }

    FreePool ((MEOW_MAIN_ACTIVITY *)Activity);

    Activity = NULL;
  }
}

/**
  Draw String.
**/
STATIC
EFI_STATUS
MainActivityDrawString (
  IN  MEOW_MAIN_ACTIVITY    *This,
  IN  CHAR16                *String,
  IN  UINTN                 PosX,
  IN  UINTN                 PosY,
  IN  BOOLEAN               Reverse
  )
{
  EFI_FONT_DISPLAY_INFO   DisplayInfo;
  EFI_IMAGE_OUTPUT        Output;

  //

  ZeroMem (&DisplayInfo, sizeof (EFI_FONT_DISPLAY_INFO));

  DisplayInfo.FontInfoMask    = EFI_FONT_INFO_ANY_FONT;
  DisplayInfo.ForegroundColor = Reverse
                                  ? (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG
                                  : (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG;
  DisplayInfo.BackgroundColor = Reverse
                                  ? (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG
                                  : (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG;

  Output.Width                = (UINT16)This->Activity.Width;
  Output.Height               = (UINT16)This->Activity.Height;
  Output.Image.Bitmap         = This->Activity.Buffer;
  //Output.Image.Screen         = gGraphProtocol;

  return MeowDrawLines (String, &DisplayInfo, &Output, PosX, PosY);
}

STATIC
EFI_STATUS
MainActivityDrawHelp (
  IN  MEOW_MAIN_ACTIVITY    *This
  )
{
  return MainActivityDrawString (
            This,
            This->StaticTexts[StaticTextsIndexHelpTitle],
            (This->Activity.Width - (StrLen (This->StaticTexts[StaticTextsIndexHelpTitle]) * EFI_GLYPH_WIDTH)) / 2,
            This->Activity.Height - EFI_GLYPH_HEIGHT - DIM_PARA_SEP,
            FALSE
            );
}

/**
  Set Timeout.
**/
STATIC
EFI_STATUS
MainActivitySetTimeout (
  IN OUT  MEOW_MAIN_ACTIVITY   *This
  )
{
  if (This->TimerCount != TIMER_TIMEOUT) {

    This->TimerCount = TIMER_TIMEOUT;

    MainActivityDrawHelp (This);
  }

  return EFI_TIMEOUT;
}

/**
  Draw Rect.
**/
STATIC
VOID
MainActivityDrawRect (
  OUT  EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *Buffer,
  IN   UINT32                           Width,
  IN   RECT                             Rect,
  IN   EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *Color,
  IN   EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *Bg  OPTIONAL
  )
{
  if ((Bg != NULL) || (Color != NULL)) {

    UINT32  y;
    UINT32  Base;

    //

    Base = Width * Rect.PosY + Rect.PosX;

    for (y = 0; y < Rect.Height; y++) {

      UINT32  Offset;
      UINT32  x;

      //

      Offset = Base + y * Width;

      for (x = 0; x < Rect.Width; x++) {
        Buffer[Offset + x] = (Bg != NULL) ? Bg[Offset + x] : *Color;
      }
    }
  }
}

/**
  Draw BootOption.
**/
STATIC
EFI_STATUS
MainActivityDrawBootOption (
  IN  MEOW_MAIN_ACTIVITY  *This,
  IN  UINTN               Index,
  IN  BOOLEAN             Reverse,
  IN  BOOLEAN             Update
  )
{
  EFI_STATUS  Status;

  UINT32      PosY;
  RECT        Rect;
  RECT        RectPath;

  //

  PosY = This->BootOptionBaseY + DIM_ITEM_SEL_HEIGHT * (UINT32)Index;

  Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
  Rect.PosY   = PosY;
  Rect.Width  = This->Activity.Width - (2 * Rect.PosX);
  Rect.Height = DIM_ITEM_SEL_HEIGHT;

  if (Reverse) {

    CHAR16 *DevicePathString;

    //

    MainActivityDrawRect (
      This->Activity.Buffer,
      This->Activity.Width,
      Rect,
      &((EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG),
      NULL
      );

    // Reverse means selected, also draw boot path.

    RectPath.PosX   = DIM_ITEM_SEL_H_OFFSET;
    RectPath.PosY   = This->BootPathBaseY;
    RectPath.Width  = This->Activity.Width - (2 * RectPath.PosX);
    RectPath.Height = (UINT32)(EFI_GLYPH_HEIGHT * This->PathRowCount);

    MainActivityDrawRect (
      This->Activity.Buffer,
      This->Activity.Width,
      RectPath,
      &((EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG),
      This->BgBuffer
      );

    This->PathRowCount = 1;

    DevicePathString = MeowPathToText (This->BootOptions[Index].FilePath);

    if (DevicePathString != NULL) {

      UINTN   PathRowCount;
      UINTN   RowIndex;
      UINTN   DevicePathStringLen;
      UINTN   CharPos;

      //

      CharPos             = 0;
      DevicePathStringLen = StrLen (DevicePathString);

      PathRowCount = This->PathRowCount;

      if (DevicePathStringLen > This->CharRowCount) {

        This->PathRowCount = DevicePathStringLen / This->CharRowCount;

        if ((This->PathRowCount * This->CharRowCount) < DevicePathStringLen) {

          This->PathRowCount++;
        }
      }

      if (PathRowCount < This->PathRowCount) {

        RectPath.Height = (UINT32)(EFI_GLYPH_HEIGHT * This->PathRowCount);

        MainActivityDrawRect (
          This->Activity.Buffer,
          This->Activity.Width,
          RectPath,
          &((EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG),
          This->BgBuffer
          );
      }

      // Split long DevicePath text.

      for (RowIndex = 0; RowIndex < This->PathRowCount; RowIndex++) {

        UINTN   CharIndex;

        //

        for (CharIndex = 0; CharIndex < This->CharRowCount; CharIndex++) {

          This->PathBuffer[CharIndex] = DevicePathString[CharPos];

          if (DevicePathString[CharPos] == L'\0') {

            break;
          }

          CharPos++;
        }

        This->PathBuffer[CharIndex] = L'\0';

        if (CharIndex > 0) {

          MainActivityDrawString (
            This,
            This->PathBuffer,
            RectPath.PosX,
            RectPath.PosY + (RowIndex * EFI_GLYPH_HEIGHT),
            FALSE
            );
        }
      }

      FreePool (DevicePathString);
    }
  }

  else if (Update) {

    MainActivityDrawRect (
      This->Activity.Buffer,
      This->Activity.Width,
      Rect,
      &((EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG),
      This->BgBuffer
      );
  }

  Status = MainActivityDrawString (
              This,
              This->BootOptions[Index].Description,
              DIM_ITEM_TEXT_X_OFFSET,
              PosY + DIM_ITEM_TEXT_Y_OFFSET,
              Reverse
              );

  if (!EFI_ERROR (Status)) {

    if (Update) {

      //@ Hack - prevent PosX repad on Qemu.
      Rect.Width += Rect.PosX;
      Rect.PosX   = 0;

      ActivityInvalidate ((MEOW_ACTIVITY *)This, Rect);

      if (Reverse) {

        //@ Hack - prevent PosX repad on Qemu.
        RectPath.Width += RectPath.PosX;
        RectPath.PosX   = 0;

        ActivityInvalidate ((MEOW_ACTIVITY *)This, RectPath);
      }
    }
  }

  return Status;
}

#if MEOW_PROGRESSBAR == 1
/**
  Draw ProgressBar.

  Taken from BootLogoLib.

  Update progress bar with title above it. It only works in Graphics mode.

  @param Title           Title above progress bar.
  @param Progress        Progress (0-100)
  @param PreviousValue   The previous value of the progress.

  @retval  EFI_STATUS       Success update the progress bar

**/
STATIC
EFI_STATUS
EFIAPI
MeowBootLogoUpdateProgress (
  IN  MEOW_MAIN_ACTIVITY  *This,
  IN  CHAR16              *Title,
  IN  UINTN               Progress,
  IN  UINTN               PreviousValue
  )
{
  EFI_STATUS  Status;

  //

  if (Progress > 100) {

    Status = EFI_INVALID_PARAMETER;
  }

  else {

    UINT32                            SizeOfX;
    UINT32                            SizeOfY;
    //EFI_GRAPHICS_OUTPUT_BLT_PIXEL     Color;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL     Background;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL     Foreground;
    UINTN                             BlockHeight;
    UINTN                             BlockWidth;
    UINTN                             BlockNum;
    UINTN                             PosX;
    UINTN                             PosY;
    UINTN                             Index;
    RECT                              Rect;

    //

    //Color       = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)BLACK_BG;
    Background  = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG;
    Foreground  = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG;

    SizeOfX     = This->Activity.Width;
    SizeOfY     = This->Activity.Height;

    BlockWidth  = SizeOfX / 100;
    BlockHeight = BlockWidth >> 1; // SizeOfY / 50

    BlockNum    = Progress;

    PosX        = ((SizeOfX % 100) == 0) ? 0 : ((SizeOfX % 100) / 2);
    PosY        = SizeOfY * 48 / 50;

    //
    // Clear progress area
    //

    //if (BlockNum == 0) {
    //  Status = gGraphProtocol->Blt (
    //                              gGraphProtocol,
    //                              &Color,
    //                              EfiBltVideoFill,
    //                              0,
    //                              0,
    //                              0,
    //                              PosY - EFI_GLYPH_HEIGHT - DIM_PARA_SEP,
    //                              SizeOfX,
    //                              SizeOfY - (PosY - EFI_GLYPH_HEIGHT - 1),
    //                              SizeOfX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
    //                              );
    //}

    //
    // Show progress by drawing blocks
    //

    for (Index = PreviousValue; Index < BlockNum; Index++) {

      UINTN   CurrPosX;

      //

      CurrPosX  = PosX + (Index * BlockWidth);
      Status    = gGraphProtocol->Blt (
                                    gGraphProtocol,
                                    &Foreground,
                                    EfiBltVideoFill,
                                    0,
                                    0,
                                    CurrPosX,
                                    PosY,
                                    (Index && (Index % 10) == 0) ? (BlockWidth - 1) : BlockWidth, // (BlockWidth - 1)
                                    BlockHeight,
                                    BlockWidth * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                                    );
    }

    //
    // Title
    //

    Rect.PosX   = 0;
    Rect.PosY   = (UINT32)(PosY - EFI_GLYPH_HEIGHT - DIM_PARA_SEP);
    Rect.Width  = SizeOfX;
    Rect.Height = EFI_GLYPH_HEIGHT;

    MainActivityDrawRect (
      This->Activity.Buffer,
      This->Activity.Width,
      Rect,
      &Background,
      This->BgBuffer
      );

    MainActivityDrawString (
      This,
      Title,
      (SizeOfX - (StrLen (Title) * EFI_GLYPH_WIDTH)) / 2,
      Rect.PosY,
      FALSE
      );

    ActivityInvalidate (&This->Activity, Rect);

    Status = EFI_SUCCESS;
  }

  return Status;
}

/**
  Draw ProgressBar.
**/
STATIC
EFI_STATUS
MainActivityDrawTimerProgressBar (
  IN  MEOW_MAIN_ACTIVITY   *This
  )
{
  EFI_STATUS  Status;

  //

  if (This->TimerCount > TIMER_COUNT) {

    Status = EFI_TIMEOUT;
  }

  else {

    CHAR16  *TmpStr;

    //

    TmpStr = CatSPrint (
                NULL,
                L"%s %02d %s",
                This->StaticTexts[StaticTextsIndexAutoBoot],
                This->TimerCount,
                This->StaticTexts[StaticTextsIndexSeconds]
                );

    if (TmpStr != NULL) {

      Status = MeowBootLogoUpdateProgress (
                  This,
                  TmpStr,
                  ((TIMER_COUNT - This->TimerCount) * 100) / TIMER_COUNT,
                  0
                  );

      if (!EFI_ERROR (Status)) {

        Status = (This->TimerCount < 0) ? EFI_TIMEOUT : EFI_SUCCESS;
      }

      FreePool (TmpStr);
    }

    else {

      Status = EFI_OUT_OF_RESOURCES;
    }
  }

  return Status;
}

#else // MEOW_PROGRESSBAR

/**
  Draw TimerString.
**/
STATIC
EFI_STATUS
MainActivityDrawTimerString (
  IN  MEOW_MAIN_ACTIVITY   *This
  )
{
  EFI_STATUS  Status;

  //

  if (This->TimerCount > TIMER_COUNT) {

    Status = EFI_TIMEOUT;
  }

  else {

    CHAR16  *String;
    RECT    Rect;

    //

    Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
    Rect.PosY   = This->BootAutoBaseY;
    Rect.Width  = This->Activity.Width - (2 * Rect.PosX);
    Rect.Height = DIM_ITEM_SEL_HEIGHT;

    MainActivityDrawRect (
      This->Activity.Buffer,
      This->Activity.Width,
      Rect,
      &((EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG),
      This->BgBuffer
      );

    String = CatSPrint (
              NULL,
              L"%s %02d %s",
              This->StaticTexts[StaticTextsIndexAutoBoot],
              This->TimerCount,
              This->StaticTexts[StaticTextsIndexSeconds]
              );

    if (String != NULL) {

      MainActivityDrawString (
        This,
        String,
        Rect.PosX,
        Rect.PosY,
        FALSE
        );

      //@ Hack - prevent PosX repad on Qemu.
      Rect.Width += Rect.PosX;
      Rect.PosX   = 0;

      ActivityInvalidate (&This->Activity, Rect);

      FreePool (String);
    }

    Status = (This->TimerCount < 0) ? EFI_TIMEOUT : EFI_SUCCESS;
  }

  return Status;
}
#endif // MEOW_PROGRESSBAR

/**
  Init MainActivity.
**/
EFI_STATUS
NewMainActivity (
  IN   UINT32         Width,
  IN   UINT32         Height,
  OUT  MEOW_ACTIVITY  **Activity
  )
{
  EFI_STATUS      Status;
  MEOW_ACTIVITY   *Super;

  //

  *Activity = AllocateZeroPool (sizeof (MEOW_MAIN_ACTIVITY));

  if (*Activity == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Super = (MEOW_ACTIVITY *)*Activity;

  // Super init.
  Status = ActivityInitialize (Super, Width, Height);

  if (!EFI_ERROR (Status)) {

    MEOW_MAIN_ACTIVITY              *This;
    RECT                            Rect;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Color;

    //

    ZeroMem (&Rect, sizeof (Rect));

    // This init.
    This = (MEOW_MAIN_ACTIVITY *)Super;

    This->Width   = Width;
    This->Height  = Height;

    // Allocate memory for background Buffer.

    This->BgBuffer = AllocatePool (sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * Width * Height);

    if (This->BgBuffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    // Init text resouces. Not using res file for now.

    This->StaticTexts = &StaticTexts[0];

    // Fill bg Buffer.

    Rect.Width  = Width;
    Rect.Height = Height;

    Color = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG;

    MainActivityDrawRect (This->BgBuffer, Width, Rect, &Color, NULL);

    // Main Title.

    Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
    Rect.Width  = Width - (2 * Rect.PosX);
    Rect.PosY   = DIM_TITLE_Y_OFFSET;
    Rect.Height = DIM_TITLE_Y_OFFSET;

    Color = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG;

    MainActivityDrawRect (This->BgBuffer, Width, Rect, &Color, NULL);
  }

  return Status;
}

STATIC
VOID
MainActivityOnTimeout (
  IN OUT  MEOW_MAIN_ACTIVITY   *This
  )
{
  if (This->TimerCount < TIMER_TIMEOUT) {

    RECT Rect;

    //

    #if MEOW_PROGRESSBAR == 1
      UINTN   PosY;

      //

      PosY = This->Activity.Height * 48 / 50;

      Rect.PosX   = 0;
      Rect.PosY   = (UINT32)(PosY - EFI_GLYPH_HEIGHT - DIM_PARA_SEP);
      Rect.Width  = This->Activity.Width;
      Rect.Height = This->Activity.Height - Rect.PosY;
    #else
      Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
      Rect.PosY   = This->BootAutoBaseY;
      Rect.Width  = This->Activity.Width - (2 * Rect.PosX);
      Rect.Height = DIM_ITEM_SEL_HEIGHT;
    #endif

    MainActivityDrawRect (
      This->Activity.Buffer,
      This->Activity.Width,
      Rect,
      &((EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG),
      This->BgBuffer
      );

    //@ Hack - prevent PosX repad on Qemu.
    Rect.Width += Rect.PosX;
    Rect.PosX   = 0;

    ActivityInvalidate (&This->Activity, Rect);

    MainActivitySetTimeout (This);
  }
}

/**
  Start MainActivity.
**/
VOID
MainActivityOnStart (
  IN OUT  MEOW_ACTIVITY   *Activity,
  IN      BOOLEAN         Reinit
  )
{
  MEOW_MAIN_ACTIVITY  *This;
  RECT                Rect;
  UINT32              y;
  UINT32              x;
  UINT32              Base;
  UINTN               i;

  //

  Base = 0;

  This = (MEOW_MAIN_ACTIVITY *)Activity;

  for (y = 0; y < Activity->Height; y++) {

    for (x = 0; x < Activity->Width; x++) {
      Activity->Buffer[Base + x] = This->BgBuffer[Base + x];
    }

    Base += Activity->Width;
  }

  // Connect drivers

  #if MEOW_BM_CONNECT_ALL == 1
    EfiBootManagerConnectAll ();
  #endif

  // Get BootOptions

  #if MEOW_BM_REFRESH_BOOT_OPTIONS == 1
    EfiBootManagerRefreshAllBootOption ();
  #endif

  // Init

  This->BootOptions       = EfiBootManagerGetLoadOptions (&This->BootOptionCount, LoadOptionTypeBoot);
  This->BootOptionBaseY   = DIM_TITLE_Y_OFFSET + (2 * DIM_ITEM_SEL_HEIGHT) + (3 * DIM_PARA_SEP);
  //This->BootFailedBaseY   = This->BootOptionBaseY + DIM_PARA_SEP + ((UINT32)This->BootOptionCount * DIM_ITEM_SEL_HEIGHT);
  //This->BootAutoBaseY     = This->BootFailedBaseY + DIM_ITEM_SEL_HEIGHT + DIM_PARA_SEP;
  This->BootAutoBaseY     = This->BootOptionBaseY + DIM_PARA_SEP + ((UINT32)This->BootOptionCount * DIM_ITEM_SEL_HEIGHT);
  This->BootAutoBaseY    += DIM_ITEM_SEL_HEIGHT + DIM_PARA_SEP;
  //This->IfShowBootFailed  = FALSE;
  This->BootPathBaseY     = This->BootAutoBaseY + DIM_ITEM_SEL_HEIGHT + (2 * DIM_PARA_SEP);
  This->PathRowCount      = 1;
  This->CharRowCount      = (Activity->Width - (2 * DIM_ITEM_SEL_H_OFFSET)) / EFI_GLYPH_WIDTH;
  This->PathBuffer        = AllocatePool (This->CharRowCount * sizeof (CHAR16));

  // Main Title

  MainActivityDrawString (
    This,
    This->StaticTexts[StaticTextsIndexMainTitle],
    DIM_ITEM_TEXT_X_OFFSET,
    DIM_TITLE_Y_OFFSET + DIM_ITEM_TEXT_Y_OFFSET,
    TRUE
    );

  // List Title

  MainActivityDrawString (
    This,
    This->StaticTexts[StaticTextsIndexListTitle],
    DIM_ITEM_SEL_H_OFFSET,
    DIM_TITLE_Y_OFFSET + DIM_ITEM_SEL_HEIGHT + (2 * DIM_PARA_SEP) + DIM_ITEM_TEXT_Y_OFFSET,
    FALSE
    );

  // Menu Items

  for (i = 0; i < This->BootOptionCount; i++) {

    #if MEOW_DRAW_BOOT_OPTION_NUMBER == 1
      CHAR16  *Description;

      //

      Description = CatSPrint (
                      NULL,
                      L"%04X | %s",
                      (UINT16)(This->BootOptions[i].OptionNumber),
                      This->BootOptions[i].Description);

      FreePool (This->BootOptions[i].Description);

      This->BootOptions[i].Description = Description;
    #endif // MEOW_DRAW_BOOT_OPTION_NUMBER

    MainActivityDrawBootOption (This, i, (i == 0), FALSE);
  }

  This->Selection = 0;

  ZeroMem (&Rect, sizeof (Rect));

  Rect.Width  = Activity->Width;
  Rect.Height = Activity->Height;

  ActivityInvalidate (Activity, Rect);

  This->TimerCount = TIMER_COUNT;

  // No countdown on Reinit.
  if (Reinit) {

    MainActivityOnTimeout (This);
  }

  //#if MEOW_PROGRESSBAR != 1
  //  MainActivityDrawTimerString (This);
  //#endif

  Activity->CountInvalid = 1;
}

/**
  Ret != EventReturnNone exit Loop.
**/
STATIC
MEOW_EVENT_RETURN
MainActivityOnMoveDown (
  IN OUT  MEOW_MAIN_ACTIVITY   *This
  )
{
  if (This->BootOptionCount > 0) {

    MainActivityDrawBootOption (This, This->Selection, FALSE, TRUE);

    This->Selection++ ;

    if (This->Selection >= This->BootOptionCount) {
      This->Selection = 0;
    }

    MainActivityDrawBootOption (This, This->Selection, TRUE, TRUE);
  }

  return EventReturnNone;
}

/**
  Ret != EventReturnNone exit Loop.
**/
STATIC
MEOW_EVENT_RETURN
MainActivityOnMoveUp (
  IN OUT  MEOW_MAIN_ACTIVITY   *This
  )
{
  if (This->BootOptionCount > 0) {

    MainActivityDrawBootOption (This, This->Selection, FALSE, TRUE);

    if (This->Selection == 0) {
      This->Selection = This->BootOptionCount - 1;
    }

    else {
      This->Selection--;
    }

    MainActivityDrawBootOption (This, This->Selection, TRUE, TRUE);
  }

  return EventReturnNone;
}

#if MEOW_BM_BOOT == 1
STATIC
VOID
MeowEfiBootManagerBoot (
  IN  EFI_BOOT_MANAGER_LOAD_OPTION    *BootOption
  )
{
  EFI_STATUS    Status;
  EFI_HANDLE    ImageHandle;

  //

  Status        = EFI_INVALID_PARAMETER;
  ImageHandle   = NULL;

  if (BootOption != NULL) {

    if ((BootOption->FilePath != NULL)
      && (BootOption->OptionType == LoadOptionTypeBoot)
      )
    {
      EfiBootManagerConnectDevicePath (BootOption->FilePath, NULL);

      Status = gBS->LoadImage (FALSE, gImageHandle, BootOption->FilePath, NULL, 0, &ImageHandle);

      if (!EFI_ERROR (Status)) {

        EFI_LOADED_IMAGE_PROTOCOL   *LoadedImage;

        //

        Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);

        if (!EFI_ERROR (Status)) {

          if (LoadedImage->ImageCodeType != EfiLoaderCode) {

            gBS->Exit (ImageHandle, EFI_INVALID_PARAMETER, 0, NULL);

            Status = EFI_INVALID_PARAMETER;
          }

          else {

            if (BootOption->OptionalDataSize) {
              LoadedImage->LoadOptionsSize  = BootOption->OptionalDataSize;
              LoadedImage->LoadOptions      = BootOption->OptionalData;
            }

            gRT->SetVariable (
                    EFI_BOOT_CURRENT_VARIABLE_NAME,
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    sizeof (UINT16),
                    &BootOption->OptionNumber
                    );

            EfiSignalEventReadyToBoot ();

            Status = gBS->StartImage (ImageHandle, &BootOption->ExitDataSize, &BootOption->ExitData);
          }
        }
      }
    }
  }

  //
  // Attempt to reboot into UEFI setup.
  //
  if (EFI_ERROR (Status)) {

    UINT64  OsIndications;
    UINT32  Attr;
    UINTN   DataSize;

    //

    //
    // Clear BootNext
    //
    Status = gRT->SetVariable (
                    EFI_BOOT_NEXT_VARIABLE_NAME,
                    &gEfiGlobalVariableGuid,
                    0,
                    0,
                    NULL
                    );

    if (ImageHandle != NULL) {

      gBS->UnloadImage (ImageHandle);
    }

    DataSize  = sizeof (OsIndications);
    Status    = gRT->GetVariable (
                       EFI_OS_INDICATIONS_SUPPORT_VARIABLE_NAME,
                       &gEfiGlobalVariableGuid,
                       &Attr,
                       &DataSize,
                       &OsIndications
                       );

    if (!EFI_ERROR (Status)) {

      if ((OsIndications & EFI_OS_INDICATIONS_BOOT_TO_FW_UI) != 0) {

        DataSize  = sizeof (OsIndications);
        Status    = gRT->GetVariable (
                            EFI_OS_INDICATIONS_VARIABLE_NAME,
                            &gEfiGlobalVariableGuid,
                            &Attr,
                            &DataSize,
                            &OsIndications
                            );

        if (!EFI_ERROR (Status)) {
          OsIndications |= EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
        }

        else {
          OsIndications  = EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
        }

        Status = gRT->SetVariable (
                        EFI_OS_INDICATIONS_VARIABLE_NAME,
                        &gEfiGlobalVariableGuid,
                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                        sizeof (OsIndications),
                        &OsIndications
                        );

        gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
      }
    }
  }

  if (BootOption != NULL) {

    BootOption->Status = Status;
  }
}
#endif // MEOW_BM_BOOT

/**
**/
STATIC
MEOW_EVENT_RETURN
MainActivityOnExit (
  IN OUT  MEOW_MAIN_ACTIVITY  *This,
  IN      MEOW_EVENT_RETURN   Ret
  )
{
  MeowClearScreen (&This->Activity, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)BLACK_BG);

  // On entering item only, not ESC

  if (Ret == EventReturnEnter) {

    EFI_BOOT_MANAGER_LOAD_OPTION    BootOption;

    //

    BootOption = This->BootOptions[This->Selection];

    #if MEOW_BM_BOOT == 1
      //EfiBootManagerBoot (&BootOption);
      MeowEfiBootManagerBoot (&BootOption);

      if (EFI_ERROR (BootOption.Status)) {
        // Handle boot failed.
      }

    #else
      gRT->SetVariable (
              EFI_BOOT_NEXT_VARIABLE_NAME,
              &gEfiGlobalVariableGuid,
              EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
              sizeof (UINT16),
              &BootOption.OptionNumber
              );
    #endif // MEOW_BM_BOOT
  }

  // Free Load Options

  EfiBootManagerFreeLoadOptions (This->BootOptions, This->BootOptionCount);

  return Ret;
}

/**
**/
STATIC
MEOW_EVENT_RETURN
MainActivityOnEnter (
  IN OUT  MEOW_MAIN_ACTIVITY   *This
  )
{
  return MainActivityOnExit (This, EventReturnEnter);
}

/**
**/
STATIC
MEOW_EVENT_RETURN
MainActivityOnEscape (
  IN OUT  MEOW_MAIN_ACTIVITY   *This
  )
{
  return MainActivityOnExit (This, EventReturnEscape);
}

/**
**/
STATIC
MEOW_EVENT_RETURN
MainActivityOnRefresh (
  IN OUT  MEOW_MAIN_ACTIVITY   *This
  )
{
  return MainActivityOnExit (This, EventReturnRefresh);
}

/**
  Ret != EventReturnNone exit Loop.
**/
STATIC
MEOW_EVENT_RETURN
MainActivityOnKey (
  IN OUT  MEOW_MAIN_ACTIVITY  *This,
  IN      EFI_INPUT_KEY       Key
  )
{
  //MEOW_ACTIVITY       *Super;
  //RECT                Rect;
  MEOW_EVENT_RETURN   Ret;

  //

  Ret = EventReturnNone;

  MainActivityOnTimeout (This);

  /*
  Super = (MEOW_ACTIVITY *)This;

  //
  // BootFailed
  //

  Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
  Rect.PosY   = This->BootFailedBaseY;
  Rect.Width  = Super->Width - (2 * Rect.PosX);
  Rect.Height = DIM_ITEM_SEL_HEIGHT;

  if (This->IfShowBootFailed) {

    MainActivityDrawRect (
      Super->Buffer,
      Super->Width,
      Rect,
      &((EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG),
      This->BgBuffer
      );

    ActivityInvalidate (Super, Rect);

    This->IfShowBootFailed = FALSE;
  }
  */

  switch (Key.ScanCode) {

    case SCAN_UP:
      Ret = MainActivityOnMoveUp (This);
      break;

    case SCAN_DOWN:
      Ret = MainActivityOnMoveDown (This);
      break;

    case SCAN_ESC:
      Ret = MainActivityOnEscape (This);
      break;

    case SCAN_F5:
      Ret = MainActivityOnRefresh (This);
      break;

    //case SCAN_NULL:
    default:
      break;
  }

  if (Key.ScanCode == SCAN_NULL) {

    switch (Key.UnicodeChar) {

      case CHAR_LINEFEED:
      case CHAR_CARRIAGE_RETURN:
      //case 0x0020: // SPACE
        Ret = MainActivityOnEnter (This);
        break;
    }
  }

  return Ret;
}

/**
  Ret != EventReturnNone exit Loop.
**/
MEOW_EVENT_RETURN
MainActivityOnEvent (
  IN OUT  MEOW_ACTIVITY    *Activity
  )
{
  MEOW_EVENT_RETURN   Ret;

  EFI_STATUS          Status;
  MEOW_MAIN_ACTIVITY  *This;
  EFI_INPUT_KEY       Key;
  EFI_EVENT           TimeoutEvent;

  //

  Ret = EventReturnNone;

  This = (MEOW_MAIN_ACTIVITY *)Activity;

  #if MEOW_PROGRESSBAR == 1
    Status = MainActivityDrawTimerProgressBar (This);
  #else
    Status = MainActivityDrawTimerString (This);
  #endif

  if (!EFI_ERROR (Status)) {

    This->TimerCount--;
  }

  else {

    if (This->TimerCount < 0) {

      Ret = MainActivityOnEnter (This);
    }
  }

  TimeoutEvent  = NULL;
  Status        = gBS->CreateEvent (EVT_TIMER, 0, NULL, NULL, &TimeoutEvent);

  if (!EFI_ERROR (Status)) {

    BOOLEAN   IsOnKey;

    //

    IsOnKey = FALSE;

    gBS->SetTimer (TimeoutEvent, TimerRelative, 10000000);

    while (EFI_ERROR (gBS->CheckEvent (TimeoutEvent))) {

      Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);

      if (!EFI_ERROR (Status)) {

        IsOnKey = TRUE;

        break;
      }
    }

    Status = gBS->SetTimer (TimeoutEvent, TimerCancel, 0);

    if (!EFI_ERROR (Status)) {

      Status = gBS->CloseEvent (TimeoutEvent);
    }

    if (IsOnKey) {

      Ret = MainActivityOnKey (This, Key);
    }
  }

  return Ret;
}
