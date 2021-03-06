#include "Meow.h"

/**
  Init Activity.
**/
EFI_STATUS
ActivityInitialize (
  IN OUT  MEOW_ACTIVITY   *This,
  IN      UINT32          Width,
  IN      UINT32          Height
  )
{
  This->Width         = Width;
  This->Height        = Height;
  This->CountInvalid  = 0;
  This->Buffer        = AllocatePool (sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * Width * Height);

  return (This->Buffer != NULL) ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}

/**
  Invalidate Activity.
**/
VOID
ActivityInvalidate (
  IN OUT  MEOW_ACTIVITY   *This,
  IN      RECT            Rect
  )
{
  UINT8   i;

  //

  // -1 means redraw whole Activity.
  // Too many! Just redraw whole Activity.

  if (This->CountInvalid >= MAX_INVALIDS) {
    return;
  }

  // Check fully collapsed Areas.

  for (i = 0; i < This->CountInvalid; i++) {

    RECT  Curr;

    //

    Curr = This->Invalids[i];

    if ((Curr.PosX <= Rect.PosX)
      && (Curr.PosY <= Rect.PosY)
      && ((Curr.PosX + Curr.Width) >= (Rect.PosX + Rect.Width))
      && ((Curr.PosY + Curr.Height) >= (Rect.PosY + Rect.Height))
      )
    {
      return;
    }

    if ((Curr.PosX > Rect.PosX)
      && (Curr.PosY > Rect.PosY)
      && ((Curr.PosX + Curr.Width) < (Rect.PosX + Rect.Width))
      && ((Curr.PosY + Curr.Height) < (Rect.PosY + Rect.Height))
      )
    {
      This->Invalids[i] = Rect;
      return;
    }
  }

  This->Invalids[This->CountInvalid] = Rect;
  This->CountInvalid++;

  // Ignore partial-collapsed areas for now.
}

/**
  Render Activity area.
**/
STATIC
EFI_STATUS
ActivityRenderArea (
  IN  MEOW_ACTIVITY   *This,
  IN  RECT            Area
  )
{
  return gGraphProtocol->Blt (
                            gGraphProtocol,
                            This->Buffer,
                            EfiBltBufferToVideo,
                            Area.PosX,
                            Area.PosY,
                            Area.PosX,
                            Area.PosY,
                            Area.Width,
                            Area.Height,
                            This->Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                            );
}

/**
  Render Activity.
**/
EFI_STATUS
ActivityRender (
  IN OUT  MEOW_ACTIVITY       *This,
  IN      MEOW_EVENT_RETURN   Ret
  )
{
  EFI_STATUS  Status;

  UINT8       i;

  //

  Status = EFI_SUCCESS;

  if ((This->CountInvalid >= MAX_INVALIDS)
    || (Ret != EventReturnNone)
    )
  {
    RECT  Rect;

    //

    Rect.PosX   = 0;
    Rect.PosY   = 0;
    Rect.Width  = This->Width;
    Rect.Height = This->Height;

    return ActivityRenderArea (This, Rect);
  }

  for (i = 0; i < This->CountInvalid; i++) {

    Status = ActivityRenderArea (This, This->Invalids[i]);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  This->CountInvalid = 0;

  return Status;
}
