#ifndef UNICODE
#define UNICODE
#endif

#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <thread>
#include <windows.h>
#include <xaudio2.h>
#include <x3daudio.h>

struct WaveData
{
    WAVEFORMATEX waveFormat;
    char* data;
    DWORD dataSize;

    ~WaveData()
    {
        delete[] data;
    }
};

HRESULT LoadWaveFile(const std::wstring& filePath, WaveData* out)
{
    if (out)
    {
        out->data = nullptr;
        out->dataSize = 0;
    }
    else
    {
        OutputDebugString(L"LoadWaveFile: out is nullptr\n");
        return E_INVALIDARG;
    }

    HMMIO hmmio = nullptr;
    MMCKINFO chunkInfo = {};
    MMCKINFO riffChunkInfo = {};

    hmmio = mmioOpen(
        const_cast<LPWSTR>(filePath.c_str()),
        nullptr,
        MMIO_READ
        );
    if (!hmmio)
    {
        OutputDebugString(L"fatal: mmioOpen failed\n");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    riffChunkInfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');

    MMRESULT ret = mmioDescend(
        hmmio,
        &riffChunkInfo,
        nullptr,
        MMIO_FINDRIFF
        );
    if (ret != MMSYSERR_NOERROR)
    {
        OutputDebugString(L"fatal: mmioDescend to RIFF chunk failed\n");
        mmioClose(hmmio, MMIO_FHOPEN);
        return HRESULT_FROM_WIN32(ret);
    }

    chunkInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
    ret = mmioDescend(
        hmmio,
        &chunkInfo,
        &riffChunkInfo,
        MMIO_FINDCHUNK
        );
    if (ret != MMSYSERR_NOERROR)
    {
        OutputDebugString(L"fatal: mmioDescend to fmt chunk failed\n");
        mmioClose(hmmio, MMIO_FHOPEN);
        return HRESULT_FROM_WIN32(ret);
    }

    DWORD readSize = mmioRead(
        hmmio,
        reinterpret_cast<HPSTR>(&out->waveFormat),
        sizeof(WAVEFORMATEX)
        );
    if (readSize != sizeof(WAVEFORMATEX))
    {
        OutputDebugString(L"fatal: mmioRead for fmt chunk failed\n");
        mmioClose(hmmio, MMIO_FHOPEN);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (out->waveFormat.wFormatTag != WAVE_FORMAT_PCM)
    {
        OutputDebugString(L"fatal: not PCM format\n");
        mmioClose(hmmio, MMIO_FHOPEN);
        return E_FAIL;
    }

    ret = mmioAscend(
        hmmio,
        &chunkInfo,
        0
        );
    if (ret != MMSYSERR_NOERROR)
    {
        OutputDebugString(L"fatal: mmioAscend from fmt chunk failed\n");
        mmioClose(hmmio, MMIO_FHOPEN);
        return HRESULT_FROM_WIN32(ret);
    }

    chunkInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
    ret = mmioDescend(
        hmmio,
        &chunkInfo,
        &riffChunkInfo,
        MMIO_FINDCHUNK
        );
    if (ret != MMSYSERR_NOERROR)
    {
        OutputDebugString(L"fatal: mmioDescend to data chunk failed\n");
        mmioClose(hmmio, MMIO_FHOPEN);
        return HRESULT_FROM_WIN32(ret);
    }

    out->dataSize = chunkInfo.cksize;

    out->data = new char[out->dataSize];
    readSize = mmioRead(
        hmmio,
        out->data,
        chunkInfo.cksize
        );
    if (readSize != chunkInfo.cksize)
    {
        OutputDebugString(L"fatal: mmioRead for data chunk failed\n");
        mmioClose(hmmio, MMIO_FHOPEN);
        delete[] out->data;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    mmioClose(hmmio, MMIO_FHOPEN);

    return S_OK;
}

#define INPUT_CHANNELS 2
#define OUTPUT_CHANNELS 8

class Audio
{
public:
    HRESULT Init();
    HRESULT Play();
    HRESULT PlayBGM();
private:
    IXAudio2 *pXAudio = nullptr;
    IXAudio2MasteringVoice *pMasteringVoice = nullptr;
    IXAudio2SourceVoice *pSourceVoice = nullptr;
    WaveData waveData = {};

    IXAudio2SourceVoice *pBGMSrcVoice = nullptr;
    WaveData bgmWaveData = {};

    X3DAUDIO_HANDLE x3dInstance = {};
    DWORD channelMask = 0;
    UINT32 channelCount = 0;
    X3DAUDIO_LISTENER listener = {};
    X3DAUDIO_EMITTER emitter = {};
    X3DAUDIO_DSP_SETTINGS dspSettings = {};
    X3DAUDIO_CONE emitterCone = {};

    DirectX::XMFLOAT3 listenerPos = {};
    DirectX::XMFLOAT3 emitterPos = {};

    FLOAT32 matrixCoefficients[INPUT_CHANNELS * OUTPUT_CHANNELS] = {};

    HRESULT Update();
    void CleanUp();
};

HRESULT Audio::Init()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: CoInitializeEx failed\n");
        CleanUp();
        return hr;
    }

    hr = XAudio2Create(&pXAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: XAudio2Create failed\n");
        CleanUp();
        return hr;
    }

    XAUDIO2_DEBUG_CONFIGURATION debugConfig = {};
    debugConfig.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
    debugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
    pXAudio->SetDebugConfiguration(&debugConfig);

    hr = pXAudio->CreateMasteringVoice(&pMasteringVoice);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: CreateMasteringVoice failed\n");
        CleanUp();
        return hr;
    }

    XAUDIO2_VOICE_DETAILS voiceDetails = {};
    pMasteringVoice->GetVoiceDetails(&voiceDetails);

    if (voiceDetails.InputChannels > OUTPUT_CHANNELS)
    {
        OutputDebugString(L"fatal: invalid number of output channels\n");
        CleanUp();
        return E_FAIL;
    }
    channelCount = voiceDetails.InputChannels;
    hr = pMasteringVoice->GetChannelMask(&channelMask);
    if (FAILED(hr))
    {
        OutputDebugString(L"fatal: GetChannelMask failed\n");
        CleanUp();
        return hr;
    }

    hr = X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3dInstance);
    if (FAILED(hr))
    {
        OutputDebugString(L"fatal: X3DAudioInitialize failed\n");
        CleanUp();
        return hr;
    }

    listenerPos = { 0.0f, 0.0f, 0.0f };
    emitterPos = { 3.0f, 0.0f, 10.0f };
    listener.Position.x = listenerPos.x;
    listener.Position.y = listenerPos.y;
    listener.Position.z = listenerPos.z;
    listener.OrientFront = { 0.0f, 0.0f, 1.0f };
    listener.OrientTop = { 0.0f, 1.0f, 0.0f };
    listener.Velocity = { 0.0f, 0.0f, 0.0f };
    emitter.Position.x = emitterPos.x;
    emitter.Position.y = emitterPos.y;
    emitter.Position.z = emitterPos.z;
    emitter.OrientFront = { 0.0f, 0.0f, 1.0f };
    emitter.OrientTop = { 0.0f, 1.0f, 0.0f };
    emitter.Velocity = { 0.0f, 0.0f, -4.0f };
    emitter.InnerRadius = 2.0f;
    emitter.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;
    emitter.CurveDistanceScaler = 4.0f;
    emitter.DopplerScaler = 1.0f;
    emitter.ChannelCount = INPUT_CHANNELS;
    emitter.ChannelRadius = 1.0f;
    FLOAT32 *emitterAzimuths= new FLOAT32[INPUT_CHANNELS];
    emitterAzimuths[0] = 0.0f;
    emitterAzimuths[1] = X3DAUDIO_2PI;
    emitter.pChannelAzimuths = emitterAzimuths;

    dspSettings.SrcChannelCount = INPUT_CHANNELS;
    dspSettings.DstChannelCount = channelCount;
    dspSettings.pMatrixCoefficients = matrixCoefficients;

    hr = LoadWaveFile(L"mywave.wav", &waveData);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: LoadWaveFile failed\n");
        CleanUp();
        return hr;
    }

    WAVEFORMATEX waveFormat;
    memcpy(&waveFormat, &waveData.waveFormat, sizeof(WAVEFORMATEX));
    waveFormat.wBitsPerSample = waveFormat.nBlockAlign * 8 / waveFormat.nChannels;

    hr = pXAudio->CreateSourceVoice(
        &pSourceVoice,
        &waveFormat
        );
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: CreateSourceVoice failed\n");
        CleanUp();
        return hr;
    }

    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = waveData.dataSize;
    buffer.pAudioData = reinterpret_cast<BYTE*>(waveData.data);
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = 0;

    hr = pSourceVoice->SubmitSourceBuffer(&buffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: SubmitSourceBuffer failed\n");
        CleanUp();
        return hr;
    }

    hr = pSourceVoice->Start();
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: Start failed\n");
        CleanUp();
        return hr;
    }

    hr = LoadWaveFile(L"mywave2.wav", &bgmWaveData);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: LoadWaveFile failed\n");
        CleanUp();
        return hr;
    }

    WAVEFORMATEX bgmWaveFormat;
    memcpy(&bgmWaveFormat, &bgmWaveData.waveFormat, sizeof(WAVEFORMATEX));
    bgmWaveFormat.wBitsPerSample = bgmWaveFormat.nBlockAlign * 8 / bgmWaveFormat.nChannels;

    hr = pXAudio->CreateSourceVoice(
        &pBGMSrcVoice,
        &bgmWaveFormat
        );
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: CreateSourceVoice failed\n");
        CleanUp();
        return hr;
    }

    XAUDIO2_BUFFER bgmBuffer = {};
    bgmBuffer.AudioBytes = bgmWaveData.dataSize;
    bgmBuffer.pAudioData = reinterpret_cast<BYTE*>(bgmWaveData.data);
    bgmBuffer.Flags = XAUDIO2_END_OF_STREAM;
    bgmBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;

    hr = pBGMSrcVoice->SubmitSourceBuffer(&bgmBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: SubmitSourceBuffer failed\n");
        CleanUp();
        return hr;
    }

    hr = pBGMSrcVoice->Start();
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: Start failed\n");
        CleanUp();
        return hr;
    }

    return S_OK;
}

HRESULT Audio::Update()
{
    OutputDebugString(std::to_wstring(emitterPos.z).c_str());
    OutputDebugString(L"\n");
    emitterPos.z -= 0.1f;
    if (emitterPos.z < -10.0f)
    {
        emitterPos.z = 10.0f;
    }
    emitter.Position.x = emitterPos.x;
    emitter.Position.y = emitterPos.y;
    emitter.Position.z = emitterPos.z;

    X3DAudioCalculate(
        x3dInstance,
        &listener,
        &emitter,
        X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER,
        &dspSettings
        );

    HRESULT hr = pSourceVoice->SetFrequencyRatio(dspSettings.DopplerFactor);
    if (FAILED(hr))
    {
        OutputDebugString(L"warning: SetFrequencyRatio failed\n");
        CleanUp();
        return hr;
    }

    hr = pSourceVoice->SetOutputMatrix(
        pMasteringVoice,
        INPUT_CHANNELS,
        channelCount,
        matrixCoefficients
        );
    if (FAILED(hr))
    {
        OutputDebugString(L"warning: SetOutputMatrix failed\n");
        CleanUp();
        return hr;
    }

    return S_OK;
}

void Audio::CleanUp()
{
    if (pSourceVoice)
    {
        pSourceVoice->DestroyVoice();
        pSourceVoice = nullptr;
    }

    if (pMasteringVoice)
    {
        pMasteringVoice->DestroyVoice();
        pMasteringVoice = nullptr;
    }

    if (pXAudio)
    {
        pXAudio->Release();
        pXAudio = nullptr;
    }

    CoUninitialize();
}

HRESULT Audio::Play()
{
    XAUDIO2_VOICE_STATE state = {};
    pSourceVoice->GetState(&state);
    while (state.BuffersQueued > 0)
    {
        Sleep(100);
        Update();
        pSourceVoice->GetState(&state);
    }

    return S_OK;
}

HRESULT Audio::PlayBGM()
{
    XAUDIO2_VOICE_STATE state = {};
    pBGMSrcVoice->GetState(&state);
    while (state.BuffersQueued > 0)
    {
        Sleep(100);
        pBGMSrcVoice->GetState(&state);
    }

    return S_OK;
}

int main()
{
    Audio audio;
    HRESULT hr = audio.Init();
    if (FAILED(hr))
    {
        return 1;
    }

    std::thread t(&Audio::PlayBGM, &audio);

    hr = audio.Play();
    if (FAILED(hr))
    {
        return 1;
    }

    t.join();

    return 0;
}
