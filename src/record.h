#ifndef RECORD_H
#define RECORD_H

#include <QString>
#include <QDateTime>
#include <QTime>

/* Purpose of Class: To store information about a session in the database and in mainwindow
 *
 * Data Members:
- 
 * - QString sessionType: A Qstring repesentation of the session frequency
 * - QString sessionGroup: QString representation of the selected time session
 *-  int intensity: a integer representation for the therapy intensity
 * - Class Functions:
 * - Getters and Setters
 *
 */

class Record {

public:

    Record();
    Record(QString type, QString group, const int intensity );
    void setIntensity(int i);
    QString toString();
    int getIntensityLevel();
    QString getDuration();
    QString getSessionType();

private:
    QString sessionType;

    QString sessionGroup;
    int sessionIntensity;

};

#endif // RECORD_H

