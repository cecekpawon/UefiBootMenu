#include "Activity.h"

EFI_STATUS
ActivityInitialize (
  IN OUT  ACTIVITY  *Activity,
  IN      UINT32    Width,
  IN      UINT32    Height
  )
{
  Activity->Width         = Width;
  Activity->Height        = Height;
  Activity->CountInvalid  = 0;
  Activity->Buffer        = AllocatePool (sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * Width * Height);

  return (Activity->Buffer != NULL) ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}

VOID
ActivityInvalidate (
  IN OUT  ACTIVITY  *Activity,
  IN      RECT      Rect
  )
{
  UINT8   i;

  //

  // -1 means redraw whole Activity.
  if (Activity->CountInvalid == 0xFF) {
    return;
  }

  // Too many! Just redraw whole Activity.

  if (Activity->CountInvalid >= 64) {
    Activity->CountInvalid = 0xFF;
    return;
  }

  // Check fully collapsed Areas.

  for (i = 0; i < Activity->CountInvalid; i++) {

    RECT  Curr;

    //

    Curr = Activity->Invalids[i];

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
      Activity->Invalids[i] = Rect;
      return;
    }
  }

  Activity->Invalids[Activity->CountInvalid] = Rect;
  Activity->CountInvalid++ ;

  // Ignore partial-collapsed areas for now.
}

STATIC
EFI_STATUS
ActivityRenderArea (
  IN  ACTIVITY                      *Activity,
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphProtocol,
  IN  RECT                          Area
  )
{
  return GraphProtocol->Blt (
                          GraphProtocol,
                          Activity->Buffer,
                          EfiBltBufferToVideo,
                          Area.PosX,
                          Area.PosY,
                          Area.PosX,
                          Area.PosY,
                          Area.Width,
                          Area.Height,
                          Activity->Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                          );
}

EFI_STATUS
ActivityRender (
  IN OUT  ACTIVITY                      *Activity,
  IN      EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphProtocol
  )
{
  EFI_STATUS  Status;
  UINT8       i;

  //

  if (Activity->CountInvalid >= 64) {

    RECT  Rect;

    //

    Rect.PosX   = 0;
    Rect.PosY   = 0;
    Rect.Width  = Activity->Width;
    Rect.Height = Activity->Height;

    return ActivityRenderArea (Activity, GraphProtocol, Rect);
  }

  for (i = 0; i < Activity->CountInvalid; i++) {

    Status = ActivityRenderArea (Activity, GraphProtocol, Activity->Invalids[i]);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Activity->CountInvalid = 0;

  return EFI_SUCCESS;
}
