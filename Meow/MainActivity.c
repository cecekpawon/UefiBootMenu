#include "MainActivity.h"
#include "MeowFunctions.h"

VOID
FreeActivity (
  IN OUT  ACTIVITY   *Activity
  )
{
  if (Activity != NULL) {

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Buffer;

    //

    Buffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((MAIN_ACTIVITY *)Activity)->BgBuffer;

    if (Buffer != NULL) {

      FreePool (Buffer);
    }

    Buffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((MAIN_ACTIVITY *)Activity)->Activity.Buffer;

    if (Buffer != NULL) {

      FreePool (Buffer);
    }

    FreePool ((MAIN_ACTIVITY *)Activity);
  }

  ClearScreen ((EFI_GRAPHICS_OUTPUT_BLT_PIXEL)BLACK_BG);
}

STATIC
VOID
MainActivityDrawRect (
  OUT  EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *Buffer,
  IN   UINT32                           Width,
  IN   RECT                             Rect,
  IN   EFI_GRAPHICS_OUTPUT_BLT_PIXEL    Color,
  IN   EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *Bg  OPTIONAL
  )
{
  UINT32  y;
  UINT32  x;
  UINT32  Base;

  //

  Base = Width * Rect.PosY + Rect.PosX;

  for (y = 0; y < Rect.Height; y++) {

    UINT32  Offset;

    //

    Offset = Base + y * Width;

    for (x = 0; x < Rect.Width; x++) {
      Buffer[Offset + x] = Bg ? Bg[Offset + x] : Color;
    }
  }
}

STATIC
EFI_STATUS
MainActivityDrawString (
  IN  MAIN_ACTIVITY   *MainActivity,
  IN  CHAR16          *String,
  IN  UINTN           PosX,
  IN  UINTN           PosY,
  IN  BOOLEAN         Reverse
  )
{
  EFI_FONT_DISPLAY_INFO   DisplayInfo;
  EFI_IMAGE_OUTPUT        Output;

  //

  ZeroMem (&DisplayInfo, sizeof (EFI_FONT_DISPLAY_INFO));

  DisplayInfo.FontInfoMask = EFI_FONT_INFO_ANY_FONT;

  if (Reverse) {
    DisplayInfo.ForegroundColor = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG;
  }

  else {
    DisplayInfo.ForegroundColor = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG;
  }

  Output.Width        = (UINT16)MainActivity->Activity.Width;
  Output.Height       = (UINT16)MainActivity->Activity.Height;
  Output.Image.Bitmap = MainActivity->Activity.Buffer;

  return DrawLines (String, &DisplayInfo, &Output, PosX, PosY);
}

STATIC
EFI_STATUS
MainActivityDrawBootOption (
  IN  MAIN_ACTIVITY   *MainActivity,
  IN  UINTN           Index,
  IN  BOOLEAN         Reverse,
  IN  BOOLEAN         Update
  )
{
  EFI_STATUS  Status;
  UINT32      PosY;
  RECT        Rect;
  RECT        RectPath;

  //

  PosY = MainActivity->BootOptionBaseY + DIM_ITEM_SEL_HEIGHT * (UINT32)Index;

  Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
  Rect.PosY   = PosY;
  Rect.Width  = MainActivity->Activity.Width - 2 * DIM_ITEM_SEL_H_OFFSET;
  Rect.Height = DIM_ITEM_SEL_HEIGHT;

  if (Reverse) {

    MainActivityDrawRect (
      MainActivity->Activity.Buffer,
      MainActivity->Activity.Width,
      Rect,
      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG,
      NULL
      );

    // Reversed measn selected, also draw boot path.

    RectPath.PosX   = 0;//DIM_ITEM_SEL_H_OFFSET;
    RectPath.PosY   = MainActivity->BootPathBaseY;
    RectPath.Width  = MainActivity->Activity.Width;
    RectPath.Height = DIM_ITEM_SEL_HEIGHT * 2;

    MainActivityDrawRect (
      MainActivity->Activity.Buffer,
      MainActivity->Activity.Width,
      RectPath,
      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG,
      MainActivity->BgBuffer
      );

    MainActivityDrawString (
      MainActivity,
      MeowPathToText (MainActivity->BootOptions[Index].FilePath),
      0,
      RectPath.PosY + DIM_ITEM_TEXT_Y_OFFSET,
      FALSE
      );
  }

  else if (Update) {

    MainActivityDrawRect (
      MainActivity->Activity.Buffer,
      MainActivity->Activity.Width,
      Rect,
      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG,
      MainActivity->BgBuffer
      );
  }

  Status = MainActivityDrawString (
              MainActivity,
              MainActivity->BootOptions[Index].Description,
              DIM_ITEM_TEXT_X_OFFSET,
              PosY + DIM_ITEM_TEXT_Y_OFFSET,
              Reverse
              );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Update) {

    ActivityInvalidate ((ACTIVITY *)MainActivity, Rect);

    if (Reverse) {

      ActivityInvalidate ((ACTIVITY *)MainActivity, RectPath);
    }
  }

  return EFI_SUCCESS;
}

STATIC
UINT32
MainActivityStrCpy (
  OUT  CHAR16  *Dst,
  IN   UINT32   DstSize,
  IN   CHAR16  *Src,
  IN   UINT32  DstOffset
  )
{
  UINT32  Offset;

  //

  for (Offset = 0; (DstOffset < DstSize) && (Src[Offset] != L'\0'); Offset++, DstOffset++ ) {
    Dst[DstOffset] = Src[Offset];
  }

  Dst[DstOffset] = L'\0';

  return DstOffset;
}

STATIC
EFI_STATUS
MainActivityDrawTimerString (
  IN  MAIN_ACTIVITY   *MainActivity
  )
{
  CHAR16  Line[256];
  UINT32  Offset;
  RECT    Rect;

  //

  Offset = 0;

  Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
  Rect.PosY   = MainActivity->BootAutoBaseY;
  Rect.Width  = MainActivity->Activity.Width - 2 * DIM_ITEM_SEL_H_OFFSET;
  Rect.Height = DIM_ITEM_SEL_HEIGHT;

  MainActivityDrawRect (
    MainActivity->Activity.Buffer,
    MainActivity->Activity.Width,
    Rect,
    (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG,
    MainActivity->BgBuffer
    );

  Offset  = MainActivityStrCpy (Line, ARRAY_SIZE (Line), MainActivity->StaticTexts[3], Offset);
  Offset += SprintUint ((UINT32)MainActivity->TimerCount, Line, ARRAY_SIZE (Line), Offset);

  MainActivityStrCpy (Line, ARRAY_SIZE (Line), MainActivity->StaticTexts[4], Offset);

  MainActivityDrawString (
    MainActivity,
    Line,
    DIM_ITEM_TEXT_X_OFFSET,
    Rect.PosY + DIM_ITEM_TEXT_Y_OFFSET,
    FALSE
    );

  ActivityInvalidate (&MainActivity->Activity, Rect);

  return EFI_SUCCESS;
}

EFI_STATUS
NewMainActivity (
  IN   UINT32     Width,
  IN   UINT32     Height,
  OUT  ACTIVITY   **Activity
  )
{
  EFI_STATUS                      Status;
  ACTIVITY                        *Super;
  MAIN_ACTIVITY                   *MainActivity;
  RECT                            Rect = { 0 };
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Color;

  //

  *Activity = AllocateZeroPool (sizeof (MAIN_ACTIVITY));

  if (*Activity == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Super = (ACTIVITY *)*Activity;

  //Super->IsNonFullScreen = FALSE;

  // Super init.
  Status = ActivityInitialize (Super, Width, Height);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  // This init.
  MainActivity = (MAIN_ACTIVITY *)Super;

  MainActivity->Width   = Width;
  MainActivity->Height  = Height;

  // Allocate memory for background Buffer.

  MainActivity->BgBuffer = AllocatePool (sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * Width * Height);

  if (MainActivity->BgBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Init text resouces. Not using res file for now.
  MainActivity->StaticTexts[0] = L"Meow Boot Menu";
  MainActivity->StaticTexts[1] = L"System efi boot options:";
  MainActivity->StaticTexts[2] = L"Boot failed.";
  MainActivity->StaticTexts[3] = L"Auto booting in ";
  MainActivity->StaticTexts[4] = L" second(s).";

  // Fill bg Buffer.

  Rect.Width  = Width;
  Rect.Height = Height;

  Color = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG;

  MainActivityDrawRect (MainActivity->BgBuffer, Width, Rect, Color, NULL);

  Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
  Rect.Width  = Width - DIM_ITEM_SEL_H_OFFSET * 2;
  Rect.PosY   = DIM_TITLE_Y_OFFSET;
  Rect.Height = DIM_TITLE_Y_OFFSET;

  Color = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_FG;

  MainActivityDrawRect (MainActivity->BgBuffer, Width, Rect, Color, NULL);

  return Status;
}

VOID
MainActivityOnStart (
  IN OUT  ACTIVITY  *Activity
  )
{
  UINT32          y;
  UINT32          x;
  UINTN           i;
  UINT32          Base;
  MAIN_ACTIVITY   *MainActivity;
  RECT            Rect = { 0 };

  //

  Base = 0;

  MainActivity = (MAIN_ACTIVITY *)Activity;

  for (y = 0; y < Activity->Height; y++) {

    for (x = 0; x < Activity->Width; x++) {
      Activity->Buffer[Base + x] = MainActivity->BgBuffer[Base + x];
    }

    Base += Activity->Width;
  }

  //EfiBootManagerConnectAll ();
  EfiBootManagerRefreshAllBootOption ();

  MainActivity->BootOptions       = EfiBootManagerGetLoadOptions (&MainActivity->BootOptionCount, LoadOptionTypeBoot);

  MainActivity->BootOptionBaseY   = DIM_TITLE_Y_OFFSET + 2 * DIM_ITEM_SEL_HEIGHT + 3 * DIM_PARA_SEP;
  MainActivity->BootFailedBaseY   = MainActivity->BootOptionBaseY + DIM_PARA_SEP + DIM_ITEM_SEL_HEIGHT * (UINT32)MainActivity->BootOptionCount;
  MainActivity->BootAutoBaseY     = MainActivity->BootFailedBaseY + DIM_ITEM_SEL_HEIGHT + DIM_PARA_SEP;
  MainActivity->IfShowBootFailed  = FALSE;
  MainActivity->BootPathBaseY     = MainActivity->BootAutoBaseY + DIM_ITEM_SEL_HEIGHT + DIM_PARA_SEP * 2;

  MainActivityDrawString (
    MainActivity,
    MainActivity->StaticTexts[0],
    DIM_ITEM_TEXT_X_OFFSET,
    DIM_TITLE_Y_OFFSET + DIM_ITEM_TEXT_Y_OFFSET,
    TRUE
    );

  MainActivityDrawString (
    MainActivity,
    MainActivity->StaticTexts[1],
    DIM_ITEM_SEL_H_OFFSET,
    DIM_TITLE_Y_OFFSET + DIM_ITEM_SEL_HEIGHT + 2 * DIM_PARA_SEP + DIM_ITEM_TEXT_Y_OFFSET,
    FALSE
    );

  if (MainActivity->BootOptionCount > 0) {
    MainActivityDrawBootOption (MainActivity, 0, TRUE, FALSE);
  }

  for (i = 1; i < MainActivity->BootOptionCount; i++) {
    MainActivityDrawBootOption (MainActivity, i, FALSE, FALSE);
  }

  MainActivity->Selection = 0;

  Rect.Width = Activity->Width;
  Rect.Height = Activity->Height;

  ActivityInvalidate (Activity, Rect);

  MainActivity->TimerCount = TIMER_COUNT;

  MainActivityDrawTimerString (MainActivity);

  Activity->CountInvalid = 1;
}

STATIC
UINTN
MainActivityOnMoveDown (
  IN OUT  MAIN_ACTIVITY   *MainActivity
  )
{
  if (MainActivity->BootOptionCount > 0) {

    MainActivityDrawBootOption (MainActivity, MainActivity->Selection, FALSE, TRUE);

    MainActivity->Selection++ ;

    if (MainActivity->Selection >= MainActivity->BootOptionCount) {
      MainActivity->Selection = 0;
    }

    MainActivityDrawBootOption (MainActivity, MainActivity->Selection, TRUE, TRUE);
  }

  return 0;
}

STATIC
UINTN
MainActivityOnMoveUp (
  IN OUT  MAIN_ACTIVITY   *MainActivity
  )
{
  if (MainActivity->BootOptionCount > 0) {

    MainActivityDrawBootOption (MainActivity, MainActivity->Selection, FALSE, TRUE);

    if (MainActivity->Selection == 0) {
      MainActivity->Selection = MainActivity->BootOptionCount - 1;
    }

    else {
      MainActivity->Selection--;
    }

    MainActivityDrawBootOption (MainActivity, MainActivity->Selection, TRUE, TRUE);
  }

  return 0;
}

STATIC
UINTN
MainActivityOnExit (
  IN OUT  MAIN_ACTIVITY   *MainActivity,
  IN      BOOLEAN         IsBoot
  )
{
  if (IsBoot) {
    UINT16  BootOrder;

    //

    BootOrder = (UINT16)MainActivity->BootOptions[MainActivity->Selection].OptionNumber;

    gRT->SetVariable (L"BootNext",
                      &gEfiGlobalVariableGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      sizeof(UINT16),
                      &BootOrder);

    //EfiBootManagerBoot (&MainActivity->BootOptions[MainActivity->Selection]);
  }

  EfiBootManagerFreeLoadOptions (MainActivity->BootOptions, MainActivity->BootOptionCount);

  return 1;
}

STATIC
UINTN
MainActivityOnEnter (
  IN OUT  MAIN_ACTIVITY   *MainActivity
  )
{
  return MainActivityOnExit (MainActivity, TRUE);
}

STATIC
UINTN
MainActivityOnEscape (
  IN OUT  MAIN_ACTIVITY   *MainActivity
  )
{
  return MainActivityOnExit (MainActivity, FALSE);
}

STATIC
VOID
MainActivityOnTimeout (
  IN OUT  MAIN_ACTIVITY   *MainActivity
  )
{
  RECT Rect;

  //

  Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
  Rect.PosY   = MainActivity->BootAutoBaseY;
  Rect.Width  = MainActivity->Activity.Width - 2 * DIM_ITEM_SEL_H_OFFSET;
  Rect.Height = DIM_ITEM_SEL_HEIGHT;

  MainActivityDrawRect (
    MainActivity->Activity.Buffer,
    MainActivity->Activity.Width,
    Rect,
    (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG,
    MainActivity->BgBuffer
    );

  ActivityInvalidate (&MainActivity->Activity, Rect);
}

STATIC
UINTN
MainActivityOnKey (
  IN OUT  MAIN_ACTIVITY   *MainActivity,
  IN      EFI_INPUT_KEY   Key
  )
{
  ACTIVITY  *Super;
  RECT      Rect;
  UINTN     Ret;

  //

  Ret = 0;

  MainActivity->TimerCount = -2;

  MainActivityOnTimeout (MainActivity);

  Super = (ACTIVITY *)MainActivity;

  Rect.PosX   = DIM_ITEM_SEL_H_OFFSET;
  Rect.PosY   = MainActivity->BootFailedBaseY;
  Rect.Width  = Super->Width - 2 * DIM_ITEM_SEL_H_OFFSET;
  Rect.Height = DIM_ITEM_SEL_HEIGHT;

  if (MainActivity->IfShowBootFailed) {

    MainActivityDrawRect (
      Super->Buffer,
      Super->Width,
      Rect,
      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)COLOR_BG,
      MainActivity->BgBuffer
      );

    ActivityInvalidate (Super, Rect);

    MainActivity->IfShowBootFailed = FALSE;
  }

  switch (Key.ScanCode) {

    case SCAN_UP:
      Ret = MainActivityOnMoveUp (MainActivity);
      break;

    case SCAN_DOWN:
      Ret = MainActivityOnMoveDown (MainActivity);
      break;

    case SCAN_ESC:
      Ret = MainActivityOnEscape (MainActivity);
      break;

    //case SCAN_NULL:
    default:
      break;
  }

  if (Key.ScanCode == SCAN_NULL) {

    switch (Key.UnicodeChar) {

      case CHAR_LINEFEED:
      case CHAR_CARRIAGE_RETURN:
        Ret = MainActivityOnEnter (MainActivity);
        break;
    }
  }

  return Ret;
}

UINTN
MainActivityOnEvent (
  IN OUT  ACTIVITY    *Activity
  )
{
  UINTN           Ret;
  EFI_STATUS      Status;
  MAIN_ACTIVITY   *MainActivity;
  EFI_INPUT_KEY   Key;
  EFI_EVENT       TimeoutEvent;

  //

  Ret = 0;

  MainActivity = (MAIN_ACTIVITY *)Activity;

  if (MainActivity->TimerCount >= -1) {

    MainActivityDrawTimerString (MainActivity);

    if (MainActivity->TimerCount == -1) {

      MainActivityOnEnter (MainActivity);
    }

    MainActivity->TimerCount--;
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

      Ret = MainActivityOnKey (MainActivity, Key);
    }
  }

  return Ret;
}
