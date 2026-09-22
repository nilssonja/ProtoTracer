#include "VoiceGate.h"

void VoiceGate::SetSensitivity(float sensitivity) {
    openMarginDB = Mathematics::Map(Mathematics::Constrain(sensitivity, 0.0f, 1.0f), 0.0f, 1.0f, 30.0f, 6.0f);
}

void VoiceGate::Update(float levelDB, float speechBandRatio) {
    history[historyIndex] = levelDB;
    historyIndex = (historyIndex + 1) % floorWindows;
    if (historyCount < floorWindows) historyCount++;

    // Speech always pauses within a few seconds, so the quietest recent window is the room and fans,
    // whether or not the gate is open. A floor that only adapted while closed could latch open forever.
    float minimum = history[0];

    for (uint8_t i = 1; i < historyCount; i++) {
        if (history[i] < minimum) minimum = history[i];
    }

    minimum = Mathematics::Max(minimum, minFloorDB);
    noiseFloorDB += (minimum - noiseFloorDB) * 0.1f;

    float openDB = noiseFloorDB + openMarginDB;
    float closeDB = openDB - hysteresisDB;

    if (!isOpen) {
        if (levelDB > openDB && speechBandRatio > minSpeechBandRatio) aboveCount++;
        else aboveCount = 0;

        if (aboveCount >= openWindows) {
            isOpen = true;
            hangCount = hangWindows;
        }
    } else {
        if (levelDB > closeDB) {
            hangCount = hangWindows;
        } else if (hangCount > 0) {
            hangCount--;
        } else {
            isOpen = false;
            aboveCount = 0;
        }
    }

    if (isOpen) {// only learn the speaking level from speech, so silence never raises the gain on the fans
        if (levelDB > peakDB) peakDB += (levelDB - peakDB) * 0.3f;
        else peakDB -= peakDecayDB;
    }

    peakDB = Mathematics::Max(peakDB, openDB + minRangeDB);

    envelope = isOpen ? Mathematics::Constrain((levelDB - closeDB) / (peakDB - 3.0f - closeDB), 0.0f, 1.0f) : 0.0f;
}

bool VoiceGate::IsOpen() {
    return isOpen;
}

float VoiceGate::GetEnvelope() {
    return envelope;
}

float VoiceGate::GetNoiseFloorDB() {
    return noiseFloorDB;
}

float VoiceGate::GetOpenThresholdDB() {
    return noiseFloorDB + openMarginDB;
}

float VoiceGate::GetSpeakingLevelDB() {
    return peakDB;
}
