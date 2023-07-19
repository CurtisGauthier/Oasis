#include "profile.h"

Profile::Profile(int iD, double level, int status) {

    this->id = iD;
    this->batteryLvl = level;
    this->powerStatus = status;
}
//Class Functions


// getters
int Profile::getId() { return id; }
double Profile::getBLvl() { return batteryLvl; }
int Profile::getPowerStatus() { return powerStatus; }


// setters
void Profile::setBLvl(double level) { batteryLvl = level; }
void Profile::setPowerStatus(int status) { powerStatus = status; }
