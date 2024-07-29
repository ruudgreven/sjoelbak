#include "Board.hpp"
#include "defines.h"
#include "pico/stdlib.h"
#include <stdio.h>

Board::Board(uint boardtype) {
    this->setBoardtype(boardtype);
    this->reset();
}

void Board::setBoardtype(uint boardtype) {
    this->boardtype = boardtype;
    printf("board:setboardtype:%d\r\n", this->boardtype);
}

uint Board::getBoardtype() {
    return this->boardtype;
}

void Board::setAvailable() {
    this->available = true;
    printf("board:available\r\n");
}

void Board::setUnavailable() {
    this->available = false;
    printf("board:unavailable\r\n");
}

bool Board::isAvailable() {
    return this->available;
}

void Board::setLaneSensorPassed(uint lanegateNumber) {
    this->lanesensorPassed[lanegateNumber] = true;
    printf("board:lanesensorpassed:%d\r\n", lanegateNumber);
}

bool Board::isLaneSensorPassed(uint lanegateNumber) {
    return this->lanesensorPassed[lanegateNumber];
}

void Board::setCurrentLanesensor(uint currentLanesensor){
    this->currentLanesensor = currentLanesensor;
}

void Board::unsetCurrentLanesensor() {
    this->currentLanesensor = UNDEFINED;
}

bool Board::getCurrentLaneSensor() {
    return this->currentLanesensor;
}

void Board::setScoregatePassed(uint scoregateNumber) {
    this->scoreGateNumber = scoreGateNumber;
    printf("board:scoregatepassed:%d\r\n", scoregateNumber);
}

bool Board::isScoregatePassed() {
    return this->scoreGateNumber != UNDEFINED;
}

void Board::reset() {
    printf("board:reset\r\n");

    this->available = true;
    for (int i = 0; i < 7; i++) {
        this->lanesensorPassed[i] = false;
    }
    this->scoreGateNumber = UNDEFINED;
    this->currentLanesensor = UNDEFINED;
}