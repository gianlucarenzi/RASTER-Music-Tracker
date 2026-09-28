// MfcAudio.cpp - multimedia timers and DirectSound output for the builds
// without MFC (see MfcTypes.h)
//
// - timeSetEvent()/timeKillEvent(): one thread per timer. CSongTimer creates
//   a new timer from inside every tick (CSong::TimerRoutine -> ChangeTimer),
//   as on Windows; here a timer created from a tick of a timer with the same
//   callback starts at that timer's next deadline, so the song keeps its
//   tempo instead of drifting by the time the callback takes.
// - IDirectSoundBuffer: the secondary buffer of PokeyRederer.cpp, a ring
//   played by PortAudio (RMT_HAVE_PORTAUDIO) with real cursors.

#include "PlatformTypes.h"

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#ifdef RMT_HAVE_PORTAUDIO
#include <portaudio.h>
#endif

// ---------------------------------------------------------------------------
// timers
// ---------------------------------------------------------------------------

namespace {

using Clock = std::chrono::steady_clock;

struct MmTimer {
    std::thread thread;
    std::atomic<bool> stop{ false };
};

std::mutex s_timersLock;
std::map<UINT, std::shared_ptr<MmTimer>> s_timers;
UINT s_nextTimerId = 1;

// set on a timer thread while its callback runs
thread_local LPTIMECALLBACK t_callback = nullptr;
thread_local DWORD_PTR t_user = 0;
thread_local Clock::time_point t_deadline;

} // namespace

UINT timeSetEvent(UINT delay, UINT, LPTIMECALLBACK callback, DWORD_PTR user, UINT flags)
{
    if (!callback || !delay) return 0;
    Clock::time_point first = Clock::now() + std::chrono::milliseconds(delay);
    if (t_callback == callback && t_user == user)
        first = t_deadline + std::chrono::milliseconds(delay);    // rescheduled from its own tick

    auto timer = std::make_shared<MmTimer>();
    UINT id;
    {
        std::lock_guard<std::mutex> lock(s_timersLock);
        id = s_nextTimerId++;
        if (!s_nextTimerId) s_nextTimerId = 1;
        s_timers[id] = timer;
    }
    timer->thread = std::thread([timer, id, delay, callback, user, flags, first] {
        Clock::time_point deadline = first;
        while (!timer->stop) {
            std::this_thread::sleep_until(deadline);
            if (timer->stop) break;
            t_callback = callback;
            t_user = user;
            t_deadline = deadline;
            callback(id, 0, user, 0, 0);
            t_callback = nullptr;
            if (!(flags & TIME_PERIODIC)) break;
            deadline += std::chrono::milliseconds(delay);
            if (deadline < Clock::now() - std::chrono::milliseconds(200))
                deadline = Clock::now();                            // too late (debugger, suspend): do not catch up
        }
    });
    return id;
}

UINT timeKillEvent(UINT id)
{
    std::shared_ptr<MmTimer> timer;
    {
        std::lock_guard<std::mutex> lock(s_timersLock);
        auto it = s_timers.find(id);
        if (it == s_timers.end()) return 1;                          // MMSYSERR_ERROR
        timer = it->second;
        s_timers.erase(it);
    }
    timer->stop = true;
    if (timer->thread.get_id() == std::this_thread::get_id())
        timer->thread.detach();                                       // killed from its own callback
    else if (timer->thread.joinable())
        timer->thread.join();
    return 0;
}

// ---------------------------------------------------------------------------
// DirectSound buffer
// ---------------------------------------------------------------------------

bool g_rmtAudioOutput = true;

struct RmtAudioStream {
#ifdef RMT_HAVE_PORTAUDIO
    PaStream* stream = nullptr;
#endif
    DWORD safety = 0;                   // bytes between the play and the write cursor
    // test device (RMT_AUDIO_DUMP=file.wav): a thread takes the ring in real
    // time, like a sound card, and writes what it would play
    std::thread dumpThread;
    std::atomic<bool> dumpStop{ false };
    FILE* dumpFile = nullptr;
    uint32_t dumpBytes = 0;
};

// copy what the device plays next out of the ring, advancing the play cursor
static void TakeFromRing(IDirectSoundBuffer* buffer, unsigned char* out, DWORD bytes)
{
    DWORD size = (DWORD)buffer->m_data.size();
    DWORD pos = buffer->m_play;
    while (bytes) {
        DWORD n = std::min(bytes, size - pos);
        std::memcpy(out, buffer->m_data.data() + pos, n);
        out += n;
        bytes -= n;
        pos = (pos + n) % size;
    }
    buffer->m_play = pos;
}

static void WriteWavHeader(FILE* f, const WAVEFORMATEX& fmt, uint32_t dataBytes)
{
    auto put32 = [f](uint32_t v) { std::fwrite(&v, 4, 1, f); };
    auto put16 = [f](uint16_t v) { std::fwrite(&v, 2, 1, f); };
    std::fseek(f, 0, SEEK_SET);
    std::fwrite("RIFF", 1, 4, f); put32(36 + dataBytes);
    std::fwrite("WAVEfmt ", 1, 8, f); put32(16); put16(1); put16(fmt.nChannels);
    put32(fmt.nSamplesPerSec); put32(fmt.nAvgBytesPerSec); put16(fmt.nBlockAlign); put16(fmt.wBitsPerSample);
    std::fwrite("data", 1, 4, f); put32(dataBytes);
}

static RmtAudioStream* StartDumpDevice(IDirectSoundBuffer* buffer, const char* path)
{
    FILE* f = std::fopen(path, "wb");
    if (!f) { std::perror(path); return nullptr; }
    auto* s = new RmtAudioStream;
    s->dumpFile = f;
    WriteWavHeader(f, buffer->m_format, 0);
    const WAVEFORMATEX fmt = buffer->m_format;
    s->safety = fmt.nSamplesPerSec / 100 * fmt.nBlockAlign;            // 10 ms
    s->dumpThread = std::thread([s, buffer, fmt] {
        auto start = Clock::now();
        uint64_t framesDone = 0;
        std::vector<unsigned char> chunk;
        while (!s->dumpStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            double t = std::chrono::duration<double>(Clock::now() - start).count();
            uint64_t due = (uint64_t)(t * fmt.nSamplesPerSec);
            if (due <= framesDone) continue;
            chunk.resize((size_t)(due - framesDone) * fmt.nBlockAlign);
            TakeFromRing(buffer, chunk.data(), (DWORD)chunk.size());
            std::fwrite(chunk.data(), 1, chunk.size(), s->dumpFile);
            s->dumpBytes += (uint32_t)chunk.size();
            framesDone = due;
        }
    });
    return s;
}

IDirectSoundBuffer::IDirectSoundBuffer(DWORD bytes, const WAVEFORMATEX* format) : m_data(bytes)
{
    if (format) m_format = *format;
}

IDirectSoundBuffer::~IDirectSoundBuffer()
{
    Stop();
}

HRESULT IDirectSoundBuffer::Unlock(void* p1, DWORD s1, void*, DWORD s2)
{
    if (p1 && !m_data.empty())
        m_cursor = (DWORD)(((unsigned char*)p1 - m_data.data()) + s1 + s2) % (DWORD)m_data.size();
    return DS_OK;
}

#ifdef RMT_HAVE_PORTAUDIO
static int PlayCallback(const void*, void* output, unsigned long frames, const PaStreamCallbackTimeInfo*,
                        PaStreamCallbackFlags, void* user)
{
    auto* buffer = (IDirectSoundBuffer*)user;
    TakeFromRing(buffer, (unsigned char*)output, (DWORD)frames * buffer->m_format.nBlockAlign);
    return paContinue;
}

static bool PortAudioReady()
{
    static int state = 0;               // 0 not tried, 1 ok, -1 failed
    if (!state) {
        state = Pa_Initialize() == paNoError ? 1 : -1;
        if (state > 0) std::atexit([] { Pa_Terminate(); });
    }
    return state > 0;
}
#endif

HRESULT IDirectSoundBuffer::Play(DWORD, DWORD, DWORD)
{
    const char* dump = std::getenv("RMT_AUDIO_DUMP");
    if (g_rmtAudioOutput && !m_stream && !m_data.empty() && m_format.nBlockAlign && dump && *dump) {
        m_play = 0;
        m_stream = StartDumpDevice(this, dump);
        return DS_OK;
    }
#ifdef RMT_HAVE_PORTAUDIO
    const WAVEFORMATEX& f = m_format;
    if (!g_rmtAudioOutput || m_stream || m_data.empty() || !f.nChannels || !f.nSamplesPerSec || !PortAudioReady())
        return DS_OK;
    PaSampleFormat format = f.wBitsPerSample == 8 ? paUInt8 : paInt16;
    auto* s = new RmtAudioStream;
    unsigned long framesPerBuffer = f.nSamplesPerSec / 200;         // 5 ms
    if (Pa_OpenDefaultStream(&s->stream, 0, f.nChannels, format, f.nSamplesPerSec, framesPerBuffer,
                             PlayCallback, this) != paNoError) {
        std::fprintf(stderr, "RMT: cannot open the audio output (PortAudio)\n");
        delete s;
        return DS_OK;                   // go on silent
    }
    const PaStreamInfo* info = Pa_GetStreamInfo(s->stream);
    double latency = info ? info->outputLatency : 0.02;
    // DirectSound's write cursor: where it is safe to write, past what the
    // device has already taken
    s->safety = ((DWORD)(latency * f.nSamplesPerSec) + framesPerBuffer) * f.nBlockAlign;
    m_play = 0;
    Pa_StartStream(s->stream);
    m_stream = s;
#endif
    return DS_OK;
}

HRESULT IDirectSoundBuffer::Stop()
{
    if (m_stream && m_stream->dumpFile) {
        m_stream->dumpStop = true;
        m_stream->dumpThread.join();
        WriteWavHeader(m_stream->dumpFile, m_format, m_stream->dumpBytes);
        std::fclose(m_stream->dumpFile);
        delete m_stream;
        m_stream = nullptr;
        return DS_OK;
    }
#ifdef RMT_HAVE_PORTAUDIO
    if (m_stream) {
        Pa_StopStream(m_stream->stream);
        Pa_CloseStream(m_stream->stream);
        delete m_stream;
        m_stream = nullptr;
    }
#endif
    return DS_OK;
}

HRESULT IDirectSoundBuffer::GetCurrentPosition(DWORD* playCursor, DWORD* writeCursor)
{
    DWORD play, write;
    if (m_stream && !m_data.empty()) {
        play = m_play;
        write = (DWORD)((play + m_stream->safety) % m_data.size());
    }
    else {
        play = write = m_cursor;        // instant mode
    }
    if (playCursor) *playCursor = play;
    if (writeCursor) *writeCursor = write;
    return DS_OK;
}
