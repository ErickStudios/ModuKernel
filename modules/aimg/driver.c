#include "../../library/lib.h"
#include "image.h"

/* dibuja una imagen*/
function
PrintImage(
    struct _ImageHeader Info,
    bytes Raw,
    uint32_t PosX,
    uint32_t PosY
)
{
uint32_t width  = Info.Width;
    uint32_t height = Info.Height;

    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            uint8_t color = Raw[y * width + x]; // si es indexado

            gDS->DrawRectangle(color, PosX + x, PosY + y, 1);
        }
    }
}

/* decodifica una imagen*/
function
DecodeImage(
    bytes Image,
    struct _ImageHeader* Info,
    bytes** Raw,
)
{
    gMS->CoppyMemory(Info, Image, sizeof(struct _ImageHeader));
    bytes PoolRaw = gMS->AllocatePool(Info->ImageSize);
    gMS->CoppyMemory(PoolRaw, Image + sizeof(struct _ImageHeader), Info->ImageSize);
    *Raw = PoolRaw;
}

/* principal */
KernelStatus
ErickMain(
    KernelServices* Services
)
{

}