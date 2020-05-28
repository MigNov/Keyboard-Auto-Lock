/*
 * BadUSB test device
 * 
 * Copyright (c) 2020 Michal "Mig" Novotny
 * 
 * Documentation:
 * https://www.arduino.cc/reference/en/language/functions/usb/keyboard/
 * https://arduino.stackexchange.com/questions/23792/press-the-windows-key-using-keyboard-press
 * 
 * USB Device: Lily GO USB
 * Board in Arduino IDE: Arduino Leonardo
 *  
 * Turn keyboard device off in Linux:
 * watch -n 1 "xinput" to check ID
 * watch -n 1 "xinput disable <ID>"
 */
#include <Keyboard.h>

// use this option for OSX or Windows:
char ctrlKey = KEY_LEFT_GUI;
// use this option for Linux:
char ctrlKey2 = KEY_LEFT_CTRL;

void keyboard_test()
{
    Keyboard.println();
    Keyboard.println("IT WORKS!!! :-)");
    Keyboard.println();
    Keyboard.println("I wrote this message as a keyboard device!");
    Keyboard.println();
    Keyboard.println("Your BadUSB device :-)");
    Keyboard.println();
    Keyboard.end();
}

void lock_station()
{
    // Lock linux stations
    Keyboard.press(ctrlKey);
    Keyboard.press('l');
    delay(100);
    Keyboard.releaseAll();
    // Lock Windows/OSX stations
    Keyboard.press(ctrlKey2);
    Keyboard.press('l');
    delay(100);
    Keyboard.releaseAll();
}

void setup() 
{
    delay(10000); // wait 10 seconds before doing anything
    Keyboard.begin();
}

void loop()
{
    delay(5000);
    keyboard_test();
    //delay(5000);
    //lock_station();
}
