#pragma once

template <size_t sampleSize>
PeakDetection<sampleSize>::PeakDetection(uint8_t lag, float threshold, float influence) {
    this->lag = lag;
    this->threshold = threshold;
    this->influence = influence;
}

template <size_t sampleSize>
void PeakDetection<sampleSize>::GetStdDev(uint8_t start, uint8_t length, float* data, float& avgRet, float& stdRet) {
    float average = 0.0f;
    float stdDev = 0.0f;

    for (uint8_t i = 0; i < length; i++) {
        average += data[start + i];
    }

    average /= (float)length;

    for (uint8_t i = 0; i < length; i++) {
        stdDev += powf(data[start + i] - average, 2.0f);
    }

    avgRet = average;
    stdRet = sqrtf(stdDev / (float)length);
}

template <size_t sampleSize>
void PeakDetection<sampleSize>::Calculate(float* data, bool* peaks) {
    float maxData = 0.0f;

    for (uint8_t i = 0; i < sampleSize; i++) {
        filData[i] = data[i];
        avg[i] = 0.0f;
        std[i] = 0.0f;
        peaks[i] = false;
        maxData = Mathematics::Max(maxData, data[i]);
    }

    if (maxData <= threshold) return;

    GetStdDev(0, lag, filData, avg[lag - 1], std[lag - 1]);

    for (uint8_t i = lag; i < sampleSize; i++) {
        if (fabs(data[i] - avg[i - 1]) > threshold * std[i - 1]) {
            peaks[i] = data[i] > avg[i - 1];
            filData[i] = influence * data[i] + (1.0f - influence) * filData[i - 1];
        }

        GetStdDev(i - lag + 1, lag, filData, avg[i], std[i]);// trailing window of the influence-filtered data
    }
}
