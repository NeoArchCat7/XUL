# 1 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino"
# 2 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2
# 3 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2
# 4 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2
# 5 "C:\\Users\\Gregor\\Desktop\\Nace\\Programiranje\\VS Code - git\\XUL\\XUL.ino" 2

USBRename usbRename = USBRename("X.U.L", "NeoArchCat7", "0001");





uint8_t faderPins[3] = {A10, A9, A8}; // Ensure correct pin assignments
uint8_t ccNumbers[3] = {1, 2, 3};
uint8_t lastValues[3] = {0};

void sendMIDI(uint8_t channel, uint8_t control, uint8_t value);
void debugPrint(const char *message);
void receiveCCValuesFromWebsite();
void saveCCValuesToEEPROM();
void loadCCValuesFromEEPROM();

void setup()
{
    loadCCValuesFromEEPROM(); // Load CC values from EEPROM

    for (int i = 0; i < 3; i++)
    {
        pinMode(faderPins[i], 0x0);
    }
    if (true)
    {
        Serial.begin(9600);
    }
}

void loop()
{
    receiveCCValuesFromWebsite();

    for (int i = 0; i < 3; i++)
    {
        uint16_t rawValue = analogRead(faderPins[i]);

        // Map raw values (0–1023) to MIDI range (0–127)
        uint8_t midiValue = map(rawValue, 0, 1023, 0, 127);

        // Send MIDI only if the change exceeds the threshold
        if (((midiValue - lastValues[i])>0?(midiValue - lastValues[i]):-(midiValue - lastValues[i])) > 1 /* Small threshold to prevent flickering*/)
        {
            sendMIDI(0, ccNumbers[i], midiValue);

            if (true)
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

void debugPrint(const char *message)
{
    if (true)
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

        for (int i = 0; i < 3; i++)
        {
            if (doc["cc"][i].is<int>())
            {
                ccNumbers[i] = doc["cc"][i];
                debugPrint("Updated CC values");
            }
        }

        saveCCValuesToEEPROM(); // Save updated CC values to EEPROM
    }
}

void saveCCValuesToEEPROM()
{
    for (int i = 0; i < 3; i++)
    {
        EEPROM.update(i, ccNumbers[i]); // Save each CC value to EEPROM
    }
    if (true)
    {
        debugPrint("CC values saved to EEPROM");
    }
}

void loadCCValuesFromEEPROM()
{
    for (int i = 0; i < 3; i++)
    {
        uint8_t value = EEPROM.read(i); // Read each CC value from EEPROM
        if (value >= 1 && value <= 127) // Validate the value
        {
            ccNumbers[i] = value;
        }
        else
        {
            ccNumbers[i] = i + 1; // Default to {1, 2, 3} if invalid
        }
    }
    if (true)
    {
        debugPrint("CC values loaded from EEPROM");
    }
}
