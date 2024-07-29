#include "Puck.hpp"
#include "defines.h"
#include "pico/stdlib.h"
#include <stdio.h>

Puck::Puck() {
    this->reset();
}

void Puck::setCurrentVisible() {
    this->puckCurrentVisible = true;
}
void Puck::unsetCurrentVisible() {
    this->puckCurrentVisible = false;
}

bool Puck::isCurrentVisible() {
    return this->puckCurrentVisible;
}

void Puck::setCurrentSpeed(double speed) {
    this->currentSpeed = speed;
    if (speed > this->topSpeed) {
        this->topSpeed = speed;
    }
    printf("puck:currentSpeed:%f\r\n", this->currentSpeed);
}

double Puck::getCurrentSpeed() {
    return this->currentSpeed;
}

double Puck::getTopSpeed() {
    return this->topSpeed;
}

void Puck::setPositionInLeds(uint positionInLeds) {
    this->positionInLeds = positionInLeds;
}

uint Puck::getPositionInLeds() {
    return this->positionInLeds;
}

uint Puck::getPositionInSections() {
    return ((this->positionInLeds - LANESENSOR0_OFFSET_IN_LEDS) / LANESENSORS_NO_OF_LEDS) + 1;
}

void Puck::setLastLanesensorPassed(uint lastLanesensorPassed) {
    this->lastLanesensorPassed = lastLanesensorPassed;
}

uint Puck::getLastLanesensorPassed() {
    return this->lastLanesensorPassed;
}

void Puck::setLost() {
    printf("puck:lost\r\n");
}
        

bool Puck::puckAlreadySeen() {
    return this->currentSpeed != UNDEFINED;
}

bool Puck::isHeadingForward() {
    return this->currentSpeed > 0;
}
bool Puck::isHeadingBackward() {
    return this->currentSpeed < 0;
}

void Puck::reset() {
    printf("puck:reset\r\n");
    this->puckCurrentVisible = false;
    this->currentSpeed = UNDEFINED;
    this->topSpeed = UNDEFINED;
    this->positionInLeds = UNDEFINED;
}