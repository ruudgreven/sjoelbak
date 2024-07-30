#ifndef BOARD_H
#define BOARD_H
#include "pico/stdlib.h"

class Board {
    private:
        int boardtype;
        bool available;     //Is false when there is an active puck on the board
        bool lanesensorPassed[7] = {false, false, false, false, false, false, false};
        bool lanesensorObstructed[7] = {false, false, false, false, false, false, false};

        int currentLanesensor;
        int scoreGateNumber;

    public:
        Board(uint boardtype);

        void setBoardtype(uint boardtype);
        int getBoardtype();

        void setAvailable();
        void setUnavailable();
        bool isAvailable();

        void setLaneSensorPassed(int lanegateNumber);
        bool isLaneSensorPassed(int lanegateNumber);
        void setCurrentLanesensor(int currentLanesensor);
        void unsetCurrentLanesensor();
        int getCurrentLaneSensor();

        void addLaneSensorObstruction(int lanegateNumber);
        void removeLaneSensorObstruction(int lanegateNumber);
        bool isLaneSensorObstructed(int lanegateNumber);

        void setScoregatePassed(int scoregateNumber);
        bool isScoregatePassed();

        void nextTurn();
        void reset();
};
#endif