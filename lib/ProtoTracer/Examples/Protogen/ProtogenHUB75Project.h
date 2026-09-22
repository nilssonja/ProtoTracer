#pragma once

#include "../Templates/ProtogenProjectTemplate.h"
#include "../../Assets/Models/OBJ/DeltaDisplayBackground.h"
#include "../../Assets/Models/FBX/NukudeFlat.h"

#include "../../Camera/CameraManager/Implementations/HUB75DeltaCameras.h"
#include "../../Controller/HUB75Controller.h"

class ProtogenHUB75Project : public ProtogenProject {
private:
    HUB75DeltaCameraManager cameras;
    HUB75Controller controller = HUB75Controller(&cameras, 50, 50);
    NukudeFace pM;
    DeltaDisplayBackground deltaDisplayBackground;
    
	const __FlashStringHelper* faceArray[11] = {F("DEFAULT"), F("ANGRY"), F("DOUBT"), F("FROWN"), F("LOOKUP"), F("SAD"), F("HEART"), F("HEART2"), F("AUDIO1"), F("AUDIO2"), F("AUDIO3")};

    void LinkControlParameters() override {//Called from parent
        AddParameter(NukudeFace::Anger, pM.GetMorphWeightReference(NukudeFace::Anger), 15);
        AddParameter(NukudeFace::Sadness, pM.GetMorphWeightReference(NukudeFace::Sadness), 15, IEasyEaseAnimator::InterpolationMethod::Cosine);
        AddParameter(NukudeFace::Surprised, pM.GetMorphWeightReference(NukudeFace::Surprised), 15);
        AddParameter(NukudeFace::Doubt, pM.GetMorphWeightReference(NukudeFace::Doubt), 15);
        AddParameter(NukudeFace::Frown, pM.GetMorphWeightReference(NukudeFace::Frown), 15);
        AddParameter(NukudeFace::LookUp, pM.GetMorphWeightReference(NukudeFace::LookUp), 15);
        AddParameter(NukudeFace::LookDown, pM.GetMorphWeightReference(NukudeFace::LookDown), 15);
        AddParameter(NukudeFace::HeartEye, pM.GetMorphWeightReference(NukudeFace::HeartEye), 20, IEasyEaseAnimator::InterpolationMethod::Cosine);

        AddParameter(NukudeFace::HideBlush, pM.GetMorphWeightReference(NukudeFace::HideBlush), 15, IEasyEaseAnimator::InterpolationMethod::Cosine, true);

        AddViseme(Viseme::MouthShape::EE, pM.GetMorphWeightReference(NukudeFace::vrc_v_ee));
        AddViseme(Viseme::MouthShape::AH, pM.GetMorphWeightReference(NukudeFace::vrc_v_aa));
        AddViseme(Viseme::MouthShape::AE, pM.GetMorphWeightReference(NukudeFace::vrc_v_ih));
        AddViseme(Viseme::MouthShape::UH, pM.GetMorphWeightReference(NukudeFace::vrc_v_dd));
        AddViseme(Viseme::MouthShape::AR, pM.GetMorphWeightReference(NukudeFace::vrc_v_oh));//"or" and "oh" both land here, the open rounded shape
        AddViseme(Viseme::MouthShape::ER, pM.GetMorphWeightReference(NukudeFace::vrc_v_ch));
        AddViseme(Viseme::MouthShape::OO, pM.GetMorphWeightReference(NukudeFace::vrc_v_ou));//"too", the tighter rounded shape
        AddViseme(Viseme::MouthShape::SS, pM.GetMorphWeightReference(NukudeFace::vrc_v_ss));

        AddBlinkParameter(pM.GetMorphWeightReference(NukudeFace::Blink));
    }

    void Default(){}

    void Angry(){
        AddParameterFrame(NukudeFace::Anger, 1.0f);
        AddMaterialFrame(Color::CRED);
    } 

    void Sad(){
        AddParameterFrame(NukudeFace::Sadness, 1.0f);
        AddParameterFrame(NukudeFace::Frown, 1.0f);
        AddMaterialFrame(Color::CBLUE);
    }

    void Surprised(){
        AddParameterFrame(NukudeFace::Surprised, 1.0f);
        AddParameterFrame(NukudeFace::HideBlush, 0.0f);
        AddMaterialFrame(Color::CRAINBOW);
    }
    
    void Doubt(){
        AddParameterFrame(NukudeFace::Doubt, 1.0f);
    }
    
    void Frown(){
        AddParameterFrame(NukudeFace::Frown, 1.0f);
    }

    void LookUp(){
        AddParameterFrame(NukudeFace::LookUp, 1.0f);
    }

    void LookDown(){
        AddParameterFrame(NukudeFace::LookDown, 1.0f);
    }

    void Heart(){
        //Blink shares the eye vertices, so it has to stay parked while the heart is shaped.
        DisableBlinking();

        AddParameterFrame(NukudeFace::HeartEye, 1.0f);
    }

    void HeartBlush(){
        Heart();
        AddParameterFrame(NukudeFace::HideBlush, 0.0f);
    }

    void SpectrumAnalyzerCallback() override {
        AddMaterialFrame(Color::CHORIZONTALRAINBOW, 0.8f);
    }

    void AudioReactiveGradientCallback() override {
        AddMaterialFrame(Color::CHORIZONTALRAINBOW, 0.8f);
    }

    void OscilloscopeCallback() override {
        AddMaterialFrame(Color::CHORIZONTALRAINBOW, 0.8f);
    }

public:
    ProtogenHUB75Project() : ProtogenProject(&cameras, &controller, 2, Vector2D(), Vector2D(192.0f, 94.0f), 22, 23, 11){
        scene.AddObject(pM.GetObject());
        scene.AddObject(deltaDisplayBackground.GetObject());

        pM.GetObject()->SetMaterial(GetFaceMaterial());
        deltaDisplayBackground.GetObject()->SetMaterial(GetFaceMaterial());

        hud.SetFaceArray(faceArray);
        hud.SetFacePixelArray(cameras.GetMainPixelGroup());

        LinkControlParameters();
        
        SetWiggleSpeed(5.0f);
        SetMenuWiggleSpeed(0.0f, 0.0f, 0.0f);
        SetMenuOffset(Vector2D(17.5f, -3.0f));
        SetMenuSize(Vector2D(192, 56));
    }

    void Update(float ratio) override {
        pM.Reset();

        uint8_t mode = Menu::GetFaceState();//change by button press

        controller.SetBrightness(Menu::GetBrightness());
        controller.SetAccentBrightness(Menu::GetAccentBrightness());

#ifdef MORSEBUTTON
        SelectFaceFromMorse(mode);
#else
        SelectFace(mode);
#endif

        UpdateFace(ratio);

        //The heart pulls the eye off the outer edge of the fit box; the anchor holds that edge for the whole
        //transition so AlignObjectFace doesn't restretch the rest of the face. Not eased on purpose.
        float heartWeight = *pM.GetMorphWeightReference(NukudeFace::HeartEye);
        pM.SetMorphWeight(NukudeFace::HeartAnchor, heartWeight > 0.001f ? 1.0f : 0.0f);

        //The stock blush sits where the heart's flank is, so it slides outboard as the heart forms. Scaled by
        //blush visibility so the hidden (collapsed) blush points never disturb the plain heart face.
        pM.SetMorphWeight(NukudeFace::HeartBlush, heartWeight * (1.0f - *pM.GetMorphWeightReference(NukudeFace::HideBlush)));

        pM.Update();

        AlignObjectFace(pM.GetObject(), -7.5f);

        pM.GetObject()->GetTransform()->SetPosition(GetWiggleOffset());
        pM.GetObject()->UpdateTransform();
    }

    void SelectFace(uint8_t code) {
        EnableBlinking();//Heart() turns this back off for itself

        bool heartFace = code == 6 || code == 7;

        if (IsBooped() && !heartFace && code != 8) {
            Surprised();
            return;
        }

        switch(code) {
            case 0: Default();  break;
            case 1: Angry();    break;
            case 2: Doubt();    break;
            case 3: Frown();    break;
            case 4: LookUp();   break;
            case 5: Sad();      break;
            case 6: Heart();        break;
            case 7: HeartBlush();   break;
            case 8: AudioReactiveGradientFace();    break;
            case 9: OscilloscopeFace();             break;
            default: SpectrumAnalyzerFace();        break;
        }

        if (IsBooped() && heartFace) AddMaterialFrame(Color::CRAINBOW);//Booped hearts stay hearts
    }

    void SelectFaceFromMorse(uint8_t code) {
        EnableBlinking();//Heart() turns this back off for itself

        bool heartFace = code == 11 || code == 12;

        if (IsBooped() && !heartFace && code != 24) {
            Surprised();
            return;
        }

        switch(code) {
            case 1: Angry();        break; // [A]ngry
            case 2: Surprised();    break; // [B]lush
            case 4: Doubt();        break; // [D]oubt
            case 6: Frown();        break; // [F]rown
            case 11: HeartBlush();  break; // [K]iss
            case 12: Heart();       break; // [L]ove
            case 19: Sad();         break; // [S]ad
            case 21: LookUp();      break; // Look [U]p
            case 22: LookDown();    break; // Look [V] Down
            case 24: AudioReactiveGradientFace();   break; // [X] X.X
            case 25: OscilloscopeFace();            break; // [Y] Oscilloscope
            case 26: SpectrumAnalyzerFace();        break; // [Z] Spectrum
            default: Default();     break; // [H] Happy
        }

        if (IsBooped() && heartFace) AddMaterialFrame(Color::CRAINBOW);//Booped hearts stay hearts
    }
};
