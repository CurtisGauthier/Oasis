#ifndef PROFILE_H
#define PROFILE_H


/* Class Purpose: Saves battery level and power level inbetween uses of the app
 *
 * Data Members:
 * - double batteryLevel: a double representation of the battery level
 * - int powerStatus: a integer representation of the power level. Either (1) ON or (0) OFF
 * - int id: a unique identifier
 *
 * Class Functions:
 * - Getters and Setters
 */




class Profile
{
public:
    Profile(int id, double level , int status);
    int getId();
    double getBLvl();
    int getPowerStatus();
    void setBLvl(double);
    void setPowerStatus(int);

private:
  int id;
  double batteryLvl;
  int powerStatus;
};

#endif // PROFILE_H
