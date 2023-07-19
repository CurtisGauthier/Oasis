#include "record.h"


Record::Record(QString type, QString group, const int intensity ) {

     
        this->sessionGroup =group;
        this->sessionIntensity = intensity;
        this->sessionType = type;
    
}


QString Record::toString() {
    QString newString =
           "   Session: " + sessionType + "\n"
            + "   Intensity Level: " + QString::number(sessionIntensity) + "\n"
            + "   Duration: " + sessionGroup;

    return newString;
}


// getters

QString Record::getSessionType() { return sessionType; }
int Record::getIntensityLevel() { return sessionIntensity; }
QString Record::getDuration() { return sessionGroup; }



// setters
void Record::setIntensity(int i) { sessionIntensity = i; }
