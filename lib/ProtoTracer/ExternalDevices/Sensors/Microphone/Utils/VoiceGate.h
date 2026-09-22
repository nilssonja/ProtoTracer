/**
 * @file VoiceGate.h
 * @brief Declares the VoiceGate class, a noise gate and level normaliser for driving mouth movement from a microphone.
 *
 * A fixed-gain microphone cannot tell the wearer's voice from the room by level alone without also
 * forcing the wearer to shout. This gate tracks the ambient noise floor, opens only for sounds that
 * rise clearly above it for long enough to be speech, and scales the result to the wearer's own
 * speaking level so that normal speech produces a full range of mouth movement.
 */

#pragma once

#include <Arduino.h>
#include "../../../../Utils/Math/Mathematics.h"

/**
 * @class VoiceGate
 * @brief Noise gate with hysteresis, minimum duration and speaking-level normalisation.
 *
 * Update() is called once per microphone sample window (not once per rendered frame).
 */
class VoiceGate {
private:
    static const uint8_t floorWindows = 96; ///< History length for the noise floor, ~3 seconds of sample windows.
    static const uint8_t openWindows = 2; ///< Consecutive loud windows needed to open, rejects clicks and bumps.
    static const uint8_t hangWindows = 4; ///< Windows the gate stays open after the level drops, bridges consonants.

    const float minFloorDB = -72.0f; ///< Lowest noise floor assumed, keeps ADC noise from opening the gate.
    const float hysteresisDB = 6.0f; ///< The gate closes this far below the level that opened it.
    const float minRangeDB = 12.0f; ///< Smallest span between the open threshold and the speaking level.
    const float peakDecayDB = 0.06f; ///< Speaking level decay per window, ~2dB per second.
    const float minSpeechBandRatio = 0.5f; ///< Share of energy that must be in the speech band to open.

    float history[floorWindows]; ///< Recent window levels, the minimum of which is the noise floor.
    uint8_t historyIndex = 0; ///< Next slot of the history to overwrite.
    uint8_t historyCount = 0; ///< Number of valid entries in the history.

    float openMarginDB = 18.0f; ///< How far above the noise floor a sound must be to open the gate.
    float noiseFloorDB = -60.0f; ///< Smoothed ambient noise floor.
    float peakDB = -30.0f; ///< Tracked speaking level, mapped to a fully open mouth.
    float envelope = 0.0f; ///< Normalised loudness while open (0.0 - 1.0).

    uint8_t aboveCount = 0; ///< Consecutive windows above the open threshold.
    uint8_t hangCount = 0; ///< Remaining windows before an open gate closes.
    bool isOpen = false; ///< Current gate state.

public:
    /**
     * @brief Constructs a new VoiceGate instance.
     */
    VoiceGate() {}

    /**
     * @brief Sets how easily the gate opens.
     *
     * @param sensitivity 0.0 needs a sound 30dB above the noise floor, 1.0 only 6dB.
     */
    void SetSensitivity(float sensitivity);

    /**
     * @brief Processes one microphone sample window.
     *
     * @param levelDB RMS level of the window in dB relative to full scale.
     * @param speechBandRatio Share of the window's energy within the speech band (0.0 - 1.0).
     */
    void Update(float levelDB, float speechBandRatio);

    /**
     * @brief Checks if the gate currently considers the wearer to be speaking.
     */
    bool IsOpen();

    /**
     * @brief Retrieves the loudness of the current speech relative to the wearer's speaking level.
     * @return 0.0 while closed, up to 1.0 at the wearer's normal speaking level.
     */
    float GetEnvelope();

    /**
     * @brief Retrieves the tracked ambient noise floor in dB relative to full scale.
     */
    float GetNoiseFloorDB();

    /**
     * @brief Retrieves the level a sound must exceed to open the gate, in dB relative to full scale.
     */
    float GetOpenThresholdDB();

    /**
     * @brief Retrieves the tracked speaking level in dB relative to full scale.
     */
    float GetSpeakingLevelDB();
};
