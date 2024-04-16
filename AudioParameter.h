#ifndef AUDIOPARAMETER_H
#define AUDIOPARAMETER_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "../Utils/Helpers.h"
#include "../Utils/Constants.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>

/*****************************************************************************//**
 *  Class name: AudioParameter
 *  Function: Parameter object for DSP classes that contains pot boilerplate code
 ********************************************************************************/

using namespace daisy;

template <class type>
class AudioParameter
{
public:

    void init(DaisySeed* seed, type mi, type ma, CurveType c, uint8_t ID, std::function<void(type)> cb)
    {
        hw = seed;
        input = 0;

        curVal = 0;
        min = mi;
        max = ma;

        curve = c;
        channelID = ID;
        callback = cb;

        if(channelID != ChannelIDs::ENCODER)
            isSelected = true;
    }
    
    inline void tick() 
    {
        if(!isSelected)
            return;

        float newInput = hw->adc.GetFloat(channelID);
        if(newInput > (input + jitter) || newInput < (input - jitter))
        {
            input = newInput;
            processCurve();
            callback(curVal);
        }
    }

    void processCurve()
    {
        switch(curve)
        {
            case LINEAR:
                curVal = ((input / 1.0f) * (max - min)) + min;
            break;
            case EXP:
                curVal = ((input * input) * (max - min)) + min;
            break;
        }
    }

    inline void setIsSelected(bool s) { isSelected = s; }
    inline type getValue() { return curVal; }
    
private:

    DaisySeed* hw;
    float input;
    const float jitter = 0.01f;

    type curVal, min, max;
    uint8_t channelID;
    CurveType curve;
    bool isSelected;

    std::function<void(type)> callback;
};

template <class type>
struct AudioParameterWrapper
{
    AudioParameter<type> param;
    type value;
};

#endif