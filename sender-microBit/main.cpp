/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "MicroBit.h"
#include "ssd1306.h"
#include "tsl256x.h"


#include <string>
#define BUFFER_LENGTH 64
#define BUFFER 64


MicroBit uBit;
MicroBitI2C i2c(I2C_SDA0,I2C_SCL0);
MicroBitPin P0(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_DIGITAL_OUT);
ssd1306 screen(&uBit, &i2c, &P0);
tsl256x tsl(&uBit,&i2c);

char *order;

int my_strlen(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

int my_strlen_const(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

    char *cryptJson(char *json, int shift) {
        char *buffer = (char*)malloc(sizeof(char) * strlen(json) + 1);
        int i;
        for (i = 0; i < my_strlen(json); i++) {
            buffer[i] = (json[i] + shift);
        }
        buffer[i] = '\0';
        return buffer;

    }

char *decipherMsg(const char *msg, int shift) {
    char *clearMsg = (char*) malloc(sizeof(char) * strlen(msg) + 1);
    int i;
    for (i = 0; i < my_strlen_const(msg); i++) {
        clearMsg[i] = (msg[i] - shift);
    }
    clearMsg[i] = '\0';
    return clearMsg;
}

void getOrderFromJson(char *json, char* order) {
    
    char propertie[BUFFER];
    for (int i = 0; i < my_strlen(json); i++) {
        if (json[i] == '\'') {
            // in a propertie
            int j = 0;
            i++;
            while (json[i] != '\'') {
                propertie[j] = json[i];
                i++;
                j++;
            }
            propertie[j] = '\0';
            puts(propertie);
            i++;
            while (json[i] != ':') {
                // trim()
                i++;
            }
            i++;
            while (json[i] != '\'') {
                // trim()
                i++;
            }
            i++;
            char value[BUFFER];
            int y = 0;
            while (json[i] != '\'') {
                value[y] = json[i];
                i++;
                y++;
            }
            value[y] = '\0';
            strcpy(order, value);
            uBit.display.scroll(value);
        }
    }

}

void onData(MicroBitEvent e)
{
    // afficher les infos reçu par le microcontrolleur
    uBit.display.scroll("RECU");
    ManagedString s = uBit.radio.datagram.recv();
    char *msg = decipherMsg(s.toCharArray(), 2);
    getOrderFromJson(msg, order);
    screen.display_line(3, 0, msg);
    screen.update_screen();
    free(msg);
    // uBit.display.scroll(order);    
}

char *getTemperature(char *buffer) {
        int ret = snprintf(buffer, BUFFER_LENGTH, "%d", uBit.thermometer.getTemperature());
        if (ret != 0) {
            return buffer;
        }
        return NULL;
}

char *getLum(char *buffer){
    uint16_t comb =0;
    uint16_t ir = 0;
    uint32_t lux = 0;
    tsl.sensor_read(&comb, &ir, &lux);
    int ret = snprintf(buffer, BUFFER_LENGTH, "%ld", lux);
    if (ret != 0) {
        return buffer;
    }
    return NULL;
}

void displayInfo(char *buffer, char *bufferLux, char *order) {

    if (strcmp(order, "TL")) {
        screen.display_line(0,0, "Temperature: ");
        screen.display_line(0, 14, getTemperature(buffer));

        screen.display_line(1,0, "Luminosite: ");
        screen.display_line(1, 13, getLum(bufferLux));

        screen.update_screen();
    } else {
        screen.display_line(1,0, "Luminosite: ");
        screen.display_line(1, 13, getLum(bufferLux));
        
        screen.display_line(0,0, "Temperature: ");
        screen.display_line(0, 14, getTemperature(buffer));

        screen.update_screen();
    }


}

char *buildDatagram(char *temp, char *lum) {
    char *json = (char*)malloc(sizeof(char) *128);
    strcpy(json, "{\"t\":");
    strcat(json, temp);
    strcat(json, ",\"l\": ");
    strcat(json, lum);
    strcat(json, "}\n");
    strcat(json , "\0");
    // screen.display_line(3,3, json);
    // screen.update_screen();
    // TODO : Chiffrement
    char * cryptedMsg = cryptJson(json, 2);
    free(json);
    uBit.display.scroll("ENVOIE");
    return cryptedMsg;
}

void serial() {
    ManagedString uart = uBit.serial.read(uBit.serial.getRxBufferSize(), ASYNC);
    if (uart.length() > 0) {
        std::string plain = uart.toCharArray();
        char *cstr = new char[plain.length() + 1];
        strcpy(cstr, plain.c_str());
    }
}

int main()
{
    // Initialise the micro:bit runtime.
    order = (char*) malloc(sizeof(char) * 3);
    order[0] = 'T';
    order[1] = 'L';
    order[2] = '\0';

    uBit.init();
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    char buffer[BUFFER_LENGTH];
    char bufferLux[BUFFER_LENGTH];

    screen.power_on();

    // Insert your code here!
    uBit.radio.enable();
    uBit.radio.setGroup(18);
    while(1)
    {       
        // screen.display_line(0,0, "Temperature: ");
        // screen.update_screen();



        char *lum = getLum(bufferLux);
        char *temp = getTemperature(buffer);
        uBit.display.scroll("A");
        displayInfo(lum, temp, order);
        char *json= buildDatagram(temp, lum);
        uBit.radio.datagram.send(json);
        free(json);
        uBit.sleep(1);
    }
    release_fiber();
}
