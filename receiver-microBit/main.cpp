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

#include <string>


MicroBit uBit;

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

void sendData(char * msg) {
    // {message: 'TL'}
    // cipher:
    cryptJson(msg, 2);
    uBit.radio.datagram.send(msg);
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


void onData(MicroBitEvent e)
{
    ManagedString s = uBit.radio.datagram.recv();
    char *clear = decipherMsg(s.toCharArray(), 2); 
    uBit.display.scroll("RECU");
    uBit.serial.printf(clear);
    // TODO UART0 : 
    ManagedString uart = uBit.serial.read(uBit.serial.getRxBufferSize(), ASYNC);
    if (uart.length() > 0) {
        std::string plain = uart.toCharArray();
        char *cstr = new char[plain.length() + 1];
        strcpy(cstr, plain.c_str());
        char *msg = cryptJson(cstr, 2);
        sendData(msg);
    } else {
        // test sans serveur 
        // sendData(cryptJson("{message: 'LT'}",2));
    }
    free(clear);    
}

int main()
{
    // Initialise the micro:bit runtime.
    
    uBit.init();
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    uBit.radio.enable();
    uBit.radio.setGroup(18);   
    while(1) {
        uBit.display.scroll(".");
        uBit.sleep(1);       
    }
    release_fiber();
}
// bleu = 1
// rose = 2
// https://lancaster-university.github.io/microbit-docs/
