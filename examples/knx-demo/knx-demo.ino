#include <Arduino.h>
#include <knx.h>

#if MASK_VERSION != 0x07B0 && (defined ARDUINO_ARCH_ESP8266 || defined ARDUINO_ARCH_ESP32)
    #include <WiFiManager.h>
#endif

/**  If you don't want a global knx object, for example because you want
 * to more finely control it's construction, this is an example
 * of how to do so. Define KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
 * and then you can DIY a knx object as shown below. In this case we use
 * the ESP32's secondary UART and late-bind the ISR function in setup().

Esp32Platform knxPlatform(&Serial2);
Bau07B0 knxBau(knxPlatform);
KnxFacade<Esp32Platform, Bau07B0> knx(knxBau);
*/

// create named references for easy access to group objects
#define goCurrent knx.getGroupObject(1)
#define goMax knx.getGroupObject(2)
#define goMin knx.getGroupObject(3)
#define goReset knx.getGroupObject(4)

float currentValue = 0;
float maxValue = 0;
float minValue = RAND_MAX;
long lastsend = 0;

void measureTemp()
{
    long now = millis();

    if ((now - lastsend) < 2000)
        return;

    lastsend = now;
    int r = rand();
    currentValue = (r * 1.0) / (RAND_MAX * 1.0);
    currentValue *= 100 * 100;

    // write new value to groupobject
    goCurrent.value<Dpt9>(currentValue);

    if (currentValue > maxValue)
    {
        maxValue = currentValue;
        goMax.value<Dpt9>(maxValue);
    }

    if (currentValue < minValue)
    {
        minValue = currentValue;
        goMin.value<Dpt9>(minValue);
    }
}

// callback from reset-GO
void handler(GroupObject& go)
{
    if (go == goReset && go.value<Dpt1>())
    {
        maxValue = 0;
        minValue = 10000;
    }
}

void setup()
{
    // You can configure the level of the different loggers.
    //Logger::logLevel("ApplicationLayer", Logger::Info);
    Serial.begin(115200);
    ArduinoPlatform::SerialDebug = &Serial;

    randomSeed(millis());

#if MASK_VERSION != 0x07B0 && (defined ARDUINO_ARCH_ESP8266 || defined ARDUINO_ARCH_ESP32)
    WiFiManager wifiManager;
    wifiManager.autoConnect("knx-demo");
#endif

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();
    GroupObject::classCallback(handler);

    // print values of parameters if device is already configured
    if (knx.configured())
    {
        Serial.print("Startverzögerung s: ");
        Serial.println(knx.paramByte(0));
        Serial.print("Aenderung senden (*0.1K): ");
        Serial.println(knx.paramByte(1));
        Serial.print("Zykl. senden min: ");
        Serial.println(knx.paramByte(2));
        Serial.print("Min/Max senden: ");
        Serial.println(knx.paramByte(3));
        Serial.print("Abgleich: ");
        Serial.println(knx.paramByte(4));
    }

    // pin or GPIO the programming led is connected to. Default is LED_BUILTIN
    // knx.ledPin(LED_BUILTIN);
    // is the led active on HIGH or low? Default is LOW
    // knx.ledPinActiveOn(HIGH);
    // pin or GPIO programming button is connected to. Default is 0
    // knx.buttonPin(0);

    // start the framework.
    knx.start();
}

void loop()
{
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if (!knx.configured())
        return;

    measureTemp();
}
