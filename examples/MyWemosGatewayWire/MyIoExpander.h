/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2021 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Maximilian Wache
 *
 */

#pragma once

typedef void (*OnMyIoExpanderInputChange_t)(uint8_t, bool);

class MyIoExpander
{
private:
    const uint8_t IN_OUT_ADDR = 0x20; // TWI IO-Expander 8574 address 000
    //const uint8_t IN_OUT_ADDR = 0x38; // TWI IO-Expander 8574A address 000

    const uint8_t INPUT_MASK = 0xC0;

    // Input pins of IO-Expander P6 - P7
    const uint8_t EXP_INCLUSION_BUTTON = 6;
    const uint8_t EXP_AUX_BUTTON = 7;

    // Output pins of IO-Expander P0 - P5
    const uint8_t EXP_RX_LED = 0;
    const uint8_t EXP_TX_LED = 1;
    const uint8_t EXP_ERROR_LED = 2;
    const uint8_t EXP_INCLUSION_LED = 3;
    const uint8_t EXP_AUX2_LED = 4;
    const uint8_t EXP_AUX_LED = 5;

public:
    MyIoExpander(void)
    {
        ledsInit();
    }

    void attachOnInputChange(OnMyIoExpanderInputChange_t onInputChange)
    {
        this->onInputChange = onInputChange;
    }

    void setup()
    {
    }

    void loop(void)
    {
        ledsProcess();
        updateExpander();
    }

    bool ledsBlinking()
    {
        return countRx || countTx || countErr;
    }

    void setLedIndication(const indication_t ind)
    {
        if ((INDICATION_TX == ind) || (INDICATION_GW_TX == ind)) {
            ledsBlinkTx(1);
        }
        else if ((INDICATION_RX == ind) || (INDICATION_GW_RX == ind)) {
            ledsBlinkRx(1);
        }
        else if (ind > INDICATION_ERR_START) {
            // Number of blinks indicates which error occurred.
            ledsBlinkErr(ind - INDICATION_ERR_START);
        }
    }

    void setInclusionMode(bool newMode)
    {
        _inclusionMode = newMode;
        bitWrite(_outData, EXP_INCLUSION_LED, !_inclusionMode);
    }

    void setAuxLed(bool state)
    {
        bitWrite(_outData, EXP_AUX_LED, !state);
    }

private:
    void ledsInit()
    {
        // initialize counters
        countRx = 0;
        countTx = 0;
        countErr = 0;

        // Subtract some, to make sure leds gets updated on first run.
        prevTime = hwMillis() - _LED_PROCESS_INTERVAL_MS;
        ledsProcess();
    }

    void ledsProcess() // do the actual blinking
    {
        // Just return if it is not the time...
        if ((hwMillis() - prevTime) < _LED_PROCESS_INTERVAL_MS) {
            return;
        }
        prevTime = hwMillis();

        uint8_t state;

        // For an On/Off ratio of 4, the pattern repeated will be [on, on, on, off]
        // until the counter becomes 0.
        if (countRx) {
            --countRx;
        }
        state = (countRx & (_LED_ON_OFF_RATIO - 1)) ? _LED_ON : _LED_OFF;
        bitWrite(_outData, EXP_RX_LED, state);

        if (countTx) {
            --countTx;
        }
        state = (countTx & (_LED_ON_OFF_RATIO - 1)) ? _LED_ON : _LED_OFF;
        bitWrite(_outData, EXP_TX_LED, state);

        if (countErr) {
            --countErr;
        }
        state = (countErr & (_LED_ON_OFF_RATIO - 1)) ? _LED_ON : _LED_OFF;
        bitWrite(_outData, EXP_ERROR_LED, state);
    }

    void ledsBlinkRx(uint8_t cnt)
    {
        if (!countRx) {
            countRx = cnt * _LED_ON_OFF_RATIO;
        }
        ledsProcess();
    }

    void ledsBlinkTx(uint8_t cnt)
    {
        if (!countTx) {
            countTx = cnt * _LED_ON_OFF_RATIO;
        }
        ledsProcess();
    }

    void ledsBlinkErr(uint8_t cnt)
    {
        if (!countErr) {
            countErr = cnt * _LED_ON_OFF_RATIO;
        }
        ledsProcess();
    }

    int16_t expanderRead(uint8_t i2caddr)
    {
        int16_t _data = -1;
        Wire.requestFrom(i2caddr, 1u);
        if (Wire.available()) {
            _data = Wire.read();
        }
        return _data;
    }

    void expanderWrite(uint8_t i2caddr, uint8_t data)
    {
        Wire.beginTransmission(i2caddr);
        Wire.write(data);
        Wire.endTransmission();
    }

    void updateExpander()
    {
        int data = expanderRead(IN_OUT_ADDR);
        if (data != -1) {
            expanderWrite(IN_OUT_ADDR, INPUT_MASK | _outData);

        }
        else {
            Serial.print("WIRE read error, status: ");
            uint8_t status = Wire.status();
            Serial.println(status);

            delay(100);
            Wire.begin();
            delay(100);
        }

        if (!_inclusionMode && (data & bit(EXP_INCLUSION_BUTTON)) == 0) {
            // Start inclusion mode
            inclusionModeSet(true);
        }

        bool buttonPressed = (data & bit(EXP_AUX_BUTTON)) == 0;
        if (_auxButtonState != buttonPressed) {
            {
                if (onInputChange != NULL) {
                    (*onInputChange)(EXP_AUX_BUTTON, buttonPressed);
                }
            }
            _auxButtonState = buttonPressed;
        }
    }

    bool _inclusionMode;
    bool _auxButtonState;
    uint8_t _outData = 0x3F; // Output data of IO-Expander
    OnMyIoExpanderInputChange_t onInputChange;

    const uint8_t _LED_ON = 0x0;
    const uint8_t _LED_OFF = 0x1;
    const uint8_t _LED_ON_OFF_RATIO = 4; // Power of 2 please
    const uint32_t _LED_PROCESS_INTERVAL_MS = MY_DEFAULT_LED_BLINK_PERIOD / _LED_ON_OFF_RATIO;

    uint8_t countRx;
    uint8_t countTx;
    uint8_t countErr;
    unsigned long prevTime;
};

MyIoExpander expander;
