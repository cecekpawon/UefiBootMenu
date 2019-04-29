#ifndef __MEOW_MAIN_ACTIVITY_H__
#define __MEOW_MAIN_ACTIVITY_H__

#include "Activity.h"

#include <Protocol/HiiFont.h>

#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootManagerLib.h>

typedef struct {
  ACTIVITY                        Activity;
  CHAR16                          *StaticTexts[8];
  EFI_BOOT_MANAGER_LOAD_OPTION    *BootOptions;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *BgBuffer;
  INT8                            TimerCount;
  UINT32                          BootAutoBaseY;
  UINT32                          BootFailedBaseY;
  UINT32                          BootOptionBaseY;
  UINT32                          BootPathBaseY;
  UINT32                          Width;
  UINT32                          Height;
  UINTN                           Selection;
  UINTN                           BootOptionCount;
  BOOLEAN                         IfShowBootFailed;
} MAIN_ACTIVITY;


EFI_STATUS
NewMainActivity (
  IN   UINT32     Width,
  IN   UINT32     Height,
  OUT  ACTIVITY   **Activity
  );

VOID
MainActivityOnStart (
  IN OUT  ACTIVITY  *Activity
  );

UINT32
MainActivityOnEvent (
  IN OUT  ACTIVITY  *Activity
  );

#endif
