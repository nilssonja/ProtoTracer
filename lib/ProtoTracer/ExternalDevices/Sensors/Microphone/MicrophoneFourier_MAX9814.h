/**
 * @file MicrophoneFourier_MAX9814.h
 * @brief Extends the MicrophoneFourierBase class for real-time FFT microphone analysis.
 *
 * This file defines the MicrophoneFourier class, which builds upon MicrophoneFourierBase to include
 * real-time sampling and processing of microphone signals using FFT. It incorporates utilities for
 * updating and resetting the microphone processing system.
 * 
 * For the MAX9814 microphone.
 *
 * @date 22/12/2024
 * @author Coela Can't
 */

#pragma once

#include <Arduino.h> // Include for Arduino compatibility.
#include "../../../Utils/Filter/DerivativeFilter.h" // Include for derivative filtering.
#include "../../../Utils/Filter/FFTFilter.h" // Include for FFT filtering.
#include "../../../Utils/Time/TimeStep.h" // Include for time management.
#include "Utils/MicrophoneFourierBase.h" // Include the base class for microphone FFT processing.

/**
 * @class MicrophoneFourier
 * @brief Provides real-time microphone analysis using FFT.
 *
 * The MicrophoneFourier class extends MicrophoneFourierBase to add capabilities for
 * real-time sampling, signal processing, and frequency bin analysis. It enables
 * dynamic updating and resetting of the microphone system.
 */
class MicrophoneFourier : public MicrophoneFourierBase {
private:
    static IntervalTimer sampleTimer; ///< Timer for managing sampling intervals.
    static TimeStep timeStep; ///< Time step utility for controlling updates.

    static uint16_t samples; ///< Number of samples collected in the current cycle.
    static uint16_t samplesStorage; ///< Total number of samples stored.
    static float refreshRate; ///< Refresh rate for processing in Hz.
    static bool samplesReady; ///< Flag indicating if samples are ready for processing.

    static uint16_t frequencyBins[OutputBins]; ///< Array for storing frequency bin data.

    static float window[FFTSize]; ///< Hann window, scaled x2 so tone magnitudes match the unwindowed FFT.
    static float levelDB; ///< RMS level of the last sample window in dB relative to ADC full scale.
    static float peakLevel; ///< Peak of the last sample window as a fraction of ADC full scale (0.0 - 1.0).
    static float speechBandRatio; ///< Fraction of the last window's energy within the speech band (0.0 - 1.0).
    static uint32_t updateCount; ///< Number of sample windows processed, increments with each new FFT.

    /**
     * @brief Callback function for the sampling timer.
     *
     * This function is triggered at each sampling interval to collect microphone data.
     */
    static void SamplerCallback();

    /**
     * @brief Starts the sampling process using the IntervalTimer.
     */
    static void StartSampler();

public:
    /**
     * @brief Initializes the microphone and FFT system.
     *
     * @param pin The pin connected to the microphone.
     * @param sampleRate The desired sampling rate in Hz.
     * @param minDB Minimum dB level for normalization.
     * @param maxDB Maximum dB level for normalization.
     * @param refreshRate The desired refresh rate for processing (default is 60 Hz).
     */
    static void Initialize(uint8_t pin, uint16_t sampleRate, float minDB, float maxDB, float refreshRate = 60.0f);

    /**
     * @brief Resets the microphone system and clears stored data.
     */
    static void Reset();

    /**
     * @brief Retrieves the RMS level of the last sample window, with the DC bias removed.
     * @return Level in dB relative to ADC full scale (0 dB is a full-scale signal, quieter is more negative).
     */
    static float GetLevelDB();

    /**
     * @brief Retrieves the peak of the last sample window, used to check the microphone gain for clipping.
     * @return Peak as a fraction of ADC full scale (0.0 - 1.0).
     */
    static float GetPeakLevel();

    /**
     * @brief Retrieves how much of the last window's energy was within the speech band (~200Hz - 3.5kHz).
     * @return Ratio of speech band energy to total energy (0.0 - 1.0).
     */
    static float GetSpeechBandRatio();

    /**
     * @brief Retrieves the number of sample windows processed so far.
     *
     * A full window takes longer to sample than a frame takes to render, so callers compare this
     * against their last seen value to tell when the FFT data is new.
     *
     * @return The running count of processed windows.
     */
    static uint32_t GetUpdateCount();

    /**
     * @brief Updates the microphone system, processing new samples and performing FFT.
     */
    static void Update();
};
