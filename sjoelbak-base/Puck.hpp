#ifndef PUCK_H
#define PUCK_H
#include "pico/stdlib.h"

class Puck {
    private:
        bool puckCurrentVisible;
        double currentSpeed;
        double topSpeed;
        int positionInLeds;
        int lastLanesensorPassed;
        
    public:
        Puck();

        void setCurrentVisible();
        void unsetCurrentVisible();
        bool isCurrentVisible();

        void setCurrentSpeed(double speed);
        double getCurrentSpeed();
        double getTopSpeed();

        void setPositionInLeds(int positionInLeds);
        int getPositionInLeds();
        int getPositionInSections();

        void setLastLanesensorPassed(int lastLanesensorPassed);
        int getLastLanesensorPassed();

        void setLost();

        bool puckAlreadySeen();
        bool isHeadingForward();
        bool isHeadingBackward();

        void reset();
};
#endif