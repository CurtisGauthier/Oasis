#include "session.h"

Session::Session(int t, QString type,int intensity, QString group)
{
    this->time = t;
    this->sessionType = type;
    this->sessionIntensity = intensity;
    this->sessionGroup = group;

    //initalize the timer
    this->timer = new QTimer(this);

    qDebug() << "constructor: " << time << sessionType << sessionGroup;

}

Session::~Session()
{
    delete timer;
}

//Getters
QString Session::getSessionType(){return sessionType;}
int Session::getSessionIntensity(){return sessionIntensity;}
QString Session::getSessionGroup(){return sessionGroup;}
QTimer* Session::getTimer() { return timer; }
int Session::getTime(){return time;}

//Setters
void Session::setSessionIntensity(int intensity){
  sessionIntensity = intensity;
}
void Session::setType(QString type){sessionType = type;}
void Session::setGroup(QString group){sessionGroup = group;}
void Session::setTime(int t){time = t;}
