#include "Meow.h"

//

STATIC  MEOW_ACTIVITY                     *gTopActivity       = NULL;
        EFI_GRAPHICS_OUTPUT_PROTOCOL      *gGraphProtocol     = NULL;
        EFI_HII_FONT_PROTOCOL             *gFontProtocol      = NULL;

#if MEOW_DEVPATH_LIB != 1
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL        *gPathConvProtocol  = NULL;
#endif

//

/**
  Main Loop.
**/
STATIC
MEOW_EVENT_RETURN
MeowLoop (
  VOID
  )
{
  MEOW_EVENT_RETURN   Ret;

  //

  Ret = EventReturnNone;

  while (TRUE) {

    EFI_STATUS    Status;

    //

    if (gTopActivity == NULL) {
      MeowLog (L"gTopActivity == NULL\n");
      Ret = EventReturnError;
      break;
    }

    Ret = MainActivityOnEvent (gTopActivity);

    Status = ActivityRender (gTopActivity, Ret);

    if (EFI_ERROR (Status)) {
      MeowLog (L"Cannot render Activity.\n");
      Ret = EventReturnError;
      break;
    }

    if (Ret != EventReturnNone) {
      break;
    }
  }

  return Ret;
}

///////////////

/**
  EntryPoint.
**/
EFI_STATUS
EFIAPI
UefiMain (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS          Status;

  MEOW_EVENT_RETURN   Ret;
  BOOLEAN             Reinit;
  BOOLEAN             Initialized;

  BOOLEAN             EnableCursor;
  UINT32              Width;
  UINT32              Height;
  UINT32              BestMode;

  //

  if (gST->ConOut == NULL) {
    goto Exit;
  }

  // Backup ConOut attributes.

  EnableCursor = gST->ConOut->Mode->CursorVisible;

  gST->ConOut->EnableCursor (gST->ConOut, FALSE);
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);

  // Graphic access.

  Status = gBS->LocateProtocol (
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  (VOID **)&gGraphProtocol
                  );

  if (EFI_ERROR (Status)) {
    // Well, if graphic's not available, where comes the shell?
    MeowLog (L"Cannot locate graphics output protocol.\n");
    goto Done;
  }

  Width     = 0;
  Height    = 0;
  BestMode  = 0;

  SetMaxRes (&Width, &Height, &BestMode);

  Status = gGraphProtocol->SetMode (gGraphProtocol, BestMode);

  //Status = gBS->InstallMultipleProtocolInterfaces (
  //                &gST->ConsoleOutHandle,
  //                &gEfiGraphicsOutputProtocolGuid,
  //                gGraphProtocol,
  //                NULL
  //                );

  // Locate font protocol, we use it to draw strings.
  Status = gBS->LocateProtocol (&gEfiHiiFontProtocolGuid, NULL, (VOID **)&gFontProtocol);

  if (EFI_ERROR (Status)) {
    MeowLog (L"Cannot locate font protocol.\n");
    goto Done;
  }

  Initialized = FALSE;

  Init:

  Reinit = FALSE;

  // Create the Main Activity.
  Status = NewMainActivity (Width, Height, &gTopActivity);

  if (EFI_ERROR (Status)) {
    MeowLog (L"Cannot allocate MainActivity.\n");
    goto Done;
  }

  #if MEOW_DEVPATH_LIB != 1
    Status = gBS->LocateProtocol (
                    &gEfiDevicePathToTextProtocolGuid,
                    NULL,
                    (VOID **)&gPathConvProtocol
                    );

    if (EFI_ERROR (Status)) {
      MeowLog (L"Cannot locate device path to text protocol.");
      gPathConvProtocol = NULL;
      goto Done;
    }
  #endif // MEOW_DEVPATH_LIB

  // Start the Activity.
  MainActivityOnStart (gTopActivity, Initialized);

  Initialized = TRUE;

  // Enter event loop.

  //MeowLog (L"-> loop.\n");
  Ret = MeowLoop ();
  //MeowLog (L"<- loop.\n");

  if (Ret == EventReturnRefresh) {
    MeowLog (L"Reinit.\n");
    Reinit = TRUE;
  }

  FreeActivity (gTopActivity, Reinit);

  if (Reinit) {
    goto Init;
  }

  Done:

  // Restore ConOut attributes.

  gST->ConOut->EnableCursor (gST->ConOut, EnableCursor);
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);

  Exit:

  #if defined(MEOW_MODE) && (MEOW_MODE == APPLICATION)
    Status = EFI_SUCCESS;
  #endif

  // Bye.
  return Status;
}
