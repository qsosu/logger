#ifndef HELPERS_H
#define HELPERS_H

#include <QObject>
#include <QDebug>
#include <math.h>
#include <QSysInfo>
#include <QStandardPaths>


class Helpers {

public:
  static QString GetBandByFreqHz(unsigned long long Freq) {
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
    if (MHz >= 432 && MHz <= 438) Band = "70CM";
    if (MHz >= 1260 && MHz <= 1300) Band = "23CM";
    if (MHz >= 2400 && MHz <= 2450) Band = "13CM";
    if (MHz >= 5650 && MHz <= 5850) Band = "6CM";
    if (MHz >= 10000 && MHz <= 10500) Band = "3CM";
    return Band;
  }
};

#endif // HELPERS_H
