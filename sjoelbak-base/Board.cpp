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

int Board::getBoardtype() {
    return this->boardtype;
}

void Board::setAvailable() {
    this->available = true;
    printf("board:available\r\n");
}

void Board::setUnavailable() {
    this->available = false;
    printf("board:unavailable\r\n");
    printf("board:startTurn\r\n");
}

bool Board::isAvailable() {
    return this->available;
}

void Board::setLaneSensorPassed(int lanegateNumber) {
    this->lanesensorPassed[lanegateNumber] = true;
    printf("board:lanesensorpassed:%d\r\n", lanegateNumber);
}

bool Board::isLaneSensorPassed(int lanegateNumber) {
    return this->lanesensorPassed[lanegateNumber];
}

void Board::setCurrentLanesensor(int currentLanesensor){
    this->currentLanesensor = currentLanesensor;
}

void Board::unsetCurrentLanesensor() {
    this->currentLanesensor = UNDEFINED;
}

int Board::getCurrentLaneSensor() {
    return this->currentLanesensor;
}

void Board::addLaneSensorObstruction(int lanegateNumber) {
    if (!isLaneSensorObstructed(lanegateNumber)) {
        lanesensorObstructed[lanegateNumber] = true;
        printf("board:obstruction_added:%d\r\n", lanegateNumber);
    }
}

void Board::removeLaneSensorObstruction(int lanegateNumber) {
    if (isLaneSensorObstructed(lanegateNumber)) {
        lanesensorObstructed[lanegateNumber] = false;
        printf("board:obstruction_removed:%d\r\n", lanegateNumber);
    }
}


bool Board::isLaneSensorObstructed(int lanegateNumber) {
    return lanesensorObstructed[lanegateNumber];
}


void Board::setScoregatePassed(int scoregateNumber) {
    this->scoreGateNumber = scoreGateNumber;
    printf("board:scoregatepassed:%d\r\n", scoregateNumber);
}

bool Board::isScoregatePassed() {
    return this->scoreGateNumber != UNDEFINED;
}

void Board::nextTurn() {
    printf("board:nextTurn\r\n");

    for (int i = 0; i < 7; i++) {
        this->lanesensorPassed[i] = false;
    }
    this->scoreGateNumber = UNDEFINED;
    this->currentLanesensor = UNDEFINED;
    this->setAvailable();
}

void Board::reset() {
    printf("board:reset\r\n");
    for (int i = 0; i < 7; i++) {
        this->lanesensorObstructed[i] = false;
    }
    for (int i = 0; i < 7; i++) {
        this->lanesensorPassed[i] = false;
    }
    this->scoreGateNumber = UNDEFINED;
    this->currentLanesensor = UNDEFINED;
    this->setAvailable();
}