#ifndef MEOW_MAIN_ACTIVITY_H
#define MEOW_MAIN_ACTIVITY_H

#include "Meow.h"

#define DIM_ITEM_SEL_H_OFFSET             (20)
#define DIM_ITEM_SEL_HEIGHT               (32)
#define DIM_ITEM_TEXT_X_OFFSET            (40)
#define DIM_ITEM_TEXT_Y_OFFSET            (8)
#define DIM_PARA_SEP                      (16)
#define DIM_TITLE_Y_OFFSET                (40)

#define TIMER_COUNT                       (10)
#define TIMER_TIMEOUT                     (TIMER_COUNT + 1)

typedef enum {
  StaticTextsIndexMainTitle,
  StaticTextsIndexHelpTitle,
  StaticTextsIndexListTitle,
  StaticTextsIndexBootFailed,
  StaticTextsIndexAutoBoot,
  StaticTextsIndexSeconds,
} StaticTextsIndex;

typedef struct {
  MEOW_ACTIVITY                   Activity;
  //CHAR16                          *StaticTexts[8];
  CHAR16                          **StaticTexts;
  EFI_BOOT_MANAGER_LOAD_OPTION    *BootOptions;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *BgBuffer;
  CHAR16                          *PathBuffer;
  INT8                            TimerCount;
  UINT32                          BootAutoBaseY;
  //UINT32                          BootFailedBaseY;
  UINT32                          BootOptionBaseY;
  UINT32                          BootPathBaseY;
  UINT32                          Width;
  UINT32                          Height;
  UINTN                           Selection;
  UINTN                           BootOptionCount;
  UINTN                           PathRowCount;
  UINTN                           CharRowCount;
  //BOOLEAN                         IfShowBootFailed;
} MEOW_MAIN_ACTIVITY;

/**
  Init MainActivity.
**/
EFI_STATUS
NewMainActivity (
  IN   UINT32         Width,
  IN   UINT32         Height,
  OUT  MEOW_ACTIVITY  **Activity
  );

/**
  Start MainActivity.
**/
VOID
MainActivityOnStart (
  IN OUT  MEOW_ACTIVITY   *Activity,
  IN      BOOLEAN         Reinit
  );

/**
  Ret != 0 exit Loop.
**/
MEOW_EVENT_RETURN
MainActivityOnEvent (
  IN OUT  MEOW_ACTIVITY  *Activity
  );

/**
  Free Activity resources.
**/
VOID
FreeActivity (
  IN OUT  MEOW_ACTIVITY   *Activity,
  IN      BOOLEAN         Reinit
  );

#endif // MEOW_MAIN_ACTIVITY_H
