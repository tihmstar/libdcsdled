//
//  main.cpp
//  libdcsdled
//
//  Created by tihmstar on 16.02.21.
//

#include <stdio.h>
#include "DCSDLed.hpp"
#include <unistd.h>
#include <mutex>

int main(int argc, const char * argv[]) {
    printf("start\n");

    DCSDLed led;

    led.enableAllLed(true);

    led.setAllLed(false);
    
    
    led.blinkLed(DCSDLed::LedColorGreen, DCSDLed::BlinkSpeedIdle);
    led.blinkLed(DCSDLed::LedColorYellow, DCSDLed::BlinkSpeedWorking);
    led.blinkLed(DCSDLed::LedColorRed, DCSDLed::BlinkSpeedInProgress);

    sleep(5);
    
    led.setLed(DCSDLed::LedColorRed, 1);
    led.setLed(DCSDLed::LedColorGreen, 0);

    std::mutex lck;
    
    lck.lock();
    lck.lock();

    printf("done\n");
    return 0;
}
