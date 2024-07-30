#include "defines.h"
#include "Board.hpp"
#include "Puck.hpp"

#include <stdio.h>
#include "pico/stdlib.h"
#include "PicoLed/PicoLed.hpp"
#include "hardware/gpio.h"
#include "PicoLedController.hpp"

int currentPuckLastseenTime = UNDEFINED;
int currentPuckLastseenPosition = UNDEFINED;
int currentPuckStartTime = UNDEFINED;

Board board = Board(UNDEFINED); 
Puck puck = Puck();

/**
 * Function to set a led position (because the strips are 'the other way around'. Led 0 is near the gates, Led 60 at the start)
 */
int ledPosition(uint index) {
    return 60 - index;
}

/**
 * Function to reset the board and the puck and all the timers
 */
void nextTurn() {
    puck.reset();
    board.nextTurn();
    currentPuckLastseenTime = UNDEFINED;
    currentPuckLastseenPosition = UNDEFINED;
    currentPuckStartTime = UNDEFINED;
}

/**
 * Callback function for lanesensors. Assume that lanesensors are connected to adjacent ports.
 */
void lanesensor_callback(uint gpio, uint32_t events) {
    int lanesensor = gpio - PIN_LANESENSOR0;

    if (events & (1 << 2)) {    //Puck appears
        //Ignore appear events if the lanesensor is obstructed
        if (board.isLaneSensorObstructed(lanesensor)) {
            return;
        }

        puck.setCurrentVisible();
        board.setCurrentLanesensor(lanesensor);
        bool validPass = true;

        if (gpio == PIN_LANESENSOR1 || (board.isLaneSensorObstructed(1) && gpio == PIN_LANESENSOR2) || (board.isLaneSensorObstructed(2) && gpio == PIN_LANESENSOR3)) {
            if (currentPuckLastseenTime == UNDEFINED) {
                //When lanesensor1 is passed for the first time start the throw timer
                currentPuckLastseenPosition = lanesensor;
                currentPuckLastseenTime = to_ms_since_boot( get_absolute_time() );
                currentPuckStartTime = to_ms_since_boot( get_absolute_time() );

                //Set board status
                board.setUnavailable();
                puck.setLastLanesensorPassed(lanesensor);
                puck.setCurrentSpeed(UNDEFINED);
            } else {
                //When the puck is on it's way back and passing lanesensor0 reset the board
                nextTurn();
            }
        } else {
            //If other lanesensors passed, calculate traveltime
            if (currentPuckStartTime != UNDEFINED && puck.getLastLanesensorPassed() != lanesensor) {
                uint currentTime = to_ms_since_boot( get_absolute_time() );
                uint travelTime = currentTime - currentPuckLastseenTime;
                uint noOfLanesPassed = lanesensor - currentPuckLastseenPosition;

                double currentPuckSpeed = (1 / (travelTime / 10.0)) * 36 * noOfLanesPassed; //time in ms / 10cm ( = ms/cm * 36 ( = km/h)
                //If last known position is smaller, then speed is negative (bouncing back)
                if (puck.getLastLanesensorPassed() > lanesensor) {
                    currentPuckSpeed = -currentPuckSpeed;
                }
                puck.setCurrentSpeed(currentPuckSpeed);

                currentPuckLastseenTime = to_ms_since_boot( get_absolute_time() );
                currentPuckLastseenPosition = lanesensor;
            } else {
                validPass = false;
            }
        }

        if (validPass) {
            //Set gateposition and interpolated position
            puck.setLastLanesensorPassed(lanesensor);
            board.setLaneSensorPassed(lanesensor);
            puck.setPositionInLeds(LANESENSOR0_OFFSET_IN_LEDS + (lanesensor * LANESENSORS_NO_OF_LEDS));

            //TODO: Remove, and set board to available when scored
            if (gpio == PIN_LANESENSOR6) {
                if (board.getBoardtype() == BOARDTYPE_3ZONES) {
                    printf("game:puckPositionInSections:%d\r\n", -1);
                }
                nextTurn();
            }
        }
    }

    if (events & (1 << 3)) {    //Puck disappears
        puck.unsetCurrentVisible();
        board.unsetCurrentLanesensor();
        if (board.isLaneSensorObstructed(lanesensor)) {
            board.removeLaneSensorObstruction(lanesensor);
        }
    }
}

int main() {
    board.setBoardtype(BOARDTYPE_3ZONES);

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
            if (board.getBoardtype() == BOARDTYPE_3ZONES) {
                printf("game:puckPositionInSections:%d\r\n", puck.getPositionInSections()/2);
            }

            if (board.getCurrentLaneSensor() != UNDEFINED) {
                board.addLaneSensorObstruction(board.getCurrentLaneSensor());
            }
            puck.setLost();
            nextTurn();
        }

        //START DRAWING

        //Set up board
        if (board.getBoardtype() == BOARDTYPE_REGULAR_ICE) {
            //Start zone
            ledStripLeft.fill( PicoLed::RGB(26, 50, 230), ledPosition(20), 20);
            ledStripRight.fill( PicoLed::RGB(26, 50, 230), ledPosition(20), 20);

            //Board
            ledStripLeft.fillGradient( PicoLed::RGB(26, 50, 230), PicoLed::RGB(26, 194, 230), 0, 36);
            ledStripRight.fillGradient( PicoLed::RGB(26, 50, 230), PicoLed::RGB(26, 194, 230), 0, 36);
        } else if (board.getBoardtype() == BOARDTYPE_3ZONES) {
            //Start zone
            ledStripLeft.fill( PicoLed::RGB(50, 50, 50), ledPosition(20), 20);
            ledStripRight.fill( PicoLed::RGB(50, 50, 50), ledPosition(20), 20);

            //Board
            ledStripLeft.fillGradient( PicoLed::RGB(80, 217, 38), PicoLed::RGB(13, 242, 40), 0, 6);
            ledStripRight.fillGradient( PicoLed::RGB(80, 217, 38), PicoLed::RGB(13, 242, 40), 0, 6);
            ledStripLeft.fillGradient( PicoLed::RGB(13, 242, 40), PicoLed::RGB(80, 217, 38), 6, 6);
            ledStripRight.fillGradient( PicoLed::RGB(13, 242, 40), PicoLed::RGB(80, 217, 38), 6, 6);

            ledStripLeft.fillGradient( PicoLed::RGB(217, 164, 38), PicoLed::RGB(242, 175, 13), 12, 6);
            ledStripRight.fillGradient( PicoLed::RGB(217, 164, 38), PicoLed::RGB(242, 175, 13), 12, 6);
            ledStripLeft.fillGradient( PicoLed::RGB(242, 175, 13), PicoLed::RGB(217, 164, 38), 18, 6);
            ledStripRight.fillGradient( PicoLed::RGB(242, 175, 13), PicoLed::RGB(217, 164, 38), 18, 6); 

            ledStripLeft.fillGradient( PicoLed::RGB(204, 51, 51), PicoLed::RGB(230, 26, 26), 24, 6);
            ledStripRight.fillGradient( PicoLed::RGB(204, 51, 51), PicoLed::RGB(230, 26, 26), 24, 6);  
            ledStripLeft.fillGradient( PicoLed::RGB(230, 26, 26), PicoLed::RGB(204, 51, 51), 30, 6);
            ledStripRight.fillGradient( PicoLed::RGB(230, 26, 26), PicoLed::RGB(204, 51, 51), 30, 6);           
        }
        
        if (board.isAvailable()) {
            //Begin bar green
            ledStripLeft.fill( PicoLed::RGB(20, 255, 20), ledPosition(24), 4);
            ledStripRight.fill( PicoLed::RGB(20, 255, 20), ledPosition(24), 4);
        } else {
            //Begin bar red
            ledStripLeft.fill( PicoLed::RGB(255, 20, 20), ledPosition(24), 4);
            ledStripRight.fill( PicoLed::RGB(255, 20, 20), ledPosition(24), 4);
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
        }

        ledStripLeft.show();
        ledStripRight.show();
        sleep_ms(5);
    }
}