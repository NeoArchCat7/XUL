#include <usb_rename.h>
#include <ArduinoJson.h>
#include <MIDIUSB.h>

USBRename dummy = USBRename("X.U.L", "NeoArchCat7", "0001");

// Configuration constants
#define NUM_FADERS 3
#define FADER_THRESHOLD 5
#define DEBUG_MODE false // Set to false to disable debug prints

int res = 0; // 0 - low res (7-bit); 1 - mid res (10-bit); 2 - high res (14-bit)
uint8_t FADER_PINS[NUM_FADERS] = {A10, A9, A8};
uint8_t CC_NUMBERS[NUM_FADERS] = {1, 2, 3};
uint16_t lastValues[NUM_FADERS] = {0};

// Function prototypes
void sendMIDI7bit(uint8_t channel, uint8_t control, uint8_t value);
void sendMIDI10bit(uint8_t channel, uint8_t control, uint16_t value);
void sendMIDI14bit(uint8_t channel, uint8_t control, uint16_t value);
uint16_t applyDeadZone(uint16_t value, uint16_t maxValue);
void debugPrint(const char *message);

void setup()
{
    for (int i = 0; i < NUM_FADERS; i++)
    {
        pinMode(FADER_PINS[i], INPUT);
    }
    if (DEBUG_MODE)
    {
        Serial.begin(9600);
    }
}

void loop()
{
    for (int i = 0; i < NUM_FADERS; i++)
    {
        uint16_t currentValue = analogRead(FADER_PINS[i]);
        currentValue = applyDeadZone(currentValue, 1023);

        if (abs(currentValue - lastValues[i]) > FADER_THRESHOLD)
        {
            if (res == 0)
            {
                sendMIDI7bit(0, CC_NUMBERS[i], currentValue >> 3);
            }
            else if (res == 1)
            {
                sendMIDI10bit(0, CC_NUMBERS[i], currentValue);
            }
            else if (res == 2)
            {
                sendMIDI14bit(0, CC_NUMBERS[i], currentValue);
            }
            if (DEBUG_MODE)
            {
                char debugMsg[50];
                sprintf(debugMsg, "Fader %d: Value = %d, CC = %d", i + 1, currentValue, CC_NUMBERS[i]);
                debugPrint(debugMsg);
            }
            lastValues[i] = currentValue;
        }
    }
}

// Function definitions
void debugPrint(const char *message)
{
    if (DEBUG_MODE)
    {
        Serial.println(message);
    }
}

uint16_t applyDeadZone(uint16_t value, uint16_t maxValue)
{
    if (value < FADER_THRESHOLD)
    {
        return 0;
    }
    else if (value > maxValue - FADER_THRESHOLD)
    {
        return maxValue;
    }
    return value;
}

void sendMIDI7bit(uint8_t channel, uint8_t control, uint8_t value)
{
    midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
    MidiUSB.sendMIDI(event);
    MidiUSB.flush();
}

void sendMIDI10bit(uint8_t channel, uint8_t control, uint16_t value)
{
    midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value >> 3};
    MidiUSB.sendMIDI(event);
    MidiUSB.flush();
}

void sendMIDI14bit(uint8_t channel, uint8_t control, uint16_t value)
{
    midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value >> 7};
    MidiUSB.sendMIDI(event);
    MidiUSB.flush();
}