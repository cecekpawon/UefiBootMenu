#ifndef MEOW_H
#define MEOW_H

//

// Append BootOptionNumber on MenuItems
#ifndef MEOW_DRAW_BOOT_OPTION_NUMBER
  #define MEOW_DRAW_BOOT_OPTION_NUMBER      (1)
#endif

// Connect drivers on start
#ifndef MEOW_BM_CONNECT_ALL
  #define MEOW_BM_CONNECT_ALL               (0)
#endif

// Refresh BootOptions on start
#ifndef MEOW_BM_REFRESH_BOOT_OPTIONS
  #define MEOW_BM_REFRESH_BOOT_OPTIONS      (0)
#endif

// Use EfiBootManagerBoot instead of BootNext
#ifndef MEOW_BM_BOOT
  #define MEOW_BM_BOOT                      (0)
#endif

// Use DevicePathLib to get short path
#ifndef MEOW_DEVPATH_LIB
  #define MEOW_DEVPATH_LIB                  (0)
#endif

// Show progressbar instead of countdown timer
#ifndef MEOW_PROGRESSBAR
  #define MEOW_PROGRESSBAR                  (1)
#endif

// Debug console
#ifndef MEOW_DEBUG
  #define MEOW_DEBUG                        (0)
#endif

//

#include <Uefi.h>

#include <Guid/GlobalVariable.h>

#include <Protocol/HiiFont.h>
#include <Protocol/LoadedImage.h>
#if MEOW_DEVPATH_LIB != 1
  #include <Protocol/DevicePathToText.h>
#endif
#include <Protocol/GraphicsOutput.h>

#include <Library/BaseMemoryLib.h>
#if MEOW_DEVPATH_LIB == 1
  #include <Library/DevicePathLib.h>
#endif
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "Activity.h"
#include "MainActivity.h"
#include "MeowFunctions.h"

#if MEOW_DEBUG == 1
  #define MeowLog(s) MeowConsoleLog (s)
#else
  #define MeowLog(s)
#endif

//

extern EFI_HII_FONT_PROTOCOL                *gFontProtocol;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL         *gGraphProtocol;

#if MEOW_DEVPATH_LIB != 1
  extern EFI_DEVICE_PATH_TO_TEXT_PROTOCOL   *gPathConvProtocol;
#endif

#endif // MEOW_H
