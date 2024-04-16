#ifndef ENCODERDRIVER_H
#define ENCODERDRIVER_H

#include "../Parameters/SteppedParameter.h"

/*****************************************************************************//**
 *  Class name: EncoderDriver
 *  Function: Driver for the encoder input, controls the value of FX parameters
 * 
 *  Based off of Encoder class from libDaisy (Stephen Hensley, 2019)
 *  https://github.com/electro-smith/libDaisy/blob/master/src/hid/encoder.h
 ********************************************************************************/

class EncoderDriver
{
public:

    void init(dsy_gpio_pin button, dsy_gpio_pin a, dsy_gpio_pin b, std::function<void()> navCb)
    {
        btn.Init(button);
        
        channelA.pin = a;
        channelB.pin = b;
        channelA.mode = DSY_GPIO_MODE_INPUT;
        channelB.mode = DSY_GPIO_MODE_INPUT;
        channelA.pull = DSY_GPIO_PULLUP;
        channelB.pull = DSY_GPIO_PULLUP;
        dsy_gpio_init(&channelA);
        dsy_gpio_init(&channelB);

        state = DISARMED;
        prevUpdate = System::GetNow();
        isUpdated = false;

        isNavigation = false;
        navCallback = navCb;

        currentParam = 0;
        valueA = 0xFF;
        valueB = 0xFF;
    }

    void tick()
    {
        btn.Debounce();
        if(btn.RisingEdge())
            buttonCallback();

        now = System::GetNow();
        if(now - prevUpdate >= 1) // adjust to change update rate, 1 = 1000Hz, 2 = 2000Hz etc.
        {
            prevUpdate = now;

            // Shift Button states to debounce
            valueA = (valueA << 1) | dsy_gpio_read(&channelA);
            valueB = (valueB << 1) | dsy_gpio_read(&channelB);

            if(state == DISARMED)
                return;
            
            if((valueA & 0x03) == 0x02 && (valueB & 0x03) == 0x00)
                parameters[currentParam]->increment();
            else if((valueB & 0x03) == 0x02 && (valueA & 0x03) == 0x00)
                parameters[currentParam]->decrement();
        }
    }

    void setIsNavigation(bool isNav)
    {
        isNavigation = isNav;
    }

    void buttonCallback()
    {
        if(isNavigation)
            navCallback();
        else 
            changeState();
    }

    bool getButtonState() {return btn.RisingEdge();}

    void changeState()
    {
        switch (state)
        {
            case DISARMED:
                state = ARMED;
            break;
            case ARMED:
                state = DISARMED;
            break;
        }
    }

    void setCurrentParam(std::string newParam)
    { 
        for(unsigned int i = 0 ; i < parameters.size() ; i++)
        {
            if(parameters[i]->getID() == newParam)
            {
                currentParam = i;
                break;
            }
        }
    }

    void addParameter(SteppedParameter* newParam)
    {
        parameters.push_back(newParam);
    }

private:

    enum EncoderState
    {
        DISARMED,
        ARMED
    } state;

    bool isUpdated;
    uint32_t prevUpdate;
    uint32_t now;

    bool isNavigation;
    std::function<void()> navCallback;

    int currentParam;
    std::vector<SteppedParameter*> parameters;

    Switch btn;
    dsy_gpio channelA;
    dsy_gpio channelB;

    uint8_t valueA;
    uint8_t valueB;
};

#endif