
struct _ImageHeader {
    uint32_t ImageSize;
    uint32_t SizeX;
    uint32_t SizeY;
};

uint32_t read_be32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |
           ((uint32_t)p[3]);
}

void GetImageThing(ObjectAny ImageContent, struct _ImageHeader** hdr, unsigned char** raw, uint32_t* ImageSize, uint32_t* SizeX, uint32_t* SizeY)
{
    #ifndef __cplusplus
	*hdr = (struct _ImageHeader*)ImageContent;
    #else
	*hdr = (_ImageHeader*)ImageContent;
    #endif
    #ifndef __cplusplus
	*raw = ImageContent + sizeof(struct _ImageHeader);
    #else
	*raw = (unsigned char*)(ImageContent + sizeof(_ImageHeader));
    #endif

	*ImageSize = read_be32((const uint8_t*)(&((*hdr)->ImageSize)));
	*SizeX = read_be32((const uint8_t*)(&((*hdr)->SizeX)));
	*SizeY = read_be32((const uint8_t*)(&((*hdr)->SizeY)));
}
