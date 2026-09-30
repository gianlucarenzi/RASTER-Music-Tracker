#include "PlatformTypes.h"
#include "SongContainer.h"

#include "GuiHelpers.h"
#include "RuntimeException.h"

CSongContainer::CSongContainer(CSong& song)
{
    m_song = &song;
    m_pokeyStreamReady = false;

    if (m_song->GetPlayMode() != PLAY_STOP) {
        ThrowRuntimeException("Song is not in play mode \"STOPPED\".");
    }
}

CSongContainer::~CSongContainer()
{
    // The dump switched all channels off: FinishedRecording() switches them
    // back on, resets the tracker driver and frees the stream buffer
    if (m_pokeyStreamReady) {
        m_pokeyStream.FinishedRecording();
    }
}

CSong& CSongContainer::GetSong()
{
    return *m_song;
}

const CPokeyStream& CSongContainer::GetPokeyStream()
{
    return GetModifiablePokeyStream();
}

CPokeyStream& CSongContainer::GetModifiablePokeyStream()
{
    if (!m_pokeyStreamReady) {
        SendInfoMessage("Generating stream data ...");
        m_song->DumpSongToPokeyStream(m_pokeyStream, PLAY_SONG, 0, 0);
        m_pokeyStreamReady = true;
    }
    return m_pokeyStream;
}