//
// Created by Patryk Mulica    on 30/04/24.
//

#ifndef DRONE8_SIMPLESTRATEGY_H
#define DRONE8_SIMPLESTRATEGY_H


#include "ScanningStrategy.h"

class SimpleStrategy: public ScanningStrategy {
public:
    void scanArea() override;

};



#endif //DRONE8_SIMPLESTRATEGY_H
