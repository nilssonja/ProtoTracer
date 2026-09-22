#pragma once

template <size_t peakCount>
void FFTVoiceDetection<peakCount>::SetThreshold(float threshold) {
    this->threshold = threshold;
}

template <size_t peakCount>
float FFTVoiceDetection<peakCount>::GetViseme(MouthShape viseme) {
    return *visRatios[viseme];
}

template <size_t peakCount>
const char* FFTVoiceDetection<peakCount>::GetDominantVisemeName() {
    static const char* names[visemeCount] = { "EE", "AE", "UH", "AR", "ER", "AH", "OO" };
    float max = 0.0f;
    uint8_t ind = visemeCount;

    for (uint8_t i = 0; i < visemeCount; i++) {
        if (max < *visRatios[i]) {
            max = *visRatios[i];
            ind = i;
        }
    }

    return ind < visemeCount ? names[ind] : "--";
}

template <size_t peakCount>
void FFTVoiceDetection<peakCount>::PrintVisemes() {
    Serial.print(f1);
    Serial.print(',');
    Serial.print(f2);
    Serial.print(',');
    Serial.println(GetDominantVisemeName());
}

template <size_t peakCount>
float FFTVoiceDetection<peakCount>::GetF1() {
    return f1;
}

template <size_t peakCount>
float FFTVoiceDetection<peakCount>::GetF2() {
    return f2;
}

template <size_t peakCount>
void FFTVoiceDetection<peakCount>::ResetVisemes() {
    for (uint8_t i = 0; i < visemeCount; i++) *visRatios[i] = 0.0f;

    formantsValid = false;// the next sound is a new utterance, do not glide to it from the last one
}

template <size_t peakCount>
void FFTVoiceDetection<peakCount>::UpdateNoiseFloor(float* spectrum) {
    for (uint16_t i = 0; i < peakCount; i++) noiseFloor[i] += (spectrum[i] - noiseFloor[i]) * noiseSmoothing;
}

template <size_t peakCount>
void FFTVoiceDetection<peakCount>::Update(float* peaks, float maxFrequency) {
    bool formantsFound = CalculateFormants(peaks, 5, maxFrequency / 2.0f / float(peakCount));

    CalculateVisemeGroup(formantsFound);
}

template <size_t peakCount>
bool FFTVoiceDetection<peakCount>::FindPeak(int16_t binLow, int16_t binHigh, int16_t valleyBin, float minimum, float lowestRatio, float& bin, float& strength) {
    int16_t best = -1;
    int16_t first = -1;

    if (binLow < 1) binLow = 1;
    if (binHigh > int16_t(peakCount) - 2) binHigh = int16_t(peakCount) - 2;
    if (valleyBin < 0 || valleyBin > binLow) valleyBin = binLow;

    strength = minimum;

    // the lowest point passed since valleyBin, a peak must rise out of it or it is only ripple on the slope of the previous peak
    float valley = peakDensity[valleyBin];

    for (int16_t i = valleyBin; i < binLow; i++) valley = Mathematics::Min(valley, peakDensity[i]);

    // a local maximum rather than the largest value, so the slope of a peak outside the range is not mistaken for one inside it
    for (int16_t i = binLow; i <= binHigh; i++) {
        valley = Mathematics::Min(valley, peakDensity[i]);

        if (peakDensity[i] <= peakDensity[i - 1] || peakDensity[i] < peakDensity[i + 1]) continue;
        if (valleyBin < binLow && peakDensity[i] - valley < prominence) continue;

        if (peakDensity[i] > strength) {
            best = i;
            strength = peakDensity[i];
        }
    }

    // a formant is the next resonance up, not the loudest one, so step down to the first peak of comparable strength
    valley = peakDensity[valleyBin];

    for (int16_t i = valleyBin; i < binLow; i++) valley = Mathematics::Min(valley, peakDensity[i]);

    for (int16_t i = binLow; i < best && first < 0; i++) {
        valley = Mathematics::Min(valley, peakDensity[i]);

        if (peakDensity[i] <= peakDensity[i - 1] || peakDensity[i] < peakDensity[i + 1]) continue;
        if (valleyBin < binLow && peakDensity[i] - valley < prominence) continue;

        if (peakDensity[i] >= strength * lowestRatio) first = i;
    }

    if (first >= 0) {
        best = first;
        strength = peakDensity[first];
    }

    if (best < 0) return false;

    // parabolic interpolation through the peak and its neighbours, the bins are wide compared to the spacing of the vowels
    float left = peakDensity[best - 1], right = peakDensity[best + 1];
    float curvature = left - 2.0f * strength + right;

    bin = float(best) + (curvature < 0.0f ? 0.5f * (left - right) / curvature : 0.0f);

    return true;
}

template <size_t peakCount>
bool FFTVoiceDetection<peakCount>::CalculateFormants(float* peaks, uint8_t bandwidth, float binHz) {
    // average each bin with its neighbours to blend the harmonics of the voice into the envelope the formants shape.
    // no wider than this, or the close first and second formants of the back vowels merge into a single peak
    for (int16_t i = 0; i < int16_t(peakCount); i++) {
        float density = 0.0f;
        uint8_t count = 0;

        for (int16_t j = i - int16_t(bandwidth) + 1; j < i + int16_t(bandwidth); j++) {
            if (j < 0 || j >= int16_t(peakCount)) continue;

            density += Mathematics::Max(peaks[j] - noiseFloor[j], 0.0f);
            count++;
        }

        peakDensity[i] = density / float(count);
    }

    float bin1, bin2, strength1, strength2;
    bool found1 = FindPeak(int16_t(f1MinHz / binHz), int16_t(f1MaxHz / binHz), -1, minStrength, 1.0f, bin1, strength1);

    if (!found1) {// no low formant, fall back to the strongest peak anywhere
        found1 = FindPeak(int16_t(f1MinHz / binHz), int16_t(f2MaxHz / binHz), -1, minStrength, 1.0f, bin1, strength1);

        if (!found1) return false;
    }

    // the second formant must stand up against the first, otherwise it is only ripple
    bool found2 = FindPeak(int16_t(bin1 + formantGapHz / binHz), int16_t(f2MaxHz / binHz), int16_t(bin1), Mathematics::Max(minStrength * 0.75f, strength1 * 0.25f), 0.7f, bin2, strength2);

    float newF1 = bin1 * binHz;
    float newF2;

    if (found2 && newF1 > f3MinF1Hz && bin2 * binHz > f3MinF2Hz) found2 = false;

    if (found2) newF2 = bin2 * binHz;
    else if (formantsValid && fabsf(newF1 - f1) < 150.0f) newF2 = f2;// same vowel with a faint second formant, keep it rather than flicker
    else newF2 = newF1 + formantGapHz;// the formants of the back vowels sit close enough to merge into one peak

    // smooth out jitter between updates, but jump straight to a new vowel
    if (formantsValid && fabsf(newF1 - f1) < 300.0f && fabsf(newF2 - f2) < 600.0f) {
        f1 += (newF1 - f1) * formantSmoothing;
        f2 += (newF2 - f2) * formantSmoothing;
    } else {
        f1 = newF1;
        f2 = newF2;
    }

    formantsValid = true;

    return true;
}

template <size_t peakCount>
void FFTVoiceDetection<peakCount>::CalculateVisemeGroup(bool formantsFound){
    float targets[visemeCount] = {};

    if(formantsFound && (f1 > threshold || f2 > threshold)){
        // F2 covers several times the range of F1, scale both so neither decides the match alone
        float f1Range = f1MaxHz - f1MinHz;
        float f2Range = f2MaxHz - f1MinHz;
        uint8_t firstClosest = 0, secondClosest = 0;
        float firstDistance = 1000000.0f, secondDistance = 1000000.0f;//arbitrary large value

        for(uint8_t i = 0; i < visemeCount; i++){//find two smallest values
            float dX = (f1 - coordinates[i]->X) / f1Range;
            float dY = (f2 - coordinates[i]->Y) / f2Range;
            float distance = dX * dX + dY * dY;//squared, so a clearly closer viseme takes most of the weight

            if(distance < firstDistance){
                secondClosest = firstClosest;
                secondDistance = firstDistance;
                firstClosest = i;
                firstDistance = distance;
            }
            else if(distance < secondDistance){
                secondClosest = i;
                secondDistance = distance;
            }
        }

        //squared again so the nearer vowel takes most of the weight, an even split dilutes both shapes
        firstDistance *= firstDistance;
        secondDistance *= secondDistance;

        float total = firstDistance + secondDistance;

        if(total > 0.0f){
            targets[firstClosest] = secondDistance / total;
            targets[secondClosest] = firstDistance / total;
        }
        else{
            targets[firstClosest] = 1.0f;
        }
    }

    for(uint8_t i = 0; i < visemeCount; i++) *visRatios[i] += (targets[i] - *visRatios[i]) * visemeSmoothing;
}
