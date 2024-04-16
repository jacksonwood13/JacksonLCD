#include "daisy_seed.h"
#include "daisysp.h"
#include "daisy_pod.h"
#include "lcdlib/ILI9341_ui_driver.h"
#include "LoopyTunes-main/DSP/Mixer.h"
#include "LoopyTunes-main/Parameters/AudioParameter.h"
#include "LoopyTunes-main/Drivers/KeypadDriver.h"
#include "LoopyTunes-main/Parameters/BinaryParameter.h"
#include "LoopyTunes-main/Drivers/EncoderDriver.h"
//#include "LoopyTunes-main/Utils/Helpers.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
UiDriver driver;
Mixer mixer;
KeypadDriver keypad;
EncoderDriver encoder;

// ADC input channels for the amplitude of each track
AdcChannelConfig adc_configs[ADCINPUTS];

// Function declarations
void DrawMixer(int channel, float value, bool selected); 
void DrawEffectsView(int trackNumber, int selectedEffectIndex);
void ToggleView(); 
void DrawParameterView(int effectIndex, bool bypassState, int currentParam);
int getNumParamsForEffect(int effectIndex);

// Track, effect and parameter tracking
int currentSelectedTrack = 0; 
int currentSelectedEffect = 0; 
int currentParam = 0; 

// Global flags to keep track of the current UI state
bool isMixerView = true;
bool isEffectView = false;
bool isParameterView = false;

bool bypassState = true; // This should represent the real bypass state of the current effect

int main(void) {
    // Initialize hardware and UI driver
    hw.Init();
    driver.Init();
    driver.Fill(COLOR_BLACK);
    driver.Update();
    System::Delay(100);

    // ADC Configuration
    adc_configs[ChannelIDs::AMP1].InitSingle(seed::A0);
    adc_configs[ChannelIDs::AMP2].InitSingle(seed::A1);
    adc_configs[ChannelIDs::AMP3].InitSingle(seed::A2);
    adc_configs[ChannelIDs::AMP4].InitSingle(seed::A3);
    adc_configs[ChannelIDs::MASTER].InitSingle(seed::A11);
    hw.adc.Init(adc_configs, ADCINPUTS);
    hw.adc.Start();

    keypad.init(seed::D0, seed::D1, seed::D2, seed::D3);

    encoder.init(seed::D4, seed::D13, seed::D14, ToggleView);
    encoder.setIsNavigation(true);
    
   
    // Main loop
    while(1) {
        
        keypad.tick(); // Check for keypad input
        encoder.tick(); 

        auto index = keypad.getIndex(); // Get the current index from the keypad
        
        if (isParameterView) {
            if (keypad.isBottomPressed()) {
                currentParam++;
                int numParams = getNumParamsForEffect(currentSelectedEffect);
                if (currentParam >= numParams) currentParam = 0;
            } else if (keypad.isTopPressed()) {
                currentParam--;
                if (currentParam < 0) currentParam = getNumParamsForEffect(currentSelectedEffect) - 1;
            }

            // Update the display for current effect, bypass state, and selected parameter
            DrawParameterView(currentSelectedEffect, bypassState, currentParam);
        }

     
       if (keypad.isLeftPressed()) {
    if (isEffectView) {
        // Navigate back to effects view
        driver.Fill(COLOR_BLACK);
        isEffectView = false;
        isParameterView = false;
        isMixerView = true;   
        
    } else if (isParameterView) {
        // Navigate back to mixer view
        driver.Fill(COLOR_BLACK);
        isMixerView = false;
        isParameterView = false;
        isEffectView = true;
        //driver.Fill(COLOR_BLACK);
    }
}
       

        if(isMixerView) {
            currentSelectedTrack = index.row + 1; // Update the selected track based on the keypad input
            for (int i = 0; i < 4; ++i) {
            float adc_value = hw.adc.GetFloat(i);
            float inverted_value = 1.0f - adc_value; 
            bool selected = (i + 1) == currentSelectedTrack;
            DrawMixer(i + 1, inverted_value, selected);
        }

        } else if(isEffectView){
            currentSelectedEffect = index.col; // Update the selected effect based on the keypad input
            DrawEffectsView(currentSelectedTrack, currentSelectedEffect);

        } else if (isParameterView) {
            if (encoder.getButtonState()) {
            bypassState = !bypassState; // Toggle the bypass state
            DrawParameterView(currentSelectedEffect, bypassState, currentParam); // Update the parameter view
            }
        }
        

        // Update the display
        driver.Update();
        System::Delay(100); // Delay to control refresh rate

        
}
}



/////////////////////////////////////////////   EFFECTS VIEW   /////////////////////////////////////////


void DrawEffectsView(int trackNumber, int selectedEffectIndex) {
    // Fill the background
    //driver.Fill(COLOR_BLACK);
    //driver.Update();
    //System::Delay(100);


    // Set colors for the UI elements
    uint16_t borderColor = COLOR_CYAN;
    uint16_t text_color = COLOR_WHITE;
    uint16_t effectBoxColor = COLOR_BLACK;
    //uint16_t labelBoxColor = COLOR_CYAN; 
    uint16_t selectedEffectColor = COLOR_GREEN;

    // Define fonts
    const FontDef& smallerFont = Font_7x10; 
    const FontDef& largerFont = Font_16x26; 

    const int smallerFontHeight = 10; 
    //const int largerFontHeight = 26;  

    // Define the display dimensions
    const int displayWidth = 320; 
    const int displayHeight = 240; 

    // Track label box and text 
    Rectangle trackLabelRect(10, 10, displayWidth - 20, 50);  
    driver.DrawRect(trackLabelRect, borderColor);
    driver.WriteString(("Track " + std::to_string(trackNumber)).c_str(), 20, 15, largerFont, COLOR_CYAN);

    // Draw the effects label box 
    Rectangle effectsLabelRect(10, 65, 150, 30);
    driver.DrawRect(effectsLabelRect, borderColor);
    driver.WriteString("Effects", 50, 70, Font_11x18, COLOR_CYAN);

    // Top position for effect boxes
    int effectsYStart = 96; 

    // Effects
    const char* effects[] = {"Delay", "Reverb", "Waveshaper", "PitchShift", "Filter"};
    int effectsCount = sizeof(effects) / sizeof(effects[0]);

    // Calc height for effect boxes 
    int effectBoxHeight = (displayHeight - effectsYStart - 10 - (effectsCount * 1)) / effectsCount; 

    // Description box same height as all effects boxes
    int descriptionBoxHeight = effectsCount * effectBoxHeight + (effectsCount - 1); 

     // Draw each effect box
    for (int i = 0; i < effectsCount; ++i) {
        int effectTop = effectsYStart + i * (effectBoxHeight + 1); 
        
        // Determine fill colour for the box
        uint16_t fillColor = (i == selectedEffectIndex) ? selectedEffectColor : effectBoxColor;

        // Draw box with the fill colour
        driver.FillRect(Rectangle(11, effectTop + 1, 148, effectBoxHeight - 2), fillColor);

        // Draw the border for the box
        driver.DrawRect(Rectangle(10, effectTop, 150, effectBoxHeight), borderColor);

        // Center text in box
        driver.WriteString(effects[i], 20, effectTop + (effectBoxHeight - smallerFontHeight) / 2, smallerFont, text_color);
    }

    // Draw the description label box and the larger description box
    Rectangle descLabelRect(170, 65, displayWidth - 180, 30);
    driver.DrawRect(descLabelRect, borderColor);
    driver.Update();
    driver.WriteString("Description", 180, 70, Font_11x18, COLOR_CYAN);

    // Draw the 
    Rectangle descRect(170, effectsYStart, displayWidth - 180, descriptionBoxHeight);
    driver.DrawRect(descRect, borderColor);
    driver.WriteString("Effect details...", 180, effectsYStart + 5, smallerFont, text_color); 
    
    //driver.Update();
    //System::Delay(100);
}

//////////////////////////////////////////    MIXER VIEW    /////////////////////////////////////////////////



void DrawMixer(int channel, float value, bool selected) {
    const int screenWidth = 320; 
    const int screenHeight = 240; 
    const int faderWidth = 30;
    const int faderHeight = 100;
    const int faderSpacing = 35; 
    const int firstFaderX = (screenWidth - (faderWidth * 4 + faderSpacing * 3)) / 2; 
    const int mixerBottom = 180; 
    uint16_t frameColor = COLOR_CYAN;
    uint16_t textColor = COLOR_WHITE;
    const int frameThickness = 2; 
    //driver.Fill(COLOR_BLACK);
    


    // Draw the header frames and texts only for the first channel to avoid redundancy
    if (channel == 1) {
        // Clear the header area
        //driver.FillRect(Rectangle(71, 0, 180, 30), frameColor);

        // Screen border
        driver.DrawRect(Rectangle(frameThickness, frameThickness, screenWidth - (2 * frameThickness), screenHeight - (2 * frameThickness)), frameColor);

        // "Tempo 120" frame and text
        const int tempoBoxWidth = 75; 
        driver.DrawRect(Rectangle(2, 2, tempoBoxWidth, 30), frameColor);
        driver.WriteString("Tempo 120", 10, 15, Font_7x10, frameColor);
        
        // "Main Mixer" 
        const int mainMixerTextY = 7; 
        int mainMixerTextX = (screenWidth / 2) - (4 * 19); 
        driver.WriteString("Main Mixer", mainMixerTextX, mainMixerTextY, Font_16x26, frameColor);

        // "Waveform" frame and text
        const int waveformBoxWidth = 72; 
        int waveformTextX = screenWidth - waveformBoxWidth; 
        driver.DrawRect(Rectangle(waveformTextX, 2, waveformBoxWidth-2, 30), frameColor);
        driver.WriteString("Waveform", waveformTextX + 5, 15, Font_7x10, frameColor);
    }

    // Fader drawing code for all channels
    int bar_x = firstFaderX + (channel - 1) * (faderWidth + faderSpacing);
    int bar_height = static_cast<int>(value * faderHeight);
    int bar_top = mixerBottom - bar_height;

    // Clear the fader area
    driver.FillRect(Rectangle(bar_x, mixerBottom - faderHeight, faderWidth, faderHeight), COLOR_BLACK);

    uint16_t color = selected ? COLOR_GREEN : COLOR_BLUE;

    // Draw the fader bar with the new height
    driver.FillRect(Rectangle(bar_x, bar_top, faderWidth, bar_height), color);

    // Write the channel number underneath the bar
    driver.WriteString(std::to_string(channel).c_str(), bar_x + (faderWidth / 2) - 5, mixerBottom + 10, Font_7x10, textColor);

    
}

////////////////////////////////////// PARAMETER VIEW ///////////////////////////////////////////////////////////

void DrawParameterView(int effectIndex, bool bypassState, int currentParam) {
    // Define UI dimensions and colors
    const uint16_t screenWidth = 320;
    const uint16_t screenHeight = 240;
    const uint16_t borderColor = COLOR_CYAN;
    const uint16_t textColor = COLOR_CYAN;
    const uint16_t sliderBackColor = COLOR_BLACK;
    //const uint16_t sliderColor = COLOR_GREEN;
    const uint16_t headerHeight = 50;
    const uint16_t paramSpacing = 35; // Adjust spacing as needed
    const uint16_t sliderHeight = 20; // Adjust height as needed
    const uint16_t sliderWidth = 180; // Slider width
    const uint16_t frameThickness = 2;
    const uint16_t sliderOffsetX = 130; // X position for sliders
    const uint16_t bypassButtonWidth = 60;
    const uint16_t bypassButtonHeight = 30;

    // Variable for dynamic effect parameters (Example values)
    int numberOfParameters = 0; // Default to 5, adjust based on the effect
    if (effectIndex == 0) { numberOfParameters = 3; } // Example: Delay has 4 parameters
    else if (effectIndex == 1) { numberOfParameters = 2; } // Example: Reverb has 3 parameters
    else if (effectIndex == 2) { numberOfParameters = 3; }
    else if (effectIndex == 3) { numberOfParameters = 2; }
    else if (effectIndex == 4) { numberOfParameters = 3; }
    // Add other conditions based on effectIndex for different effects

    // Parameter names for each effect
    const char* parameterNames[][5] = {
        {"Amount", "Size", "Feedback", nullptr}, // Delay
        {"Amount", "Size", nullptr},             // Reverb
        {"Amount", "Function Control", "Mode", nullptr}, // Waveshaper
        {"Amount", "Semitones", nullptr},        // Pitch Shifter
        {"Mode", "Frequency", "Resonance", nullptr} // Filter
    };

    // Clear the screen for the parameter view
    //driver.Fill(COLOR_BLACK);

    // Screen border
    driver.DrawRect(Rectangle(frameThickness, frameThickness, screenWidth - (2 * frameThickness), screenHeight - (2 * frameThickness)), borderColor);

    // Header and effect name
    const char* effectNames[] = {"Delay", "Reverb", "Waveshaper", "PitchShift", "Filter"};
    char headerText[50];
    snprintf(headerText, sizeof(headerText), "Track %d-%s", currentSelectedTrack, effectNames[effectIndex]);
    driver.DrawRect(Rectangle(10, 10, screenWidth - 20, headerHeight), borderColor);
    driver.WriteString(headerText, 15, 15, Font_16x26, textColor);

    // Draw on/off button for Bypass
    uint16_t bypassButtonY = headerHeight + 15; ///
    uint16_t buttonColor = bypassState ? COLOR_GREEN : COLOR_RED;
    Rectangle bypassButtonRect(10, bypassButtonY, bypassButtonWidth, bypassButtonHeight); ///
    driver.FillRect(bypassButtonRect, buttonColor);
    driver.DrawRect(bypassButtonRect, borderColor);
    driver.WriteString(bypassState ? "On" : "Off", 20, headerHeight + 20, Font_11x18, textColor);

    uint16_t startY = bypassButtonRect.GetY() + bypassButtonHeight + 30; /////

    // Draw parameters for the selected effect
    for (int i = 0; parameterNames[effectIndex][i] != nullptr; ++i) {
        uint16_t paramY = startY + (i * paramSpacing);
        driver.WriteString(parameterNames[effectIndex][i], 10, paramY + 5, Font_7x10, textColor);

        // Draw slider for the parameter value
        Rectangle sliderRect(sliderOffsetX, paramY, sliderWidth, sliderHeight);
        //Rectangle sliderRect(screenWidth - sliderWidth - 10, paramY, sliderWidth, sliderHeight);
        driver.FillRect(sliderRect, sliderBackColor);
        // Slider foreground representing the actual parameter value (this part is just for illustration)
        driver.FillRect(Rectangle(sliderOffsetX, paramY, sliderWidth * 0.5, sliderHeight), (i == currentParam) ? COLOR_GREEN : COLOR_BLUE); // Assume 50% value for illustration
        // Slider border
        driver.DrawRect(Rectangle(sliderOffsetX, paramY, sliderWidth, sliderHeight), (i == currentParam) ? COLOR_GREEN : COLOR_BLUE);
    }

    
    //driver.Update();
}


int getNumParamsForEffect(int effectIndex) {
    // Returns the number of parameters for the chosen effect
    switch(effectIndex) {
        case 0: return 3; // Delay
        case 1: return 2; // Reverb
        case 2: return 3; // Waveshaper
        case 3: return 2; // Pitch Shifter
        case 4: return 3; // Filter
        default: return 0; // Default case
    }
}

void ToggleView() {
    if (isMixerView) {
        isMixerView = false;
        isEffectView = true;
        isParameterView = false;
    } else if (isEffectView) {
        isEffectView = false;
        isParameterView = true;
    }
    driver.Fill(COLOR_BLACK); 
}