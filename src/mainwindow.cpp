#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //initialize a profile
    profile = new Profile(1, 100, 0);
    currentBatteryLevel = profile->getBLvl();
    qDebug() << "battery level: " << currentBatteryLevel;

    //initalize default session
    currentSession = new Session(0,"SessionType", 1,"sessionGroup");

    // initialize variables
    currentTimer = -1;
    currentBatteryLevel = 100.00;
    currentSessionIndex = -1;
    currentSessionGroupIndex = -1;
    leftClipOn = false; // ear clips default off
    rightClipOn = false;
    connectionStatus = 0;
    powerStatus = false;
    currentSessionFlag = false;
    connectionFlag = false;
    connectiontimer = new QTimer(this);

    changeButtonStatus();
    sessionGroupLightChange();
    sessionLightChange();

    //initialize sessions
    initializeSessions();

    //set ui
    ui->powerLight->setVisible(false);


    //connect buttons
    connect(ui->powerButton, &QPushButton::released, this, &MainWindow::powerChange);
    connect(ui->leftEarClipButton, &QPushButton::released, this, &MainWindow::leftEarClipSensor);
    connect(ui->rightEarClipButton, &QPushButton::released, this, &MainWindow::rightEarClipSensor);
    connect(ui->rechargeBatteryButton, &QPushButton::released, this, &MainWindow::rechargeBattery);
    connect(ui->switchSessionGroupButton, &QPushButton::released, this, &MainWindow::switchSessionGroup);
    connect(ui->switchSessionButton, &QPushButton::released, this, &MainWindow::switchSession);
    connect(ui->selectButton, &QPushButton::released, this, &MainWindow::startSession);
    connect(ui->recordButton, &QPushButton::released, this, &MainWindow::writeToRecordingHistory);
    connect(ui->updateConnectionButton, &QPushButton::released, this, &MainWindow::updateConnection);
    connect(ui->favouriteIntensityButton, &QPushButton::released, this, &MainWindow::saveIntensity);

    // initialize battery levels
    ui->batterySpinBox->setValue(profile->getBLvl());

    //initalize increase/decrease intensity arrows
    connect(ui->upButton, &QPushButton::released, this, &MainWindow::increaseIntensity);
    connect(ui->downButton, &QPushButton::released, this, &MainWindow::decreaseIntensity);

    // initialize DAVID user mode designer
    connect(ui->designFinishButton, &QPushButton::released, this, &MainWindow::userDesignHandler);

    //change battery level from admin panel
    connect(ui->batterySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::changeBatteryLevel);

    //initialize recording history path
    QString recordingPath = qApp->applicationDirPath() + "/recording_history.txt";

    //if no operation in 2 minutes, the device will turn off
    qDebug() << "WARNING: if no operation in 2 minutes, the device will turn off.";
}

MainWindow::~MainWindow()
{
    delete ui;
    delete currentSession;
    delete profile;
}

void MainWindow::changeButtonStatus()
{
    // enable or disable all the buttons upon power change
    ui->upButton->setEnabled(powerStatus);
    ui->downButton->setEnabled(powerStatus);
    ui->switchSessionButton->setEnabled(powerStatus);
    ui->switchSessionGroupButton->setEnabled(powerStatus);
    ui->selectButton->setEnabled(powerStatus);
    ui->recordButton->setEnabled(powerStatus);
    ui->batterySpinBox->setEnabled(powerStatus);
    ui->leftEarClipButton->setEnabled(powerStatus);
    ui->rightEarClipButton->setEnabled(powerStatus);
    ui->userDesignedSpinBox->setEnabled(powerStatus);
    ui->designFinishButton->setEnabled(powerStatus);
    ui->rechargeBatteryButton->setEnabled(powerStatus);
    ui->connectionSelectBox->setEnabled(powerStatus);
    ui->updateConnectionButton->setEnabled(powerStatus);

    //turn off lights when power off
    if(!powerStatus){
        sessionGroupLightChange();
        sessionLightChange();
        ui->leftCESChannel->setStyleSheet("border-image: url(:/icons/left.svg);");
        ui->rightCESChannel->setStyleSheet("border-image: url(:/icons/right.svg);");
    }
}

void MainWindow::powerChange()
{
    // if no session is on , turn off directly
    // if session is on, turn off session first, then turn device off
    if (currentBatteryLevel > 0)
    {
        powerStatus = !powerStatus;
        // change light color
        ui->powerLight->setVisible(powerStatus);
        changeButtonStatus();
        // read record
        if(powerStatus) readRecordingHistory();
        //if session is going on, do a soft off
        if(currentSessionFlag && !powerStatus) softOff();
    }
}

void MainWindow::softOff()
{
    qDebug() << "Soft Off in progress. The session will end gradually.";
}

void MainWindow::leftEarClipSensor()
{
    leftClipOn = !leftClipOn;
    // set light color
    if (leftClipOn) {
        ui->leftCESChannel->setStyleSheet("border-image: url(:/icons/left_2.svg);");
    } else {
        ui->leftCESChannel->setStyleSheet("border-image: url(:/icons/left.svg);");
    }

    if(currentSessionFlag && !leftClipOn){
        qDebug()<< "Ear clip disconnected. The session is stopping soon.";
        QTimer::singleShot(3000,this, &MainWindow::powerChange);
    }

}

void MainWindow::rightEarClipSensor()
{
    rightClipOn = !rightClipOn;
    // set light color
    if (rightClipOn) {
        ui->rightCESChannel->setStyleSheet("border-image: url(:/icons/right_2.svg);");
    } else {
        ui->rightCESChannel->setStyleSheet("border-image: url(:/icons/right.svg);");
    }

    if(currentSessionFlag && !rightClipOn){
        qDebug()<< "Ear clip disconnected. The session is stopping soon.";
        QTimer::singleShot(3000,this, &MainWindow::powerChange);
    }
}

void MainWindow::rechargeBattery()
{
    profile->setBLvl(100.00);
    currentBatteryLevel = profile->getBLvl();
    ui->batterySpinBox->setValue(profile->getBLvl());
}

void MainWindow::changeBatteryLevel(double newLevel)
{
    if (newLevel >= 0.0 && newLevel <= 100.0) {
        // if power out, turn off the device
        if (newLevel == 0.0 && powerStatus == true) {
            powerChange();
            profile->setBLvl(0);
        }else{
            profile->setBLvl(newLevel);
            currentBatteryLevel = profile->getBLvl();
            qDebug() << "new battery level: " << currentBatteryLevel;
        }
        // display to admin panel
        ui->batterySpinBox->setValue(newLevel);
        // display low battery warning
        if (newLevel <= 20.0) qDebug() << "Battery below 20 percent.";
        if (newLevel <= 10.0) qDebug() << "Battery below 10 percent. Replace the battery immediately. ";
    }
}

void MainWindow::connectionTest()
{
    if(currentSessionFlag){
        QString quality = ui->connectionSelectBox->currentText();
        qDebug() << "User select test connection is: " << quality;

        // if both ear clips on and user select "excellent" connection
        if(leftClipOn && rightClipOn && quality == "Excellent"){
            sessionLightChange();
            ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
            ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
            ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
            if(currentSession->getSessionType() == "Sub-Delta"){
                ui->CESModeTwo->setStyleSheet("border-image: url(:/icons/mode_two_2.svg);");
            }else{
                ui->CESModeOne->setStyleSheet("border-image: url(:/icons/mode_one_2.svg);");
            }
            connectionFlag = true;
            qDebug() <<"You may now adjust the intensity to a desired level";
        // if both ear clips on and user select "okay" connection
        }else if(leftClipOn && rightClipOn && quality == "Okay"){
            sessionLightChange();
            ui->levelSix->setStyleSheet("border-image: url(:/icons/six_2.svg);");
            ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
            ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
            if(currentSession->getSessionType() == "Sub-Delta"){
                ui->CESModeTwo->setStyleSheet("border-image: url(:/icons/mode_two_2.svg);");
            }else{
                ui->CESModeOne->setStyleSheet("border-image: url(:/icons/mode_one_2.svg);");
            }
            connectionFlag = true;
            qDebug() <<"You may now adjust the intensity to a desired level";
        // if either of the earclips is disconnected
        }else if(!leftClipOn || !rightClipOn){
            sessionLightChange();
            ui->levelEight->setStyleSheet("border-image: url(:/icons/eight_2.svg);");
            ui->levelSeven->setStyleSheet("border-image: url(:/icons/seven_2.svg);");
            connectionFlag = false;
            qDebug() <<"ERROR: Bad connection. Please attach an earpiece";
            qDebug() <<"Device will be powered off after 8 seconds.";

            // set timer 8 sec until power off
            connectiontimer->setInterval(8000);
            connectiontimer->setSingleShot(true);
            connect(connectiontimer,&QTimer::timeout, this, &MainWindow::powerChange);
            connectiontimer->start();
        }
     }
}

void MainWindow::updateConnection(){
    if(!connectionFlag && leftClipOn && rightClipOn)
    {
        qDebug() << "updated connection";
        connectiontimer->stop();
        startSession();
    }else{
        qDebug() << "no update on connection";
    }
}

void MainWindow::initializeSessions()
{
    sessionGroups.resize(3);
    int t;
    QString g;

    for (int i=0; i<3; i++)
    {
        sessionGroups[i].resize(4);
        qDebug() << "session group " << i << " initiazlied.";
        switch (i) {
        case 0:
            t = 20;
            g = "20 Minute Session";
            break;
        case 1:
            t = 45;
            g = "45 Minute Session";
            break;
        default:
            t = 0; // set user designed mode default=0
            g = "User Designed Session";
            break;
        }
        sessionGroups[i][0] = new Session(t, "Sub-Delta", 1, g);
        sessionGroups[i][1] = new Session(t, "Delta", 1, g);
        sessionGroups[i][2] = new Session(t, "Theta", 1, g);
        sessionGroups[i][3] = new Session(t, "Alpha", 1, g);
    }
}

void MainWindow::sessionGroupLightChange()
{
    ui->sessionGroupThree->setStyleSheet("border-image: url(:/icons/userDesigned.png);");
    ui->sessionGroupOne->setStyleSheet("border-image: url(:/icons/20minute.png);");
    ui->sessionGroupTwo->setStyleSheet("border-image: url(:/icons/45minute.png);");
}

void MainWindow::switchSessionGroup()
{
    currentSessionGroupIndex++;
    switch (currentSessionGroupIndex) {
    case 0:
        sessionGroupLightChange();
        ui->sessionGroupOne->setStyleSheet("border-image: url(:/icons/20minute_2.png);");
        break;
    case 1:
        sessionGroupLightChange();
        ui->sessionGroupTwo->setStyleSheet("border-image: url(:/icons/45minute_2.png);");
        break;
    case 2:
        sessionGroupLightChange();
        ui->sessionGroupThree->setStyleSheet("border-image: url(:/icons/userDesigned_2.png);");
        break;
    case 3:
        sessionGroupLightChange();
        currentSessionGroupIndex = 0;
        ui->sessionGroupOne->setStyleSheet("border-image: url(:/icons/20minute_2.png);");
        break;
    default:
        break;
    }
}

void MainWindow::sessionLightChange()
{
    //reset session light on top left
    ui->sessionOneLight->setStyleSheet("color: rgb(230, 230, 230);");
    ui->sessionTwoLight->setStyleSheet("color: rgb(230, 230, 230);");
    ui->sessionThreeLight->setStyleSheet("color: rgb(230, 230, 230);");
    ui->sessionFourLight->setStyleSheet("color: rgb(230, 230, 230);");
    //reset frequency light
    ui->frequencyOne->setStyleSheet("color: rgb(230, 230, 230);");
    ui->frequencyTwo->setStyleSheet("color: rgb(230, 230, 230);");
    ui->frequencyThree->setStyleSheet("color: rgb(230, 230, 230);");
    ui->frequencyFour->setStyleSheet("color: rgb(230, 230, 230);");
    //reset level light
    ui->levelOne->setStyleSheet("border-image: url(:/icons/one.svg);");
    ui->levelTwo->setStyleSheet("border-image: url(:/icons/two.svg);");
    ui->levelThree->setStyleSheet("border-image: url(:/icons/three.svg);");
    ui->levelFour->setStyleSheet("border-image: url(:/icons/four.svg);");
    ui->levelFive->setStyleSheet("border-image: url(:/icons/five.svg);");
    ui->levelSix->setStyleSheet("border-image: url(:/icons/six.svg);");
    ui->levelSeven->setStyleSheet("border-image: url(:/icons/seven.svg);");
    ui->levelEight->setStyleSheet("border-image: url(:/icons/eight.svg);");
}

void MainWindow::switchSession()
{
    currentSessionIndex++;
    switch (currentSessionIndex) {
    case 0:
        sessionLightChange();
        ui->sessionOneLight->setStyleSheet("color: rgb(255, 208, 0);");
        ui->frequencyOne->setStyleSheet("color: rgb(255, 208, 0);");
        ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
    case 1:
        sessionLightChange();
        ui->sessionTwoLight->setStyleSheet("color: rgb(255, 208, 0);");
        ui->frequencyTwo->setStyleSheet("color: rgb(255, 208, 0);");
        ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
        break;
    case 2:
        sessionLightChange();
        ui->sessionThreeLight->setStyleSheet("color: rgb(255, 208, 0);");
        ui->frequencyThree->setStyleSheet("color: rgb(255, 208, 0);");
        ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
        break;
    case 3:
        sessionLightChange();
        ui->sessionFourLight->setStyleSheet("color: rgb(255, 208, 0);");
        ui->frequencyFour->setStyleSheet("color: rgb(255, 208, 0);");
        ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
        break;
    case 4:
        sessionLightChange();
        currentSessionIndex = 0;
        ui->sessionOneLight->setStyleSheet("color: rgb(255, 208, 0);");
        ui->frequencyOne->setStyleSheet("color: rgb(255, 208, 0);");
        ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
    default:
        break;
    }
}
//timer running for our duration in seconds
void MainWindow::startSession(){

    if (currentSessionIndex > -1 && currentSessionGroupIndex > -1)
    {
        //start the session after a session has been selected
        // step 0: if user designed session, ask for user input number

        //step 0.1: set the current session to the session that was selected
        currentSession = sessionGroups[currentSessionGroupIndex][currentSessionIndex];
        if (currentSession->getTime() == 0){
           userDesignHandler();
           qDebug() << "User design time is: " << currentSession->getTime();
        }
        //set current session flag
        currentSessionFlag = true;

        // step 1: do connection test
        connectionTest();

        //check to see if we have enough battery, if we dont reset the flag and warn user to charge the battery
        double checkBatteryUsage = 0.0;
        checkBatteryUsage = currentSession->getSessionIntensity() + currentSession->getTime();
        if(currentBatteryLevel-checkBatteryUsage < 10){
            qDebug() << "WARNING: Not enough battery for selected session. Please recharge the batterys";
            connectionFlag = false;
        }
        // step 2: do 5 sec count down
        if(connectionFlag){
            qDebug() << "Beggining Session in 5 seconds: ";
            for(int i = 5; i>0; i--){
                qDebug() << i;
            }
            // step 3: start the session
            qDebug() <<currentSession->getSessionGroup() << currentSession->getSessionType()<< "has started for"<< currentSession->getTime()<<"minutes with instensity set at:" << currentSession->getSessionIntensity();
            QTimer::singleShot(currentSession->getTime()*1000, this, &MainWindow::endSession);
        }
    }
}
void MainWindow::endSession(){
    drainBattery();
    qDebug() <<currentSession->getSessionGroup() << currentSession->getSessionType()<< "has ended after"<< currentSession->getTime()<<"minutes with final instensity set at:" << currentSession->getSessionIntensity();

    currentSessionFlag = false;
    connectionFlag = false;
    //implement softoff function
    powerChange();
}
void MainWindow::userDesignHandler()
{
    int userInputTime;
    userInputTime = ui->userDesignedSpinBox->value();
    currentSession->setTime(userInputTime);
}


void MainWindow::drainBattery(){
    //if the power is greater than 0 and the power is on, then decrement the battery level
    //how do we do this concurrently while the power is on?

    //using a simple function of the sum of intensity plus the length of time of the session to drain the battery ---> naive formula
    //if user designated a time greater than (100-intesnity) then it might break
    double batteryUsage = 0.0;
    if(currentSessionFlag){
        batteryUsage = currentSession->getSessionIntensity() + currentSession->getTime();
        if(currentBatteryLevel - batteryUsage <= 10){
            qDebug() <<"Not enough power to start the session";
        }else{
            batteryUsage = currentBatteryLevel - batteryUsage;
            changeBatteryLevel(batteryUsage);
        }
    }
}

void MainWindow::decreaseIntensity(){
    //if we are in a session, use the INT down arrow to decrease intensity
    //Light up the corresponding bar graph LED and make the topmost LED on the bargraph blink
    if(currentSessionFlag && connectionFlag){
        if(currentSession->getSessionIntensity() != 1){
            currentSession->setSessionIntensity(currentSession->getSessionIntensity() - 1);
            qDebug() << "The current intensity is: " << currentSession->getSessionIntensity();
            switch (currentSession->getSessionIntensity()) {
            case 1:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                break;
            case 2:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                break;
            case 3:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                break;
            case 4:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                break;
            case 5:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
                break;
            case 6:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
                ui->levelSix->setStyleSheet("border-image: url(:/icons/six_2.svg);");
                break;
            case 7:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
                ui->levelSix->setStyleSheet("border-image: url(:/icons/six_2.svg);");
                ui->levelSeven->setStyleSheet("border-image: url(:/icons/seven_2.svg);");
                break;
            case 8:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
                ui->levelSix->setStyleSheet("border-image: url(:/icons/six_2.svg);");
                ui->levelSeven->setStyleSheet("border-image: url(:/icons/seven_2.svg);");
                ui->levelEight->setStyleSheet("border-image: url(:/icons/eight_2.svg);");
            default:
                break;
            }
        }
    }else{
        qDebug() << "Please select a session before decreasing intensity and make sure connection test is finished";
    }
}
void MainWindow::increaseIntensity(){
    //if we are in a session, use the INT up arrow to increase intensity
    //show it on the bar graph for a brief period

    if(currentSessionFlag && connectionFlag){
        if(currentSession->getSessionIntensity() != 8){
            currentSession->setSessionIntensity(currentSession->getSessionIntensity() + 1);
            qDebug() << "The current intensity is: " << currentSession->getSessionIntensity();
            switch (currentSession->getSessionIntensity()) {
            case 1:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                break;
            case 2:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                break;
            case 3:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                break;
            case 4:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                break;
            case 5:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
                break;
            case 6:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
                ui->levelSix->setStyleSheet("border-image: url(:/icons/six_2.svg);");
                break;
            case 7:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
                ui->levelSix->setStyleSheet("border-image: url(:/icons/six_2.svg);");
                ui->levelSeven->setStyleSheet("border-image: url(:/icons/seven_2.svg);");
                break;
            case 8:
                sessionLightChange();
                ui->levelOne->setStyleSheet("border-image: url(:/icons/one_2.svg);");
                ui->levelTwo->setStyleSheet("border-image: url(:/icons/two_2.svg);");
                ui->levelThree->setStyleSheet("border-image: url(:/icons/three_2.svg);");
                ui->levelFour->setStyleSheet("border-image: url(:/icons/four_2.svg);");
                ui->levelFive->setStyleSheet("border-image: url(:/icons/five_2.svg);");
                ui->levelSix->setStyleSheet("border-image: url(:/icons/six_2.svg);");
                ui->levelSeven->setStyleSheet("border-image: url(:/icons/seven_2.svg);");
                ui->levelEight->setStyleSheet("border-image: url(:/icons/eight_2.svg);");
            default:
                break;
            }
        }
    }else{
        qDebug() << "Please select a session before increasing intensity and make sure connection test is finished";
    }
}

void MainWindow::readRecordingHistory()
{
    QFile file(qApp->applicationDirPath() + "recording_history.txt");
    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Recording history loaded successfully";
        ui->recordingBrowser->setPlainText(file.readAll());
        file.close();
    }
    else{
        qDebug() << "Error loading previous recording history";
    }

}

void MainWindow::writeToRecordingHistory()
{
    QString newRecord = "Session Group: " + currentSession->getSessionGroup() + "\n"
            + "Session Type: " + currentSession->getSessionType() + "\n"
            + "Intensity Level: " + QString::number(currentSession->getSessionIntensity()) + "\n";

    QFile file(qApp->applicationDirPath() + "recording_history.txt");
    if (file.open(QFile::WriteOnly | QIODevice::Append | QFile::Text))
    {
        QTextStream stream(&file);
        stream << "-----Treatment Record------" << "\n" << newRecord << "\n";
//               << "Session Group: " << currentSession->getSessionGroup() << "\n"
//               << "Session Type: " << currentSession->getSessionType() << "\n"
//               << "Session Intensity: " << currentSession->getSessionIntensity() << "\n";
        qDebug() << "Recording saved successfully";
        file.close();
    }
    readRecordingHistory();
}

void MainWindow::saveIntensity()
{
    qDebug() << "Current intensity " << currentSession->getSessionIntensity() << " is saved as preference.";
}
