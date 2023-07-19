#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

/* Purpose of class: Session object to hold information about a session
 *
 * Data Members:
 * -int time: A integer representation of how long a session lasts
 * - QString sessionGroup: a QString representation of the sessions group
 * - QString sessionType : a QString representation of the sessions type
 * - int sessionIntensity : A Integer representation of the session intensity
 * - QTimer* timer: A QTimer object to keep track of how long a session should last
 *
 * Class functions:
 * -getters for data members
 */


class Session : public QObject
{

Q_OBJECT

public:
    Session(int time, QString sessionType, int sessionIntensity, QString sessionGroup);
    ~Session();

    //getters
    int getTime();
    QString getSessionType();
    QString getSessionGroup();
    int getSessionIntensity();
    QTimer* getTimer();

    //setters
    void setSessionIntensity(int intensity);
    void setType(QString type);
    void setGroup(QString group);
    void setTime(int t);


private:
    int time;
    QString sessionType;
    int sessionIntensity;
    QString sessionGroup;
    QTimer* timer;

};

#endif // SESSION_H
