#ifndef MEOW_MAIN_ACTIVITY_H
#define MEOW_MAIN_ACTIVITY_H

#include "Activity.h"

#include <Protocol/HiiFont.h>

#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#define DIM_ITEM_SEL_H_OFFSET     (40)
#define DIM_ITEM_SEL_HEIGHT       (32)
#define DIM_ITEM_TEXT_X_OFFSET    (80)
#define DIM_ITEM_TEXT_Y_OFFSET    (8)
#define DIM_PARA_SEP              (16)
#define DIM_TITLE_Y_OFFSET        (40)

#define TIMER_COUNT               (10)

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

UINTN
MainActivityOnEvent (
  IN OUT  ACTIVITY  *Activity
  );

VOID
FreeActivity (
  IN OUT  ACTIVITY  *Activity
  );

#endif // MEOW_MAIN_ACTIVITY_H
