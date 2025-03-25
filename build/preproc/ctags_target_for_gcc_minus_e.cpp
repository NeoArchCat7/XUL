# 1 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino"
# 2 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2
# 3 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2
# 4 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2
# 5 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2

USBRename usbRename = USBRename("X.U.L", "NeoArchCat7", "0001");





const uint8_t faderPins[3] = {A10, A9, A8};
uint8_t ccNumbers[3] = {1, 2, 3};
uint8_t lastValues[3] = {0};
float smoothedRawValues[3] = {0}; // Store smoothed raw values

void sendMIDI(uint8_t channel, uint8_t control, uint8_t value);
void receiveCCValuesFromWebsite();
void saveCCValuesToEEPROM();
void loadCCValuesFromEEPROM();

void setup()
{
    loadCCValuesFromEEPROM();

    for (int i = 0; i < 3; i++)
    {
        pinMode(faderPins[i], 0x0);
        smoothedRawValues[i] = analogRead(faderPins[i]); // Initialize smoothed values
    }
}

void loop()
{
    receiveCCValuesFromWebsite();

    for (int i = 0; i < 3; i++)
    {
        // Read and smooth the raw analog value
        uint16_t rawValue = analogRead(faderPins[i]);
        smoothedRawValues[i] = (0.2 /* Smoothing factor for analog readings*/ * rawValue) + ((1 - 0.2 /* Smoothing factor for analog readings*/) * smoothedRawValues[i]);

        // Map smoothed raw values to MIDI values
        uint8_t midiValue = (uint16_t)smoothedRawValues[i] / 8;

        // Send MIDI only if the MIDI value has changed
        if (midiValue != lastValues[i])
        {
            sendMIDI(0, ccNumbers[i], midiValue);
            lastValues[i] = midiValue;
        }
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
            return;
        }

        for (int i = 0; i < 3; i++)
        {
            if (doc["cc"][i].is<int>())
            {
                uint8_t newCC = doc["cc"][i];
                if (ccNumbers[i] != newCC) // Only update if the value has changed
                {
                    ccNumbers[i] = newCC;
                    saveCCValuesToEEPROM(); // Save updated CC values to EEPROM
                }
            }
        }
    }
}

void saveCCValuesToEEPROM()
{
    for (int i = 0; i < 3; i++)
    {
        EEPROM.update(i, ccNumbers[i]);
    }
}

void loadCCValuesFromEEPROM()
{
    for (int i = 0; i < 3; i++)
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
}
