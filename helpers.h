#ifndef HELPERS_H
#define HELPERS_H

#include <QObject>
#include <math.h>

class Helpers {

public:
  static QString GetBandByFreqHz(int Freq) {
    QString Band = "";
    int MHz = round(Freq / 1000000);

    if (MHz == 1) Band = "160M";
    if (MHz == 3) Band = "80M";
    if (MHz == 7) Band = "40M";
    if (MHz == 10) Band = "30M";
    if (MHz == 14) Band = "20M";
    if (MHz == 18) Band = "17M";
    if (MHz == 21) Band = "15M";
    if (MHz == 24) Band = "12M";
    if (MHz == 28 || MHz == 29) Band = "10M";
    if (MHz == 50) Band = "6M";
    if (MHz == 144 || MHz == 145) Band = "2M";
    if (MHz == 432 || MHz == 433 || MHz == 434 || MHz == 435 || MHz == 436 || MHz == 437 || MHz == 438) Band = "70CM";
    if (MHz == 1296) Band = "23CM";
    if (MHz == 2300) Band = "13CM";
    if (MHz == 5700) Band = "6CM";
    if (MHz == 10000) Band = "3CM";
    return Band;
  }

  static double BandToDefaultFreqMHz(QString band) {
    if (band == "160M") return 1.8;
    if (band == "80M") return 3.5;
    if (band == "40M") return 7.0;
    if (band == "30M") return 10.1;
    if (band == "20M") return 14.0;
    if (band == "17M") return 18.068;
    if (band == "15M") return 21.0;
    if (band == "12M") return 24.89;
    if (band == "10M") return 28.0;
    if (band == "6M") return 50.0;
    if (band == "2M") return 144.0;
    if (band == "70CM") return 432.0;
    if (band == "23CM") return 1296.0;
    if (band == "13CM") return 2300.0;
    if (band == "6CM") return 5700.0;
    if (band == "3CM") return 10000.0;
    return 0.0;
}

};

#endif // HELPERS_H
