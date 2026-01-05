/* modules/teleq/pkg.h */

struct _Telecomunication
{
    char Hdr[4];

    uint8_t From[6];
    uint8_t To[6];

    uint32_t DataSize;
};