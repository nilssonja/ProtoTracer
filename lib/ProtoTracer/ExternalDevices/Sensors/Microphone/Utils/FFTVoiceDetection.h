/**
 * @file FFTVoiceDetection.h
 * @brief Declares the FFTVoiceDetection template class for real-time viseme detection based on FFT data.
 *
 * This file defines the FFTVoiceDetection class, which extends the Viseme class to provide functionality
 * for detecting mouth shapes (visemes) based on formant frequencies extracted from FFT analysis of voice signals.
 *
 * @date 22/12/2024
 * @author Coela Can't
 */

#pragma once

#include "../../../../Renderer/Utils/IndexGroup.h" // Include for utility structures.
#include "../../../../Renderer/Utils/Triangle2D.h" // Include for 2D triangle utilities.
#include "../../../../Utils/Math/Vector2D.h" // Include for 2D vector utilities.

/**
 * @class Viseme
 * @brief Defines the available mouth shapes (visemes).
 */
class Viseme {
public:
    /**
     * @enum MouthShape
     * @brief Enumerates the possible mouth shapes for viseme detection.
     */
    enum MouthShape {
        EE, ///< Mouth shape corresponding to the "EE" sound.
        AE, ///< Mouth shape corresponding to the "AE" sound.
        UH, ///< Mouth shape corresponding to the "UH" sound.
        AR, ///< Mouth shape corresponding to the "AR" sound.
        ER, ///< Mouth shape corresponding to the "ER" sound.
        AH, ///< Mouth shape corresponding to the "AH" sound.
        OO, ///< Mouth shape corresponding to the "OO" sound.
        SS  ///< Mouth shape corresponding to the "SS" sound (optional).
    };
};

/**
 * @class FFTVoiceDetection
 * @brief Detects visemes based on FFT voice analysis.
 *
 * The FFTVoiceDetection class uses formant frequencies (F1 and F2) derived from FFT peaks
 * to detect and assign weights to various mouth shapes (visemes). The strongest spectral peak in
 * each formant's range is tracked over time, and the weight is shared between the two visemes
 * nearest to it so the mouth glides between shapes rather than snapping.
 *
 * @tparam peakCount The number of peaks to analyze in the FFT data.
 */
template <size_t peakCount>
class FFTVoiceDetection : public Viseme {
private:
    static const uint8_t visemeCount = 7; ///< Number of supported visemes.

    // Formant frequency coordinates (F1, F2) in Hz for each viseme, the midpoints of readings taken while holding
    // each vowel inside the head with MICDEBUG enabled. Recalibrate the same way for a different wearer or microphone.
    Vector2D visEE = Vector2D(191.0f, 2234.0f); ///< Coordinates for "EE", as in "see".
    Vector2D visAE = Vector2D(585.0f, 1606.0f); ///< Coordinates for "AE", as in "cat".
    Vector2D visUH = Vector2D(536.0f, 803.0f); ///< Coordinates for "UH", as in "cup".
    Vector2D visAR = Vector2D(460.0f, 768.0f); ///< Coordinates for "AR", as in "or".
    Vector2D visER = Vector2D(400.0f, 1309.0f); ///< Coordinates for "ER", as in "bird".
    Vector2D visAH = Vector2D(541.0f, 1252.0f); ///< Coordinates for "AH", as in "father".
    Vector2D visOO = Vector2D(222.0f, 545.0f); ///< Coordinates for "OO", as in "too".

    Vector2D* coordinates[visemeCount] = { &visEE, &visAE, &visUH, &visAR, &visER, &visAH, &visOO }; ///< Array of viseme coordinates.

    // Viseme probabilities.
    float visRatioEE = 0.0f; ///< Probability for "EE".
    float visRatioAE = 0.0f; ///< Probability for "AE".
    float visRatioUH = 0.0f; ///< Probability for "UH".
    float visRatioAR = 0.0f; ///< Probability for "AR".
    float visRatioER = 0.0f; ///< Probability for "ER".
    float visRatioAH = 0.0f; ///< Probability for "AH".
    float visRatioOO = 0.0f; ///< Probability for "OO".

    float* visRatios[visemeCount] = { &visRatioEE, &visRatioAE, &visRatioUH, &visRatioAR, &visRatioER, &visRatioAH, &visRatioOO }; ///< Array of viseme probabilities.

    const float f1MinHz = 150.0f; ///< Lowest frequency searched for the first formant, EE sits at the bottom of the range.
    const float f1MaxHz = 1200.0f; ///< Highest frequency searched for the first formant.
    const float f2MaxHz = 3600.0f; ///< Highest frequency searched for the second formant.
    const float formantGapHz = 300.0f; ///< Minimum spacing between the two formants.
    const float f3MinF1Hz = 450.0f; ///< With F1 above this...
    const float f3MinF2Hz = 2250.0f; ///< ...a second peak above this is the third formant of a back vowel whose first two merged, no vowel has both.
    const float minStrength = 0.08f; ///< Weakest peak accepted as a formant (0.0 - 1.0).
    const float prominence = 0.04f; ///< Rise out of the preceding valley needed for a peak to count as the next formant.
    const float formantSmoothing = 0.5f; ///< Share of each new formant reading blended in per update.
    const float visemeSmoothing = 0.6f; ///< Share of each new viseme weight blended in per update.

    const float noiseSmoothing = 0.1f; ///< Share of each silent spectrum blended into the noise floor.

    float noiseFloor[peakCount] = {}; ///< Spectrum of the background noise, learned while nobody is speaking.
    float peakDensity[peakCount]; ///< Smoothed spectral envelope.

    float f1 = 0.0f; ///< Formant frequency F1, smoothed over time.
    float f2 = 0.0f; ///< Formant frequency F2, smoothed over time.
    bool formantsValid = false; ///< Whether f1/f2 hold a previous reading to smooth from.

    float threshold = 400.0f; ///< Threshold for formant calculations.

    /**
     * @brief Finds the strongest local maximum of the spectral envelope within a range of bins.
     *
     * @param binLow First bin of the range (inclusive).
     * @param binHigh Last bin of the range (inclusive).
     * @param valleyBin Bin of the previous formant, peaks must rise by the prominence out of the valley since it. -1 for none.
     * @param minimum Weakest peak to accept.
     * @param lowestRatio Take the lowest peak at least this share of the strongest, 1.0 for the strongest itself.
     * @param bin Output, position of the peak in fractional bins.
     * @param strength Output, height of the peak.
     * @return True if a peak was found.
     */
    bool FindPeak(int16_t binLow, int16_t binHigh, int16_t valleyBin, float minimum, float lowestRatio, float& bin, float& strength);

    /**
     * @brief Calculates formant frequencies (F1 and F2) from FFT peaks.
     *
     * @param peaks Array of FFT peak values.
     * @param bandwidth Number of bins either side of each bin averaged into the spectral envelope.
     * @param binHz Width of one bin in Hz.
     * @return True if at least one formant was found.
     */
    bool CalculateFormants(float* peaks, uint8_t bandwidth, float binHz);

    /**
     * @brief Calculates the viseme weights based on formants.
     *
     * @param formantsFound False if this update found no formants, in which case all weights fall to zero.
     */
    void CalculateVisemeGroup(bool formantsFound);

public:
    /**
     * @brief Constructs a new FFTVoiceDetection instance.
     */
    FFTVoiceDetection() {}

    /**
     * @brief Sets the threshold for formant calculations.
     *
     * @param threshold The new threshold value.
     */
    void SetThreshold(float threshold);

    /**
     * @brief Retrieves the probability of a specific viseme.
     *
     * @param viseme The viseme to query.
     * @return The probability of the specified viseme (0.0 - 1.0).
     */
    float GetViseme(MouthShape viseme);

    /**
     * @brief Prints the probabilities of all visemes to the serial console.
     */
    void PrintVisemes();

    /**
     * @brief Retrieves the first formant frequency in Hz, for calibrating the viseme coordinates.
     */
    float GetF1();

    /**
     * @brief Retrieves the second formant frequency in Hz, for calibrating the viseme coordinates.
     */
    float GetF2();

    /**
     * @brief Retrieves the name of the viseme with the highest weight.
     * @return Two letter name, or "--" if no viseme is active.
     */
    const char* GetDominantVisemeName();

    /**
     * @brief Resets all viseme probabilities to zero.
     */
    void ResetVisemes();

    /**
     * @brief Learns the background noise from a spectrum captured while nobody is speaking.
     *
     * The learned floor is subtracted from the spectrum given to Update(), which removes fans and
     * hum without fading out a held vowel the way a filter that adapts during speech would.
     * If this is never called the floor stays at zero and Update() uses its input as given.
     *
     * @param spectrum Array of FFT values in the same form as given to Update().
     */
    void UpdateNoiseFloor(float* spectrum);

    /**
     * @brief Updates the viseme probabilities based on new FFT data.
     *
     * @param peaks Array of FFT peak values.
     * @param maxFrequency Maximum frequency represented in the FFT data.
     */
    void Update(float* peaks, float maxFrequency);
};

#include "FFTVoiceDetection.tpp" // Include the template implementation.
