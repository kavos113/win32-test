#ifndef UNICODE
#define UNICODE
#endif

#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <windows.h>
#include <xaudio2.h>

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

int main()
{
    IXAudio2 *pXAudio = nullptr;
    IXAudio2MasteringVoice *pMasteringVoice = nullptr;
    IXAudio2SourceVoice *pSourceVoice = nullptr;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: CoInitializeEx failed\n");
        return 1;
    }

    hr = XAudio2Create(&pXAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: XAudio2Create failed\n");
        CoUninitialize();
        return 1;
    }

    hr = pXAudio->CreateMasteringVoice(&pMasteringVoice);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: CreateMasteringVoice failed\n");
        pXAudio->Release();
        CoUninitialize();
        return 1;
    }

    WaveData waveData = {};
    hr = LoadWaveFile(L"mywave.wav", &waveData);
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: LoadWaveFile failed\n");
        pMasteringVoice->DestroyVoice();
        pXAudio->Release();
        CoUninitialize();
        return 1;
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
        delete &waveData;
        pMasteringVoice->DestroyVoice();
        pXAudio->Release();
        return 1;
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
        pSourceVoice->DestroyVoice();
        pMasteringVoice->DestroyVoice();
        pXAudio->Release();
        CoUninitialize();
        return 1;
    }

    hr = pSourceVoice->Start();
    if (FAILED(hr))
    {
        OutputDebugString(L"FATAL: Start failed\n");
        pSourceVoice->DestroyVoice();
        pMasteringVoice->DestroyVoice();
        pXAudio->Release();
        CoUninitialize();
        return 1;
    }

    XAUDIO2_VOICE_STATE state = {};
    pSourceVoice->GetState(&state);
    while (state.BuffersQueued > 0)
    {
        Sleep(100);
        pSourceVoice->GetState(&state);
    }

    pSourceVoice->DestroyVoice();
    pMasteringVoice->DestroyVoice();
    pXAudio->Release();

    CoUninitialize();

    return 0;
}