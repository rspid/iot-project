# IoT Projet

## Exercice 1 - Mise en place du réseau

### Communication passerelle -> android

- Protocole UDP
- Cryptage côté passerelle & côté android

### Communication microbit -> microbit

- Transmission radio
- Cryptage à l'envoie

## Exercice 2 - Configuration des capteurs

### Microbit déployé

- Récolte la température & la luminosité
- Affiche les valeurs sur l'écran OLED dans **l'ordre demandé par la passerelle**

### Écran OLED

- Relié au Microbit déployé
- Affiche les informations reçu

### Communication avec la passerelle

- Format de message: **JSON**
- Écoute & envoie des messages
- Donnée de config envoyé en en-tête

## Code température & affichage

Exemple de code d'affichage

```cpp
#include "ssd1306.h"
#include "MicroBit.h"

#include <string>

MicroBit uBit;
MicroBitI2C i2c(I2C_SDA0,I2C_SCL0);
MicroBitPin P0(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_DIGITAL_OUT);
#define BUFFER_LENGTH 64


char * getTemperature(char *buffer) {
        int ret = snprintf(buffer, BUFFER_LENGTH, "%d", uBit.thermometer.getTemperature());
        if (ret != 0) {
            return buffer;
        }
        return NULL;
}
void displayTemperature(ssd1306 screen) {
    char buffer[BUFFER_LENGTH];
    screen.display_line(0,0, "Temperature: ");
    screen.display_line(0, 14, getTemperature(buffer));
    screen.update_screen();
}
int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();
    ssd1306 screen(&uBit, &i2c, &P0);
    screen.power_on();
    while(true)
    {
        displayTemperature(screen);
        uBit.sleep(5);
    }
    release_fiber();

}


```

## Code Radio

    Exemple de code radio

### Receiver

```cpp

#include "MicroBit.h"

MicroBit uBit;

void onData(MicroBitEvent e)
{
    ManagedString s = uBit.radio.datagram.recv();
    if (s == "1")
    {
        uBit.display.scroll("SUCCESS_1");

    }
    if (s == "2")
    {
        uBit.display.scroll("SUCCESS_2");
    }
}
int main()
{
    uBit.init();
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    uBit.radio.enable();

    while(1) {
        uBit.display.scroll("2");
        uBit.sleep(1);
    }
    release_fiber();
}
// https://lancaster-university.github.io/microbit-docs/


```

### Sender

```cpp
#include "MicroBit.h"

MicroBit uBit;

int main()
{
    uBit.init();
    uBit.radio.enable();
    while(1)
    {
        if (uBit.buttonA.isPressed()) {
            uBit.display.scroll("1");
            uBit.radio.datagram.send("1");
        }
        else if (uBit.buttonB.isPressed()) {
            uBit.display.scroll("1");
            uBit.radio.datagram.send("2");

        }
        uBit.sleep(1);
    }
    release_fiber();
}
```

### Format

- JSON

```C
{"t":25,"l":540}
```

### Chiffrement

- Chiffrement à décalage (CAESAR)
- Pas de 2

###

## Ressources

[![C++]()](https://lancaster-university.github.io/microbit-docs/)
