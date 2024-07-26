#include <stdio.h>
#include "pico/stdlib.h"
#include "PicoLed/PicoLed.hpp"
#include "hardware/gpio.h"
#include "PicoLedController.hpp"

//PINS
#define PIN_LEDSTRIP_LEFT 17  
#define PIN_LEDSTRIP_RIGHT 16  
#define PIN_LANESENSOR0 2
#define PIN_LANESENSOR1 3
#define PIN_LANESENSOR2 4
#define PIN_LANESENSOR3 5
#define PIN_LANESENSOR4 6
#define PIN_LANESENSOR5 7
#define PIN_LANESENSOR6 8

//BOARD TYPES
#define BOARDTYPE_REGULAR 1

//DEFINES
#define LEDSTRIP_LENGTH 60
#define LANESENSOR0_OFFSET 17
#define LANESENSORS_LEDS 6

#define PUCK_NOT_SCANNED -1

uint boardtype = BOARDTYPE_REGULAR;

uint currentPuckPostionGate = PUCK_NOT_SCANNED;          
uint currentPuckPositionInterpolateLed = PUCK_NOT_SCANNED;
uint currentPuckLastseenTime = -1;
double currentPuckSpeed = -1;
bool currentPuckVisible = false;
bool startAllowed = true;

//Sensor callback
void lanesensor_callback(uint gpio, uint32_t events) {
    //printf("GPIO %d %s\n", gpio, events);

    uint mask = (1 << 3);
    if (events & (1 << 3)) {    //Puck appears
        currentPuckVisible = true;
        printf("puck:gate: %d\r\n", currentPuckPostionGate);

        if (gpio == PIN_LANESENSOR1) {
            //If lanesensor 1 is passed, save time and set startAllowed to false
            currentPuckLastseenTime = to_ms_since_boot( get_absolute_time() );
            startAllowed = false;
        } else {
            //If other lanesensors passed, calculate traveltime
            if (currentPuckPostionGate != gpio - PIN_LANESENSOR0) {
                uint currentTime = to_ms_since_boot( get_absolute_time() );
                uint travelTime = currentTime - currentPuckLastseenTime;

                currentPuckSpeed = (1 / (travelTime / 10.0)) * 36; //time in ms / 10cm ( = ms/cm * 36 ( = km/h)
                //If last known position is smaller, then speed is negative (bouncing back)
                if (currentPuckPostionGate > gpio - PIN_LANESENSOR0) {
                    currentPuckSpeed = -currentPuckSpeed;
                }
                printf("puck:speed: %f\r\n", currentPuckSpeed);
                currentPuckLastseenTime = to_ms_since_boot( get_absolute_time() );
            }
        }

        //TODO: Remove, and set start allowed when scored
        if (gpio == PIN_LANESENSOR6) {
            startAllowed = true;
        }

        //Set gateposition and interpolated position
        currentPuckPostionGate = gpio - PIN_LANESENSOR0;    //Assume that all lanesensor uses GPIO pins in order
        currentPuckPositionInterpolateLed = LANESENSOR0_OFFSET + (currentPuckPostionGate * LANESENSORS_LEDS);
    }
    if (events & (1 << 4)) {    //Puck disappears
        if (currentPuckPostionGate == gpio) {   //If it was last seen at this position, then it's 'missing' now
            currentPuckVisible = false;
        }
    }
    //printf("currentPuckPositionInterpolateLed: %d\r\n", currentPuckPositionInterpolateLed);
}

int ledPosition(uint index) {
    return 60 - index;
}

int main() {
    const uint led_pin = 25;

    //Initialize
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    //Initialize chosen serial port
    stdio_init_all();

    //Initialize led strips
    auto ledStripLeft = PicoLed::addLeds<PicoLed::WS2812B>(pio0, 0, PIN_LEDSTRIP_LEFT, LEDSTRIP_LENGTH, PicoLed::FORMAT_GRB);
    auto ledStripRight = PicoLed::addLeds<PicoLed::WS2812B>(pio1, 0, PIN_LEDSTRIP_RIGHT, LEDSTRIP_LENGTH, PicoLed::FORMAT_GRB);
    ledStripLeft.fill( PicoLed::RGB(0, 0, 0) );
    ledStripRight.fill( PicoLed::RGB(0, 0, 0) );
    ledStripLeft.show();
    ledStripRight.show();

    //Initilialize lanesensor
    //gpio_set_irq_enabled_with_callback(PIN_LANESENSOR0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR3, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR4, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR5, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR6, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);

    while (true) {
        //Set up startzone
        if (startAllowed) {
            //Start zone yellow
            ledStripLeft.fill( PicoLed::RGB(240, 240, 20), ledPosition(20), 20);
            ledStripRight.fill( PicoLed::RGB(240, 240, 20), ledPosition(20), 20);
        } else {
            //Start zone yellow (something else??)
            ledStripLeft.fill( PicoLed::RGB(240, 240, 20), ledPosition(20), 20);
            ledStripRight.fill( PicoLed::RGB(240, 240, 20), ledPosition(20), 20);
        }

        //Begin bar
        if (startAllowed) {
            //Begin bar green
            ledStripLeft.fill( PicoLed::RGB(20, 255, 20), ledPosition(24), 4);
            ledStripRight.fill( PicoLed::RGB(20, 255, 20), ledPosition(24), 4);
        } else {
            //Begin bar red
            ledStripLeft.fill( PicoLed::RGB(255, 20, 20), ledPosition(24), 4);
            ledStripRight.fill( PicoLed::RGB(255, 20, 20), ledPosition(24), 4);
        }

        //Set up board
        if (boardtype == BOARDTYPE_REGULAR) {
            //The board
            ledStripLeft.fillGradient( PicoLed::RGB(26, 50, 230), PicoLed::RGB(26, 194, 230), 0, 36);
            ledStripRight.fillGradient( PicoLed::RGB(26, 50, 230), PicoLed::RGB(26, 194, 230), 0, 36);
        }


        //The puck is not visible
        if (currentPuckPostionGate == PUCK_NOT_SCANNED) {
            //Do some interpolation

        }

        //If there is a puck active
        if (currentPuckPositionInterpolateLed != PUCK_NOT_SCANNED) {
            //Draw trail
            ledStripLeft.setPixelColor(ledPosition(currentPuckPositionInterpolateLed + 2), PicoLed::RGB(210,210,255));
            ledStripLeft.setPixelColor(ledPosition(currentPuckPositionInterpolateLed + 1), PicoLed::RGB(210,210,255));
            ledStripLeft.setPixelColor(ledPosition(currentPuckPositionInterpolateLed), PicoLed::RGB(70,70,190));
            ledStripLeft.setPixelColor(ledPosition(currentPuckPositionInterpolateLed - 1), PicoLed::RGB(128,128,146));
            
            ledStripRight.setPixelColor(ledPosition(currentPuckPositionInterpolateLed + 2), PicoLed::RGB(210,210,255));
            ledStripRight.setPixelColor(ledPosition(currentPuckPositionInterpolateLed + 1), PicoLed::RGB(210,210,255));
            ledStripRight.setPixelColor(ledPosition(currentPuckPositionInterpolateLed), PicoLed::RGB(70,70,190));
            ledStripRight.setPixelColor(ledPosition(currentPuckPositionInterpolateLed - 1), PicoLed::RGB(128,128,146));
        } else {

        }

        ledStripLeft.show();
        ledStripRight.show();
        sleep_ms(5);
    }
}