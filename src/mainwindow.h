#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QListWidget>
#include <QGraphicsView>
#include <QTimer>
#include <QStatusBar>
#include <QVector>
#include <QtGlobal>
#include <QDebug>
#include <QFile>
#include <QTextBrowser>
#include <QPlainTextEdit>
#include <QApplication>

#include "session.h"
#include "profile.h"
#include "record.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void connectionTest();
    void changeButtonStatus();
    void initializeSessions();
    void startSession();
    void endSession();
    void drainBattery();
    void sessionGroupLightChange();
    void sessionLightChange(); // if change session group or session, reset session light
    void readRecordingHistory();
    void writeToRecordingHistory();
    void softOff();

private:
    QVector<QVector<Session*>> sessionGroups;
    QVector<Record*> recordings;
    QString recordingPath;

    Profile* profile;
    Session* currentSession;

    bool powerStatus;
    bool leftClipOn;
    bool rightClipOn;

    int currentBatteryLevel;
    int currentTimer;
    int currentSessionGroupIndex;
    int currentSessionIndex;
    int connectionStatus; // 0 for no connection, 1 for okay connection, 2 for excellent connection
    bool currentSessionFlag;
    bool connectionFlag;

    QTimer *connectiontimer;

    Ui::MainWindow *ui;

private slots:
    void powerChange();
    void leftEarClipSensor();
    void rightEarClipSensor();
    void changeBatteryLevel(double);
    void rechargeBattery();
    void switchSessionGroup();
    void switchSession();
    void increaseIntensity();
    void decreaseIntensity();
    void userDesignHandler();
    void updateConnection();
    void saveIntensity();

};
#endif // MAINWINDOW_H
