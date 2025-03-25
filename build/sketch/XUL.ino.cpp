#include <Arduino.h>
#line 1 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino"
#include <usb_rename.h>
#include <MIDIUSB.h>
#include <ArduinoJson.h>
#include <EEPROM.h> // Include EEPROM library

USBRename usbRename = USBRename("X.U.L", "NeoArchCat7", "0001");

#define NUM_FADERS 3
#define ENABLE_DEBUG true
#define RAW_THRESHOLD 4 // Minimum raw value change to update MIDI

const uint8_t faderPins[NUM_FADERS] = {A10, A9, A8}; // Store in flash memory (const)
uint8_t ccNumbers[NUM_FADERS] = {1, 2, 3};
uint8_t lastValues[NUM_FADERS] = {0};
uint16_t lastRawValues[NUM_FADERS] = {0}; // Store last raw values for threshold comparison

void sendMIDI(uint8_t channel, uint8_t control, uint8_t value);
void debugPrint(const char *message);
void receiveCCValuesFromWebsite();
void saveCCValuesToEEPROM();
void loadCCValuesFromEEPROM();

#line 23 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino"
void setup();
#line 38 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino"
void loop();
#line 23 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino"
void setup()
{
    loadCCValuesFromEEPROM();

    for (int i = 0; i < NUM_FADERS; i++)
    {
        pinMode(faderPins[i], INPUT);
    }

    if (ENABLE_DEBUG)
    {
        Serial.begin(9600);
    }
}

void loop()
{
    receiveCCValuesFromWebsite();

    for (int i = 0; i < NUM_FADERS; i++)
    {
        uint16_t rawValue = analogRead(faderPins[i]);

        // Check if the raw value has changed significantly
        if (abs(rawValue - lastRawValues[i]) >= RAW_THRESHOLD)
        {
            lastRawValues[i] = rawValue; // Update the last raw value

            // Map raw values (0–1023) to MIDI range (0–127) using integer division
            uint8_t midiValue = rawValue / 8;

            // Send MIDI only if the MIDI value has changed
            if (midiValue != lastValues[i])
            {
                sendMIDI(0, ccNumbers[i], midiValue);

                if (ENABLE_DEBUG)
                {
                    char debugMsg[100];
                    sprintf(debugMsg, "Fader %d: Raw = %d, MIDI = %d, CC = %d",
                            i + 1, rawValue, midiValue, ccNumbers[i]);
                    debugPrint(debugMsg);
                }

                lastValues[i] = midiValue;
            }
        }
    }
}

void debugPrint(const char *message)
{
    if (ENABLE_DEBUG)
    {
        Serial.println(message);
    }
}

void sendMIDI(uint8_t channel, uint8_t control, uint8_t value)
{
    midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
    MidiUSB.sendMIDI(event);
    MidiUSB.flush();
}

void receiveCCValuesFromWebsite()
{
    if (Serial.available() > 0)
    {
        StaticJsonDocument<200> doc;
        String jsonString = Serial.readStringUntil('\n');
        DeserializationError error = deserializeJson(doc, jsonString);

        if (error)
        {
            debugPrint("Failed to parse JSON");
            return;
        }

        for (int i = 0; i < NUM_FADERS; i++)
        {
            if (doc["cc"][i].is<int>())
            {
                uint8_t newCC = doc["cc"][i];
                if (ccNumbers[i] != newCC) // Only update if the value has changed
                {
                    ccNumbers[i] = newCC;
                    saveCCValuesToEEPROM(); // Save updated CC values to EEPROM
                    debugPrint("Updated CC values and saved to EEPROM");
                }
            }
        }
    }
}

void saveCCValuesToEEPROM()
{
    for (int i = 0; i < NUM_FADERS; i++)
    {
        EEPROM.update(i, ccNumbers[i]); // Save each CC value to EEPROM
    }

    if (ENABLE_DEBUG)
    {
        debugPrint("CC values saved to EEPROM");
    }
}

void loadCCValuesFromEEPROM()
{
    for (int i = 0; i < NUM_FADERS; i++)
    {
        uint8_t value = EEPROM.read(i);

        if (value >= 1 && value <= 127)
        {
            ccNumbers[i] = value;
        }
        else
        {
            ccNumbers[i] = i + 1; // Default to {1, 2, 3} if invalid
        }
    }

    if (ENABLE_DEBUG)
    {
        debugPrint("CC values loaded from EEPROM");
    }
}
