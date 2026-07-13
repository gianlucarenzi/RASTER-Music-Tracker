#include "PlatformTypes.h"
#include "LZSSFile.h"
#include "Song.h"

int CLZSSFile::GetFrameSize(const CSong& song) {

    return song.IsStereo() ? 18 : 9;
}
