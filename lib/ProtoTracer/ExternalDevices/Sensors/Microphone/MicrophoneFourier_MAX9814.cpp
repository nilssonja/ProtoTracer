#include "MicrophoneFourier_MAX9814.h"

IntervalTimer MicrophoneFourier::sampleTimer;
TimeStep MicrophoneFourier::timeStep = TimeStep(60);

uint16_t MicrophoneFourier::frequencyBins[];
float MicrophoneFourier::window[];
float MicrophoneFourier::levelDB = -100.0f;
float MicrophoneFourier::peakLevel = 0.0f;
float MicrophoneFourier::speechBandRatio = 0.0f;
uint32_t MicrophoneFourier::updateCount = 0;

uint16_t MicrophoneFourier::samples = 0;
uint16_t MicrophoneFourier::samplesStorage = 0;
float MicrophoneFourier::refreshRate = 60.0f;
bool MicrophoneFourier::samplesReady = false;


void MicrophoneFourier::SamplerCallback() {
    int inputSample = analogRead(pin);

    inputSamp[samples++] = (float)inputSample;
    inputSamp[samples++] = 0.0f;

    inputStorage[samplesStorage++] = inputSample;

    if (samples >= FFTSize * 2) {
        sampleTimer.end();
        samplesReady = true;
    }
}

void MicrophoneFourier::StartSampler() {
    samplesReady = false;
    samples = 0;
    samplesStorage = 0;
    sampleTimer.begin(SamplerCallback, 1000000 / sampleRate);
}

void MicrophoneFourier::Initialize(uint8_t pin, uint16_t sampleRate, float minDB, float maxDB, float refreshRate) {
    MicrophoneFourier::minDB = minDB;
    MicrophoneFourier::maxDB = maxDB;
    MicrophoneFourier::pin = pin;
    MicrophoneFourier::refreshRate = refreshRate;

    pinMode(pin, INPUT);
    analogReadResolution(12);

    MicrophoneFourier::sampleRate = sampleRate;
    MicrophoneFourier::samples = 0;
    MicrophoneFourier::samplesReady = false;

    float windowRange = float(sampleRate) / 2.0f / float(OutputBins);

    timeStep.SetFrequency(refreshRate);

    for (uint8_t i = 0; i < OutputBins; i++) {
        float frequency = (float(i) * windowRange);
        frequencyBins[i] = uint16_t(frequency / float(sampleRate / FFTSize));
    }

    for (uint16_t i = 0; i < FFTSize; i++) {
        window[i] = 1.0f - cosf(2.0f * Mathematics::MPI * float(i) / float(FFTSize - 1));
    }

    StartSampler();
    isInitialized = true;
}

void MicrophoneFourier::Reset() {
    for (int i = 0; i < FFTSize * 2; i++) {
        inputSamp[i] = 0.0f;
    }
}

float MicrophoneFourier::GetLevelDB() {
    return levelDB;
}

float MicrophoneFourier::GetPeakLevel() {
    return peakLevel;
}

float MicrophoneFourier::GetSpeechBandRatio() {
    return speechBandRatio;
}

uint32_t MicrophoneFourier::GetUpdateCount() {
    return updateCount;
}

void MicrophoneFourier::Update() {
    if (!samplesReady) return;// the sampler is still filling the buffer, keep the previous window's results

    const float fullScale = 2048.0f;// half of the 12-bit range, the microphone output idles at mid-rail
    float mean = 0.0f;
    float sumSquares = 0.0f;
    float peak = 0.0f;

    for (uint16_t i = 0; i < FFTSize; i++) mean += inputSamp[i * 2];

    mean /= float(FFTSize);

    for (uint16_t i = 0; i < FFTSize; i++) {
        float sample = inputSamp[i * 2] - mean;// remove the DC bias so it cannot leak into the low bins

        sumSquares += sample * sample;
        peak = Mathematics::Max(peak, fabsf(sample));

        inputSamp[i * 2] = sample * window[i];
    }

    levelDB = 20.0f * log10f(Mathematics::Max(sqrtf(sumSquares / float(FFTSize)), 0.01f) / fullScale);
    peakLevel = Mathematics::Constrain(peak / fullScale, 0.0f, 1.0f);

    fft.Radix2FFT(inputSamp);
    fft.ComplexMagnitude(inputSamp, outputMagn);

    float speechEnergy = 0.0f;
    float totalEnergy = 0.0f;
    uint16_t speechBinL = uint16_t(200.0f * float(FFTSize) / float(sampleRate));
    uint16_t speechBinH = uint16_t(3500.0f * float(FFTSize) / float(sampleRate));

    for (uint16_t i = 1; i < FFTSize / 2; i++) {
        float energy = outputMagn[i] * outputMagn[i];

        totalEnergy += energy;
        if (i >= speechBinL && i <= speechBinH) speechEnergy += energy;
    }

    speechBandRatio = totalEnergy > 0.0f ? speechEnergy / totalEnergy : 0.0f;
    updateCount++;

    float averageMagnitude = 0.0f;

    for (uint8_t i = 0; i < OutputBins - 1; i++) {
        float intensity = 20.0f * log10f(AverageMagnitude(i, i + 1));

        intensity = map(intensity, minDB, maxDB, 0.0f, 1.0f);
        intensity = intensity > 0.0f ? intensity : 0.0f;// below minDB is silence, not a negative to be folded back up by the filters

        outputData[i] = intensity;
        outputDataFilt[i] = fftFilters[i].Filter(intensity);
        if (i % 12 == 0) averageMagnitude = peakFilterRate.Filter(inputStorage[i] / 4096.0f);
    }

    averageMagnitude *= 10.0f;
    threshold = powf(averageMagnitude, 2.0f);
    threshold = threshold > 0.2f ? (threshold * 5.0f > 1.0f ? 1.0f : threshold * 5.0f) : 0.0f;

    Reset();
    StartSampler();
}
