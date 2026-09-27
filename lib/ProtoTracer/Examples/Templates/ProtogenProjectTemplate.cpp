#include "ProtogenProjectTemplate.h"

void ProtogenProject::LinkParameters(){
    eEA.AddParameter(&offsetFace, offsetFaceInd, 40, 0.0f, 1.0f);
    eEA.AddParameter(&offsetFaceSA, offsetFaceIndSA, 40, 0.0f, 1.0f);
    eEA.AddParameter(&offsetFaceARG, offsetFaceIndARG, 40, 0.0f, 1.0f);
    eEA.AddParameter(&offsetFaceOSC, offsetFaceIndOSC, 40, 0.0f, 1.0f);
}

void ProtogenProject::SetBaseMaterial(Material* material){
    materialAnimator.SetBaseMaterial(Material::Add, material);
}

void ProtogenProject::SetMaterialLayers(){
    materialAnimator.SetBaseMaterial(Material::Add, &gradientMat);
    materialAnimator.AddMaterial(Material::Replace, &yellowMaterial, 40, 0.0f, 1.0f);//layer 1
    materialAnimator.AddMaterial(Material::Replace, &orangeMaterial, 40, 0.0f, 1.0f);//layer 2
    materialAnimator.AddMaterial(Material::Replace, &whiteMaterial, 40, 0.0f, 1.0f);//layer 3
    materialAnimator.AddMaterial(Material::Replace, &greenMaterial, 40, 0.0f, 1.0f);//layer 4
    materialAnimator.AddMaterial(Material::Replace, &purpleMaterial, 40, 0.0f, 1.0f);//layer 5
    materialAnimator.AddMaterial(Material::Replace, &redMaterial, 40, 0.0f, 1.0f);//layer 6
    materialAnimator.AddMaterial(Material::Replace, &blueMaterial, 40, 0.0f, 1.0f);//layer 7
    materialAnimator.AddMaterial(Material::Replace, &flowNoise, 40, 0.15f, 1.0f);//layer 8
    materialAnimator.AddMaterial(Material::Replace, &rainbowSpiral, 40, 0.0f, 1.0f);//layer 9
    materialAnimator.AddMaterial(Material::Replace, &hRainbow, 40, 0.0f, 1.0f);//layer 10
    materialAnimator.AddMaterial(Material::Replace, &blackMaterial, 40, 0.0f, 1.0f);//layer 11
    materialAnimator.AddMaterial(Material::Replace, &sA, 20, 0.0f, 1.0f);
    materialAnimator.AddMaterial(Material::Replace, &aRG, 20, 0.0f, 1.0f);
    materialAnimator.AddMaterial(Material::Replace, &oSC, 20, 0.0f, 1.0f);

    backgroundMaterial.SetBaseMaterial(Material::Add, Menu::GetMaterial());
    backgroundMaterial.AddMaterial(Material::Replace, &yellowMaterial, 40, 0.0f, 1.0f);//layer 1
    backgroundMaterial.AddMaterial(Material::Replace, &orangeMaterial, 40, 0.0f, 1.0f);//layer 2
    backgroundMaterial.AddMaterial(Material::Replace, &whiteMaterial, 40, 0.0f, 1.0f);//layer 3
    backgroundMaterial.AddMaterial(Material::Replace, &greenMaterial, 40, 0.0f, 1.0f);//layer 4
    backgroundMaterial.AddMaterial(Material::Replace, &purpleMaterial, 40, 0.0f, 1.0f);//layer 5
    backgroundMaterial.AddMaterial(Material::Replace, &redMaterial, 40, 0.0f, 1.0f);//layer 6
    backgroundMaterial.AddMaterial(Material::Replace, &blueMaterial, 40, 0.0f, 1.0f);//layer 7
    backgroundMaterial.AddMaterial(Material::Replace, &flowNoise, 40, 0.0f, 1.0f);//layer 8
    backgroundMaterial.AddMaterial(Material::Replace, &rainbowSpiral, 40, 0.0f, 1.0f);//layer 9
    backgroundMaterial.AddMaterial(Material::Replace, &hRainbow, 40, 0.0f, 1.0f);//layer 10
    backgroundMaterial.AddMaterial(Material::Replace, &blackMaterial, 40, 0.0f, 1.0f);//layer 11
    backgroundMaterial.AddMaterial(Material::Add, &sA, 20, 0.0f, 1.0f);
    backgroundMaterial.AddMaterial(Material::Add, &aRG, 20, 0.0f, 1.0f);
    backgroundMaterial.AddMaterial(Material::Add, &oSC, 20, 0.0f, 1.0f);
}

void ProtogenProject::UpdateKeyFrameTracks(){
    blink.Update();
}

void ProtogenProject::UpdateFFTVisemes(){
    const Viseme::MouthShape vowels[] = {Viseme::EE, Viseme::AE, Viseme::UH, Viseme::AR, Viseme::ER, Viseme::AH, Viseme::OO};
    bool useMicrophone = Menu::UseMicrophone();

    if(useMicrophone && MicrophoneFourier::GetUpdateCount() != lastMicUpdate){//a sample window outlasts a frame, only classify new data
        lastMicUpdate = MicrophoneFourier::GetUpdateCount();

        voiceGate.SetSensitivity(float(Menu::GetMicLevel()) / 10.0f);
        voiceGate.Update(MicrophoneFourier::GetLevelDB(), MicrophoneFourier::GetSpeechBandRatio());

        //the unfiltered spectrum, the filtered one adapts to a held vowel and fades it out
        if(voiceGate.IsOpen()){
            voiceDetection.Update(MicrophoneFourier::GetFourier(), MicrophoneFourier::GetSampleRate());
        }
        else{
            voiceDetection.UpdateNoiseFloor(MicrophoneFourier::GetFourier());
            voiceDetection.ResetVisemes();
        }

        #ifdef MICDEBUG
        if(lastMicUpdate % 4 == 0){
            Serial.print("MIC level ");       Serial.print(MicrophoneFourier::GetLevelDB(), 1);
            Serial.print("dB, peak ");        Serial.print(MicrophoneFourier::GetPeakLevel() * 100.0f, 0);
            Serial.print("%, floor ");        Serial.print(voiceGate.GetNoiseFloorDB(), 1);
            Serial.print("dB, opens at ");    Serial.print(voiceGate.GetOpenThresholdDB(), 1);
            Serial.print("dB, speaking ");    Serial.print(voiceGate.GetSpeakingLevelDB(), 1);
            Serial.print("dB, speech band "); Serial.print(MicrophoneFourier::GetSpeechBandRatio(), 2);
            Serial.print(voiceGate.IsOpen() ? ", OPEN " : ", closed ");
            Serial.print(voiceGate.GetEnvelope(), 2);
            Serial.print(", F1 ");            Serial.print(voiceDetection.GetF1(), 0);
            Serial.print(", F2 ");            Serial.print(voiceDetection.GetF2(), 0);
            Serial.print(", ");               Serial.println(voiceDetection.GetDominantVisemeName());
        }
        #endif
    }

    unsigned long now = micros();
    float dT = Mathematics::Constrain(float(now - lastVisemeMicros) / 1000000.0f, 0.0f, 0.1f);
    lastVisemeMicros = now;

    //a mouth opens faster than it closes
    float envelopeTarget = useMicrophone ? voiceGate.GetEnvelope() : 0.0f;
    mouthEnvelope += (envelopeTarget - mouthEnvelope) * (1.0f - expf(-dT / (envelopeTarget > mouthEnvelope ? 0.03f : 0.12f)));

    for(uint8_t i = 0; i < visemeSlots; i++){//visemes may share a morph, so clear before accumulating
        if(visemeParameters[i]) *visemeParameters[i] = 0.0f;
    }

    float shaped = 0.0f;//share of the mouth given a vowel shape that has a morph to show it
    float jaw = 0.0f;//share of the mouth whose vowel shape wants the loudness jaw under it

    for(Viseme::MouthShape vowel : vowels){
        //only follow the detection while speaking, so the mouth keeps its last shape as it closes
        if(useMicrophone && voiceGate.IsOpen()) visemeWeights[vowel] += (voiceDetection.GetViseme(vowel) - visemeWeights[vowel]) * (1.0f - expf(-dT / 0.04f));

        if(visemeParameters[vowel]){
            *visemeParameters[vowel] += visemeWeights[vowel] * mouthEnvelope * mouthGain;
            shaped += visemeWeights[vowel];
            jaw += visemeWeights[vowel] * visemeJaw[vowel];
        }
    }

    //unshaped loudness opens the mouth on its own, and the lip-only vowels ask for the same jaw under them
    jaw += 1.0f - Mathematics::Constrain(shaped, 0.0f, 1.0f);

    if(visemeParameters[Viseme::SS]) *visemeParameters[Viseme::SS] += Mathematics::Constrain(jaw, 0.0f, 1.0f) * mouthEnvelope * 0.5f;
}

void ProtogenProject::SetMaterialColor(){
    switch(Menu::GetFaceColor()){
        case 1: materialAnimator.AddMaterialFrame(yellowMaterial, 0.8f); break;
        case 2: materialAnimator.AddMaterialFrame(orangeMaterial, 0.8f); break;
        case 3: materialAnimator.AddMaterialFrame(whiteMaterial, 0.8f); break;
        case 4: materialAnimator.AddMaterialFrame(greenMaterial, 0.8f); break;
        case 5: materialAnimator.AddMaterialFrame(purpleMaterial, 0.8f); break;
        case 6: materialAnimator.AddMaterialFrame(redMaterial, 0.8f); break;
        case 7: materialAnimator.AddMaterialFrame(blueMaterial, 0.8f); break;
        case 8: materialAnimator.AddMaterialFrame(rainbowSpiral, 0.8f); break;
        case 9: materialAnimator.AddMaterialFrame(flowNoise, 0.8f); break;
        case 10: materialAnimator.AddMaterialFrame(hRainbow, 0.8f); break;
        case 11: materialAnimator.AddMaterialFrame(blackMaterial, 0.8f); break;
        default: break;
    }
}

void ProtogenProject::UpdateFace(float ratio) {
    while(!frameLimiter.IsReady()) delay(1);

    Menu::Update(ratio);

    fanController.SetPWM(Menu::GetFanSpeed() * 25);
    
    xOffset = fGenMatXMove.Update();
    yOffset = fGenMatYMove.Update();
    
    if (Menu::UseBoopSensor()) {
        //Only one of the two sensors is ever fitted (they share address 0x39); the absent one always reads false.
        bool booped60 = boop60.isBooped();
        bool booped99 = boop99.isBooped();

        isBooped = booped60 || booped99;
    }

    hud.SetEffect(Menu::GetEffect());// Pull Effect from menu and store reference in hud for observing data
    hud.Update();
    this->scene.SetEffect(&hud);// Use HUD as effect for overlay/data extraction

    UpdateFFTVisemes();

    MicrophoneFourier::Update();

    sA.SetHueAngle(ratio * 360.0f * 4.0f);
    sA.SetMirrorYState(Menu::MirrorSpectrumAnalyzer());
    sA.SetFlipYState(!Menu::MirrorSpectrumAnalyzer());
    
    aRG.SetRadius((xOffset + 2.0f) * 2.0f + 25.0f);
    aRG.SetSize(Vector2D((xOffset + 2.0f) * 10.0f + 50.0f, (xOffset + 2.0f) * 10.0f + 50.0f));
    aRG.SetHueAngle(ratio * 360.0f * 8.0f);
    aRG.SetRotation(ratio * 360.0f * 2.0f);

    oSC.SetHueAngle(ratio * 360.0f * 8.0f);
    
    SetMaterialColor();
    RGBColor hueFront = RGBColor(255, 0, 0).HueShift(Menu::GetHueF() * 36);
    RGBColor hueBack  = RGBColor(255, 0, 0).HueShift(Menu::GetHueB() * 36);

    gradientSpectrum[0] = hueFront;
    gradientSpectrum[1] = hueBack;
    gradientMat.UpdateGradient(gradientSpectrum);

    flowNoise.SetGradient(hueFront, 0);
    flowNoise.SetGradient(hueBack, 1);

    UpdateKeyFrameTracks();

    eEA.Update();
    
    flowNoise.Update(ratio);
    rainbowSpiral.Update(ratio);
    hRainbow.Update(ratio);
    materialAnimator.Update();
    backgroundMaterial.Update();

    uint8_t faceSize = Menu::GetFaceSize();
    float scale = Menu::ShowMenu() * 0.6f + 0.4f;
    float faceSizeOffset = faceSize * (cameraSize.X / 20.0f);// /2 for min of half size, /10 for 10 face size options
    float faceSizeMaxX = cameraSize.X / 2.0f;

    float xMaxCamera = cameraSize.X - faceSizeMaxX + faceSizeOffset;
    
    aRG.SetPosition(Vector2D(xMaxCamera / 2.0f + xOffset * 4.0f, cameraSize.Y / 2.0f + yOffset * 4.0f));

    objA.SetCameraMax(Vector2D(xMaxCamera, cameraSize.Y - cameraSize.Y * offsetFace).Multiply(scale));
}


void ProtogenProject::SetCameraMain(Vector2D min, Vector2D max){
    this->camMin = min;
    this->camMax = max;

    objA.SetCameraMin(camMin);
    objA.SetCameraMax(camMax);
}

void ProtogenProject::SetCameraRear(Vector2D min, Vector2D max){
    this->camMinRear = min;
    this->camMaxRear = max;

    objARear.SetCameraMin(camMinRear);
    objARear.SetCameraMax(camMaxRear);
}

Transform ProtogenProject::GetAlignmentTransform(Vector2D min, Vector2D max, Object3D* obj, float rotation, float margin){
    objAOther.SetCameraMin(min);
    objAOther.SetCameraMax(max);

    objAOther.SetPlaneOffsetAngle(rotation);
    objAOther.SetEdgeMargin(margin);
    
    return objAOther.GetTransform(obj);
}

Transform ProtogenProject::GetAlignmentTransform(Vector2D min, Vector2D max, Object3D** objects, uint8_t objectCount, float rotation, float margin){
    objAOther.SetCameraMin(min);
    objAOther.SetCameraMax(max);

    objAOther.SetPlaneOffsetAngle(rotation);
    objAOther.SetEdgeMargin(margin);
    
    return objAOther.GetTransform(objects, objectCount);
}

void ProtogenProject::AlignObject(Vector2D min, Vector2D max, Object3D* obj, float rotation, float margin, bool mirror){
    objAOther.SetCameraMin(min);
    objAOther.SetCameraMax(max);

    objAOther.SetPlaneOffsetAngle(rotation);
    objAOther.SetEdgeMargin(margin);
    objAOther.AlignObject(obj);
    
    objAOther.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjects(Vector2D min, Vector2D max, Object3D** objects, uint8_t objectCount, float rotation, float margin, bool mirror){
    objAOther.SetCameraMin(min);
    objAOther.SetCameraMax(max);

    objAOther.SetPlaneOffsetAngle(rotation);
    objAOther.SetEdgeMargin(margin);
    objAOther.AlignObjects(objects, objectCount);
    
    objAOther.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectNoScale(Vector2D min, Vector2D max, Object3D* obj, float rotation, float margin, bool mirror){
    objAOther.SetCameraMin(min);
    objAOther.SetCameraMax(max);

    objAOther.SetPlaneOffsetAngle(rotation);
    objAOther.SetEdgeMargin(margin);
    objAOther.AlignObjectNoScale(obj);
    
    objAOther.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectsNoScale(Vector2D min, Vector2D max, Object3D** objects, uint8_t objectCount, float rotation, float margin, bool mirror){
    objAOther.SetCameraMin(min);
    objAOther.SetCameraMax(max);

    objAOther.SetPlaneOffsetAngle(rotation);
    objAOther.SetEdgeMargin(margin);
    objAOther.AlignObjectsNoScale(objects, objectCount);
    
    objAOther.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectFace(Object3D* obj, float rotation, float margin, bool mirror){
    objA.SetPlaneOffsetAngle(rotation);
    objA.SetEdgeMargin(margin);
    objA.AlignObject(obj);
    objA.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectsFace(Object3D** objects, uint8_t objectCount, float rotation, float margin, bool mirror){
    objA.SetPlaneOffsetAngle(rotation);
    objA.SetEdgeMargin(margin);
    objA.AlignObjects(objects, objectCount);
    objA.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectNoScaleFace(Object3D* obj, float rotation, float margin, bool mirror){
    objA.SetPlaneOffsetAngle(rotation);
    objA.SetEdgeMargin(margin);
    objA.AlignObjectNoScale(obj);
    objA.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectsNoScaleFace(Object3D** objects, uint8_t objectCount, float rotation, float margin, bool mirror){
    objA.SetPlaneOffsetAngle(rotation);
    objA.SetEdgeMargin(margin);
    objA.AlignObjectsNoScale(objects, objectCount);
    objA.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectRear(Object3D* obj, float rotation, float margin, bool mirror){
    objARear.SetPlaneOffsetAngle(rotation);
    objARear.SetEdgeMargin(margin);
    objARear.AlignObject(obj);
    objARear.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectsRear(Object3D** objects, uint8_t objectCount, float rotation, float margin, bool mirror){
    objARear.SetPlaneOffsetAngle(rotation);
    objARear.SetEdgeMargin(margin);
    objARear.AlignObjects(objects, objectCount);
    objARear.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectNoScaleRear(Object3D* obj, float rotation, float margin, bool mirror){
    objARear.SetPlaneOffsetAngle(rotation);
    objARear.SetEdgeMargin(margin);
    objARear.AlignObjectNoScale(obj);
    objARear.SetMirrorX(mirror);
}

void ProtogenProject::AlignObjectsNoScaleRear(Object3D** objects, uint8_t objectCount, float rotation, float margin, bool mirror){
    objARear.SetPlaneOffsetAngle(rotation);
    objARear.SetEdgeMargin(margin);
    objARear.AlignObjectsNoScale(objects, objectCount);
    objARear.SetMirrorX(mirror);
}


ObjectAlign* ProtogenProject::GetObjectAlign(){
    return &objAOther;
}

ObjectAlign* ProtogenProject::GetObjectAlignFace(){
    return &objA;
}

ObjectAlign* ProtogenProject::GetObjectAlignRear(){
    return &objARear;
}

float ProtogenProject::GetFaceScale(){
    uint8_t faceSize = Menu::GetFaceSize();

    float xSizeRatio = 1.0f - 0.5f + faceSize * (1.0f / 20.0f);

    return xSizeRatio;
}

void ProtogenProject::AddParameter(uint8_t index, float* parameter, uint16_t transitionFrames, IEasyEaseAnimator::InterpolationMethod interpolationMethod, bool invertDirection){
    if(invertDirection){
        eEA.AddParameter(parameter, index, transitionFrames, 1.0f, 0.0f);
    }
    else{
        eEA.AddParameter(parameter, index, transitionFrames, 0.0f, 1.0f);
    }

    eEA.SetInterpolationMethod(index, interpolationMethod);
}

void ProtogenProject::SetMouthGain(float gain){
    mouthGain = gain;
}

void ProtogenProject::AddViseme(Viseme::MouthShape visemeName, float* parameter, float jaw){
    visemeParameters[visemeName] = parameter;
    visemeJaw[visemeName] = jaw;
}

void ProtogenProject::AddBlinkParameter(float* blinkParameter){
    blink.AddParameter(blinkParameter);

    blinkSet = true;
}

void ProtogenProject::AddParameterFrame(uint16_t ProjectIndex, float target){
    eEA.AddParameterFrame(ProjectIndex, target);
}

void ProtogenProject::AddMaterial(Material::Method method, Material* material, uint16_t frames, float minOpacity, float maxOpacity){
    materialAnimator.AddMaterial(method, material, frames, minOpacity, maxOpacity);
}

void ProtogenProject::AddMaterialFrame(Color color, float opacity){
    switch(color){
        case CYELLOW:
            materialAnimator.AddMaterialFrame(yellowMaterial, opacity);
            break;
        case CORANGE:
            materialAnimator.AddMaterialFrame(orangeMaterial, opacity);
            break;
        case CWHITE:
            materialAnimator.AddMaterialFrame(whiteMaterial, opacity);
            break;
        case CGREEN:
            materialAnimator.AddMaterialFrame(greenMaterial, opacity);
            break;
        case CPURPLE:
            materialAnimator.AddMaterialFrame(purpleMaterial, opacity);
            break;
        case CRED:
            materialAnimator.AddMaterialFrame(redMaterial, opacity);
            break;
        case CBLUE:
            materialAnimator.AddMaterialFrame(blueMaterial, opacity);
            break;
        case CRAINBOW:
            materialAnimator.AddMaterialFrame(rainbowSpiral, opacity);
            break;
        case CRAINBOWNOISE:
            materialAnimator.AddMaterialFrame(flowNoise, opacity);
            break;
        case CHORIZONTALRAINBOW:
            materialAnimator.AddMaterialFrame(hRainbow, opacity);
            break;
        case CBLACK:
            materialAnimator.AddMaterialFrame(blackMaterial, opacity);
            break;
        default:
            break;
    }
}

void ProtogenProject::AddMaterialFrame(Material& material, float opacity){
    materialAnimator.AddMaterialFrame(material, opacity);
}

void ProtogenProject::AddBackgroundMaterial(Material::Method method, Material* material, uint16_t frames, float minOpacity, float maxOpacity){
    backgroundMaterial.AddMaterial(method, material, frames, minOpacity, maxOpacity);
}

void ProtogenProject::AddBackgroundMaterialFrame(Color color, float opacity){
    switch(color){
        case CYELLOW:
            backgroundMaterial.AddMaterialFrame(yellowMaterial, opacity);
            break;
        case CORANGE:
            backgroundMaterial.AddMaterialFrame(orangeMaterial, opacity);
            break;
        case CWHITE:
            backgroundMaterial.AddMaterialFrame(whiteMaterial, opacity);
            break;
        case CGREEN:
            backgroundMaterial.AddMaterialFrame(greenMaterial, opacity);
            break;
        case CPURPLE:
            backgroundMaterial.AddMaterialFrame(purpleMaterial, opacity);
            break;
        case CRED:
            backgroundMaterial.AddMaterialFrame(redMaterial, opacity);
            break;
        case CBLUE:
            backgroundMaterial.AddMaterialFrame(blueMaterial, opacity);
            break;
        case CRAINBOW:
            backgroundMaterial.AddMaterialFrame(rainbowSpiral, opacity);
            break;
        case CRAINBOWNOISE:
            backgroundMaterial.AddMaterialFrame(flowNoise, opacity);
            break;
        case CHORIZONTALRAINBOW:
            backgroundMaterial.AddMaterialFrame(hRainbow, opacity);
            break;
        case CBLACK:
            backgroundMaterial.AddMaterialFrame(blackMaterial, opacity);
            break;
        default:
            break;
    }
}

void ProtogenProject::AddBackgroundMaterialFrame(Material& material, float opacity){
    backgroundMaterial.AddMaterialFrame(material, opacity);
}

void ProtogenProject::SpectrumAnalyzerFace(){
    sA.Update(MicrophoneFourier::GetFourierFiltered());

    eEA.AddParameterFrame(offsetFaceInd, 1.0f);
    eEA.AddParameterFrame(offsetFaceIndSA, 1.0f);

    materialAnimator.AddMaterialFrame(sA, offsetFaceSA);
    backgroundMaterial.AddMaterialFrame(sA, offsetFaceSA);

    SpectrumAnalyzerCallback();
}

void ProtogenProject::AudioReactiveGradientFace(){
    aRG.Update(MicrophoneFourier::GetFourierFiltered());

    eEA.AddParameterFrame(offsetFaceInd, 1.0f);
    eEA.AddParameterFrame(offsetFaceIndARG, 1.0f);

    materialAnimator.AddMaterialFrame(aRG, offsetFaceARG);
    backgroundMaterial.AddMaterialFrame(aRG, offsetFaceARG);

    AudioReactiveGradientCallback();
}

void ProtogenProject::OscilloscopeFace(){
    oSC.Update(MicrophoneFourier::GetSamples());

    eEA.AddParameterFrame(offsetFaceInd, 1.0f);
    eEA.AddParameterFrame(offsetFaceIndOSC, 1.0f);

    materialAnimator.AddMaterialFrame(oSC, offsetFaceOSC);
    backgroundMaterial.AddMaterialFrame(oSC, offsetFaceOSC);

    OscilloscopeCallback();
}

void ProtogenProject::HideFace(){
    eEA.AddParameterFrame(offsetFaceInd, 1.0f);
}

void ProtogenProject::DisableBlinking(){
    blink.Pause();
    blink.Reset();
}

void ProtogenProject::EnableBlinking(){
    blink.Reset();
    blink.Play();
}

bool ProtogenProject::IsBooped(){
    return isBooped;
}

Vector3D ProtogenProject::GetWiggleOffset(){
    return Vector3D(fGenMatXMove.Update(), fGenMatYMove.Update(), 0);
}

void ProtogenProject::SetWiggleSpeed(float multiplier){
    fGenMatXMove.SetPeriod(5.3f / multiplier);
    fGenMatYMove.SetPeriod(6.7f / multiplier);
}

void ProtogenProject::SetMenuWiggleSpeed(float multiplierX, float multiplierY, float multiplierR){
    Menu::SetWiggleSpeed(multiplierX, multiplierY, multiplierR);
}

void ProtogenProject::SetMenuOffset(Vector2D offset){
    Menu::SetPositionOffset(offset);
}

void ProtogenProject::SetMenuSize(Vector2D size){
    Menu::SetSize(size);
}

Material* ProtogenProject::GetFaceMaterial(){
    return &materialAnimator;
}

Material* ProtogenProject::GetBackgroundMaterial(){
    return &backgroundMaterial;
}

ProtogenProject::ProtogenProject(CameraManager* cameras, Controller* controller, uint8_t numObjects, Vector2D camMin, Vector2D camMax, uint8_t microphonePin, uint8_t buttonPin, uint8_t faceCount) : Project(cameras, controller, numObjects + 1) {
    this->camMin = camMin;
    this->camMax = camMax;
    this->microphonePin = microphonePin;
    this->buttonPin = buttonPin;
    this->faceCount = faceCount;

    this->scene.AddObject(background.GetObject());
    
    background.GetObject()->SetMaterial(&backgroundMaterial);

    LinkParameters();

    SetMaterialLayers();

    objA.SetCameraMax(camMax);
    objA.SetCameraMin(camMin);
    objA.SetJustification(ObjectAlign::Stretch);
    
    this->scene.EnableEffect();

    cameraSize = camMax - camMin;

    //sA.SetSize(cameraSize);
    //sA.SetPosition(cameraSize / 2.0f);

    //oSC.SetSize(cameraSize);
    //oSC.SetPosition(cameraSize / 2.0f);

    
    sA.SetSize(Vector2D(220.0f, 72.0f));
    sA.SetPosition(Vector2D());

    oSC.SetSize(Vector2D(220.0f, 72.0f));
    oSC.SetPosition(Vector2D());

    hud.SetFaceMax(camMax);
    hud.SetFaceMin(camMin);
}

void ProtogenProject::Initialize() {
    controller->Initialize();

    boop60.Initialize(5);
    boop99.Initialize(5);

    hud.Initialize();

    fanController.Initialize();

    MicrophoneFourier::Initialize(microphonePin, 8000, 50.0f, 120.0f);//8KHz sample rate, 50dB min, 120dB max
    
    #ifdef NEOTRELLISMENU
    Menu::Initialize(faceCount);//NeoTrellis
    #else
    Menu::Initialize(faceCount, buttonPin, 500);//7 is number of faces
    #endif
}
