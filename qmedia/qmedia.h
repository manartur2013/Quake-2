#include "../qcommon/qcommon.h"
#include "../client/media.h"

compressed_image_t *QM_BitmapToPNG(byte *bitmap, int width, int height);
compressed_image_t *QM_BitmapToJPEG(byte* bitmap, int width, int height);

extern mimport_t mi;