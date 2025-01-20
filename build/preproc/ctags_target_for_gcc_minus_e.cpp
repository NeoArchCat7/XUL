# 1 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino"
# 2 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2
# 3 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2
# 4 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2

USBRename dummy = USBRename("X.U.L", "NeoArchCat7", "0001");

// Configuration constants




int res = 0; // 0 - low res (7-bit); 1 - mid res (10-bit); 2 - high res (14-bit)
uint8_t FADER_PINS[3] = {A10, A9, A8};
uint8_t CC_NUMBERS[3] = {1, 2, 3};
uint16_t lastValues[3] = {0};

// Function prototypes
void sendMIDI7bit(uint8_t channel, uint8_t control, uint8_t value);
void sendMIDI10bit(uint8_t channel, uint8_t control, uint16_t value);
void sendMIDI14bit(uint8_t channel, uint8_t control, uint16_t value);
uint16_t applyDeadZone(uint16_t value, uint16_t maxValue);
void debugPrint(const char *message);

void setup()
{
    for (int i = 0; i < 3; i++)
    {
        pinMode(FADER_PINS[i], 0x0);
    }
    if (false /* Set to false to disable debug prints*/)
    {
        Serial.begin(9600);
    }
}

void loop()
{
    for (int i = 0; i < 3; i++)
    {
        uint16_t currentValue = analogRead(FADER_PINS[i]);
        currentValue = applyDeadZone(currentValue, 1023);

        if (((currentValue - lastValues[i])>0?(currentValue - lastValues[i]):-(currentValue - lastValues[i])) > 5)
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
            if (false /* Set to false to disable debug prints*/)
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
    if (false /* Set to false to disable debug prints*/)
    {
        Serial.println(message);
    }
}

uint16_t applyDeadZone(uint16_t value, uint16_t maxValue)
{
    if (value < 5)
    {
        return 0;
    }
    else if (value > maxValue - 5)
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
