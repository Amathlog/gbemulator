#pragma once

// #include <new_exe/messageService/audioMessageService.h>
#include <RtAudio.h>

namespace GBEmulatorExe
{
    class AudioSystem
    {
    public:
        AudioSystem(unsigned nbChannels = 2, unsigned sampleRate = 44100, unsigned bufferFrames = 256);

        virtual ~AudioSystem();

        bool Initialize();
        bool Shutdown();
        void Enable(bool value);
        bool IsEnabled() const { return m_enabled; }

        unsigned GetSampleRate() const { return m_sampleRate; }

        virtual int RenderCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                                   double streamTime, RtAudioStreamStatus status, void *userData) = 0;

    protected:
        unsigned m_nbChannels = 2;
        unsigned m_sampleRate = 44100;
        unsigned m_bufferFrames = 256;
        // AudioMessageService m_messageService;
        
        bool m_enabled = true;
        bool m_initialized = false;
        RtAudio* m_dac = nullptr;

    };
}