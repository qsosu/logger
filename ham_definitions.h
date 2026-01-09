#ifndef HAM_DEFINITIONS_H
#define HAM_DEFINITIONS_H

#include <QString>

struct bandData {
    int band_id;
    QString band_name;
    QString band_value;
    QString band_freq;
};

struct modeData {
    int mode_id;
    QString mode_name;
    QString mode_value;
    QString mode_report;
};


#endif // HAM_DEFINITIONS_H
