/**
 * @file SSD1306.h
 * @brief Declares the HeadsUpDisplay class, which mirrors the main display onto an SSD1306/SH1106 OLED.
 *
 * The HeadsUpDisplay sits in the screen-space effect chain so it sees every finished frame. It
 * copies the main camera's pixels, upscaled and thresholded to 1-bit, onto the full OLED so the
 * wearer sees exactly what the outside panels show — face, effects and the on-panel menu.
 *
 * @date 22/12/2024
 * @author Coela Can't
 */

#pragma once

#include <Arduino.h> // Include for Arduino compatibility.
#include <Wire.h> // Include for I2C communication.
#include <Adafruit_GFX.h> // Include for Adafruit GFX library.

#include "../../Examples/UserConfiguration.h" // Include for user-specific configurations.
#include "../../Utils/Math/Mathematics.h" // Include for mathematical utilities.
#include "../../Scene/Screenspace/Effect.h" // Include for effect interface.
#include "../../Utils/Time/TimeStep.h" // Include for timestep utility.

#ifdef SH1106
#include "Adafruit_SH1106.h" // Include for SH1106 display (untested).
#else
#include <Adafruit_SSD1306.h> // Include for SSD1306 display.
#endif

/**
 * @class HeadsUpDisplay
 * @brief Mirrors the main display onto an SSD1306/SH1106 OLED.
 *
 * The HeadsUpDisplay class acts as an Effect, allowing it to intercept the rendered frame of
 * every camera. Only the main camera's pixel group (set with SetFacePixelArray) is drawn; the
 * side panels share the same coordinate space and would otherwise overlap the face.
 */
class HeadsUpDisplay : public Effect {
private:
    static const uint16_t SCREEN_WIDTH = 128; ///< Width of the OLED screen.
    static const uint16_t SCREEN_HEIGHT = 64; ///< Height of the OLED screen.
    static const int8_t OLED_RESET = -1; ///< Reset pin for the display.
    static const uint32_t splashTime = 2500; ///< Duration for splash screen in milliseconds.
    static const int bufferSize = SCREEN_WIDTH * SCREEN_HEIGHT / 8; ///< 1-bit frame buffer size.
    static const uint8_t litThreshold = 24; ///< Brightest channel must exceed this (of 255) to light an OLED pixel.

    Effect* subEffect = nullptr; ///< Used to capture the complete rendered frame
    TimeStep timeStep = TimeStep(15); ///< Limits the display to update 15 times per second
    bool didBegin = false; ///< True if the I2C interface started correctly
    bool splashFinished = false; ///< True when the splash startup screen is finished
    bool mirrorX = false; ///< Flip the mirrored image horizontally
    Vector2D faceMin; ///< Unused; kept for API compatibility
    Vector2D faceMax; ///< Unused; kept for API compatibility
    uint32_t startMillis; ///< Start time of the display for the splash screen

#ifdef SH1106
    static Adafruit_SH1106 display;
#else
    static Adafruit_SSD1306 display;
#endif
    static uint8_t faceBitmap[bufferSize]; ///< 1-bit copy of the main display, built up during ApplyEffect

    static const uint8_t CoelaSplash[];
    static const uint8_t PrototracerSplash[];

    const IPixelGroup* facePixels = nullptr; ///< The main camera's pixel group; only this one is mirrored

    /**
     * @brief Resets the display buffer to a blank state.
     */
    void ResetDisplayBuffer();

    /**
     * @brief Lights a dot-sized block in the mirror buffer.
     *
     * @param x Left column of the block on the OLED.
     * @param y Top row of the block on the OLED.
     * @param dot Block size in OLED pixels.
     */
    void FillBlock(uint16_t x, uint16_t y, uint8_t dot);

    /**
     * @brief Draws the mirror buffer to the display.
     */
    void DrawMirror();

public:
    /**
     * @brief Constructs a HeadsUpDisplay.
     *
     * @param faceMin Unused; kept for API compatibility.
     * @param faceMax Unused; kept for API compatibility.
     */
    HeadsUpDisplay(Vector2D faceMin, Vector2D faceMax);

    /**
     * @brief No longer used; the OLED mirrors the panels instead of printing face names.
     *
     * @param faceNames Pointer to an array of face names.
     */
    void SetFaceArray(const __FlashStringHelper** faceNames);

    /**
     * @brief Sets the main camera's pixel group; only this group is mirrored to the OLED.
     *
     * @param pixelGroup Pointer to the main camera's pixel group.
     */
    void SetFacePixelArray(const IPixelGroup* pixelGroup);

    /**
     * @brief Flips the mirrored image horizontally.
     *
     * @param mirrorX True to flip.
     */
    void SetMirrorX(bool mirrorX);

    /**
     * @brief No longer used; kept for API compatibility.
     */
    void SetFaceMin(Vector2D faceMin);

    /**
     * @brief No longer used; kept for API compatibility.
     */
    void SetFaceMax(Vector2D faceMax);

    /**
     * @brief Initializes the display and related components.
     */
    void Initialize();

    /**
     * @brief Resets the I2C bus in case of communication errors.
     */
    void ResetI2CBus();

    /**
     * @brief Updates the display content based on the current state.
     */
    void Update();

    /**
     * @brief Sets the sub-effect to be applied to the display.
     *
     * @param effect Pointer to the Effect to be applied.
     */
    void SetEffect(Effect* effect);

    /**
     * @brief Applies the sub-effect, then copies the main camera's pixels into the mirror buffer.
     *
     * @param pixelGroup Pointer to the pixel group to process.
     */
    void ApplyEffect(IPixelGroup* pixelGroup);

};
