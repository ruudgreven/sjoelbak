#include <stdio.h>
#include "pico/stdlib.h"
#include "PicoLed/PicoLed.hpp"
#include "hardware/gpio.h"
#include "PicoLedController.hpp"

//PINS
#define PIN_LEDSTRIP_LEFT 18  
#define PIN_LEDSTRIP_RIGHT 19  
#define PIN_LANESENSOR0 2
#define PIN_LANESENSOR1 3
#define PIN_LANESENSOR2 4
#define PIN_LANESENSOR3 5
#define PIN_LANESENSOR4 6
#define PIN_LANESENSOR5 7
#define PIN_LANESENSOR6 9

//DEFINES
#define LEDSTRIP_LENGTH 60
#define LANESENSOR0_OFFSET 17
#define LANESENSORS_LEDS 6

#define PUCK_NOT_SCANNED -1

int currentPuckPostionGate = PUCK_NOT_SCANNED;          
int currentPuckPositionInterpolateLed = PUCK_NOT_SCANNED;

//Sensor callback
void lanesensor_callback(uint gpio, uint32_t events) {
    //printf("GPIO %d %s\n", gpio, events);

    uint mask = (1 << 3);
    if (events & (1 << 3)) {    //Puck appears
        currentPuckPostionGate = gpio - PIN_LANESENSOR0;    //Assume that all lanesensor uses GPIO pins in order
        currentPuckPositionInterpolateLed = LANESENSOR0_OFFSET + (currentPuckPostionGate * LANESENSORS_LEDS);
    }
    if (events & (1 << 4)) {    //Puck disappears
        if (currentPuckPostionGate == gpio) {   //If it was last seen at this position, then it's 'missing' now
            currentPuckPostionGate = PUCK_NOT_SCANNED;
        }
    }
    printf("currentPuckPostionGate: %d", currentPuckPostionGate);
    printf("currentPuckPositionInterpolateLed: %d\r\n", currentPuckPositionInterpolateLed);
}

int ledPosition(uint index) {
    return 59 - index;
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

    //Initilialize lanesensor
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR3, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR4, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR5, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);
    gpio_set_irq_enabled_with_callback(PIN_LANESENSOR6, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &lanesensor_callback);

    while (true) {
        ledStripLeft.fill( PicoLed::RGB(255, 0, 0) );
        ledStripRight.fill( PicoLed::RGB(0, 0, 255) );
        //The puck is not visible
        if (currentPuckPostionGate == PUCK_NOT_SCANNED) {
            //Do some interpolation

        }

        //If there is a puck active
        if (currentPuckPositionInterpolateLed != PUCK_NOT_SCANNED) {
            ledStripLeft.setPixelColor(ledPosition(currentPuckPositionInterpolateLed - 1), PicoLed::RGB(255,255,255));
            ledStripLeft.setPixelColor(ledPosition(currentPuckPositionInterpolateLed), PicoLed::RGB(255,255,255));
            ledStripLeft.setPixelColor(ledPosition(currentPuckPositionInterpolateLed + 1), PicoLed::RGB(255,255,255));
            
            ledStripRight.setPixelColor(ledPosition(currentPuckPositionInterpolateLed - 1), PicoLed::RGB(255,255,255));
            ledStripRight.setPixelColor(ledPosition(currentPuckPositionInterpolateLed), PicoLed::RGB(255,255,255));
            ledStripRight.setPixelColor(ledPosition(currentPuckPositionInterpolateLed + 1), PicoLed::RGB(255,255,255));
        } else {

        }

        ledStripLeft.show();
        ledStripRight.show();
        sleep_ms(5);
    }
}