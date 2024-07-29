#include "defines.h"
#include "Board.hpp"
#include "Puck.hpp"

#include <stdio.h>
#include "pico/stdlib.h"
#include "PicoLed/PicoLed.hpp"
#include "hardware/gpio.h"
#include "PicoLedController.hpp"

int currentPuckLastseenTime = UNDEFINED;
int currentPuckStartTime = UNDEFINED;

Board board = Board(UNDEFINED); 
Puck puck = Puck();

/**
 * Callback function for lanesensors
 */
void lanesensor_callback(uint gpio, uint32_t events) {
    int lanesensor = gpio - PIN_LANESENSOR0;

    uint mask = (1 << 3);
    if (events & (1 << 3)) {    //Puck appears
        puck.setCurrentVisible();
        board.setCurrentLanesensor(lanesensor);

        if (gpio == PIN_LANESENSOR1) {
            if (currentPuckLastseenTime != UNDEFINED) {
                //When lanesensor1 is passed for the first time start the throw timer
                currentPuckLastseenTime = to_ms_since_boot( get_absolute_time() );
                currentPuckStartTime = to_ms_since_boot( get_absolute_time() );

                //Set board status
                board.setUnavailable();
                board.setLaneSensorPassed(lanesensor);
                puck.setLastLanesensorPassed(lanesensor);
                puck.setCurrentSpeed(0);
            } else {
                //When the puck is on it's way back and passing lanesensor0
                currentPuckLastseenTime = UNDEFINED;
                puck.reset();
                board.reset();
            }
        } else {
            //If other lanesensors passed, calculate traveltime
            if (puck.getLastLanesensorPassed() != lanesensor) {
                uint currentTime = to_ms_since_boot( get_absolute_time() );
                uint travelTime = currentTime - currentPuckLastseenTime;

                double currentPuckSpeed = (1 / (travelTime / 10.0)) * 36; //time in ms / 10cm ( = ms/cm * 36 ( = km/h)
                //If last known position is smaller, then speed is negative (bouncing back)
                if (puck.getLastLanesensorPassed() > lanesensor) {
                    currentPuckSpeed = -currentPuckSpeed;
                }
                puck.setCurrentSpeed(currentPuckSpeed);
                currentPuckLastseenTime = to_ms_since_boot( get_absolute_time() );
            }
        }

        //TODO: Remove, and set board to available when scored
        if (gpio == PIN_LANESENSOR6) {
            board.reset();
            puck.reset();
        }

        //Set gateposition and interpolated position
        puck.setLastLanesensorPassed(lanesensor);
        board.setLaneSensorPassed(lanesensor);
        puck.setPositionInLeds(LANESENSOR0_OFFSET_IN_LEDS + (lanesensor * LANESENSORS_NO_OF_LEDS));
    }

    if (events & (1 << 4)) {    //Puck disappears
        puck.unsetCurrentVisible();
        board.unsetCurrentLanesensor();
    }
}

int ledPosition(uint index) {
    return 60 - index;
}

int main() {
    board.setBoardtype(BOARDTYPE_REGULAR);

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
        //The puck is not visible
        if (puck.isCurrentVisible()) {
            //Do some interpolation

        }

        //Reset the puck and board if the puck isn't seen for 4 seconds
        if (currentPuckStartTime != UNDEFINED && ( to_ms_since_boot(get_absolute_time()) - currentPuckStartTime) > 4000) {
            if (board.getCurrentLaneSensor() != UNDEFINED) {
                printf("system:obstruction:%d\r\n", board.getCurrentLaneSensor());
            }
            board.reset();
            puck.setLost();
            puck.reset();
            currentPuckLastseenTime = UNDEFINED;
            currentPuckStartTime = UNDEFINED;
        }

        //START DRAWING
        //Set up startzone and begin bar
        if (board.isAvailable()) {
            //Start zone yellow
            ledStripLeft.fill( PicoLed::RGB(240, 240, 20), ledPosition(20), 20);
            ledStripRight.fill( PicoLed::RGB(240, 240, 20), ledPosition(20), 20);

            //Begin bar green
            ledStripLeft.fill( PicoLed::RGB(20, 255, 20), ledPosition(24), 4);
            ledStripRight.fill( PicoLed::RGB(20, 255, 20), ledPosition(24), 4);
        } else {
            //Start zone yellow (something else??)
            ledStripLeft.fill( PicoLed::RGB(240, 240, 20), ledPosition(20), 20);
            ledStripRight.fill( PicoLed::RGB(240, 240, 20), ledPosition(20), 20);

            //Begin bar red
            ledStripLeft.fill( PicoLed::RGB(255, 20, 20), ledPosition(24), 4);
            ledStripRight.fill( PicoLed::RGB(255, 20, 20), ledPosition(24), 4);
        }

        //Set up board
        if (board.getBoardtype() == BOARDTYPE_REGULAR) {
            //The board
            ledStripLeft.fillGradient( PicoLed::RGB(26, 50, 230), PicoLed::RGB(26, 194, 230), 0, 36);
            ledStripRight.fillGradient( PicoLed::RGB(26, 50, 230), PicoLed::RGB(26, 194, 230), 0, 36);
        }

        //If there is a puck active
        if (puck.puckAlreadySeen()) {
            //Draw trail
            ledStripLeft.setPixelColor(ledPosition(puck.getPositionInLeds() + 2), PicoLed::RGB(210,210,255));
            ledStripLeft.setPixelColor(ledPosition(puck.getPositionInLeds() + 1), PicoLed::RGB(210,210,255));
            ledStripLeft.setPixelColor(ledPosition(puck.getPositionInLeds()), PicoLed::RGB(70,70,190));
            ledStripLeft.setPixelColor(ledPosition(puck.getPositionInLeds() - 1), PicoLed::RGB(128,128,146));
            
            ledStripRight.setPixelColor(ledPosition(puck.getPositionInLeds() + 2), PicoLed::RGB(210,210,255));
            ledStripRight.setPixelColor(ledPosition(puck.getPositionInLeds() + 1), PicoLed::RGB(210,210,255));
            ledStripRight.setPixelColor(ledPosition(puck.getPositionInLeds()), PicoLed::RGB(70,70,190));
            ledStripRight.setPixelColor(ledPosition(puck.getPositionInLeds() - 1), PicoLed::RGB(128,128,146));
        } else {

        }

        ledStripLeft.show();
        ledStripRight.show();
        sleep_ms(5);
    }
}