#ifndef BOARD_H
#define BOARD_H
#include "pico/stdlib.h"

class Board {
    private:
        int boardtype;
        bool available;     //Is false when there is an active puck on the board
        bool lanesensorPassed[7] = {false, false, false, false, false, false, false};
        int currentLanesensor;
        int scoreGateNumber;

    public:
        Board(uint boardtype);

        void setBoardtype(uint boardtype);
        uint getBoardtype();

        void setAvailable();
        void setUnavailable();
        bool isAvailable();

        void setLaneSensorPassed(uint lanegateNumber);
        bool isLaneSensorPassed(uint lanegateNumber);
        void setCurrentLanesensor(uint currentLanesensor);
        void unsetCurrentLanesensor();
        bool getCurrentLaneSensor();

        void setScoregatePassed(uint scoregateNumber);
        bool isScoregatePassed();

        void reset();
};
#endif