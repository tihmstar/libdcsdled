//
//  DCSDLed.cpp
//  libdcsdled
//
//  Created by tihmstar on 16.02.21.
//

#include "DCSDLed.hpp"
#include <libgeneral/macros.h>
#include <unistd.h>

static uint64_t GCD(uint64_t a, uint64_t b){
    if (a == 0) return b;
    if (b == 0) return a;

    do {
        uint64_t h = a % b;
        a = b;
        b = h;
    } while (b != 0);
    return a;
}

DCSDLed::DCSDLed(bool waitForDevice)
: _ftdi(NULL), _isConnected(false), _blinkerIters{}, _blinkerCurrent{}, _blinkerSleepTime(0)
{
    assure(_ftdi = ftdi_new());
    connect(waitForDevice);
    startLoop();
}

DCSDLed::~DCSDLed(){
    stopLoop();
    disconnect();
    safeFreeCustom(_ftdi, ftdi_free);
}

#pragma mark private
void DCSDLed::connect(bool waitForDevice){
    int err = 0;
    disconnect();
    while (true) {
        if ((_isConnected = ((err = ftdi_usb_open(_ftdi, 0x0403, 0x8a88)) >=0))) break;
        retassure(waitForDevice, "unable to open ftdi device: %d (%s)\n", err, ftdi_get_error_string(_ftdi));
        debug("unable to open ftdi device: %d (%s)\n", err, ftdi_get_error_string(_ftdi));
        sleep(1);
    }
    debug("ftdi open succeeded: %d\n",err);
}

void DCSDLed::disconnect(){
    try {
        enableAllLed(1);
        setAllLed(1);
    } catch (...) {
        //
    }
    if (_isConnected) {
        ftdi_usb_close(_ftdi); _isConnected = false;
    }
}

void DCSDLed::stopAction() noexcept{
    _blinkerSleepTime = 1;
    _blinkerUpdate.notifyAll();
}

void DCSDLed::loopEvent(){
    int err = 0;
    uint8_t v = 0;
    while (_blinkerSleepTime == 0) {
        //no blinker active
        _blinkerUpdate.wait();
    }
    std::unique_lock<std::mutex> ul(_ledlock);
    retassure(!(err = ftdi_read_pins(_ftdi,&v)), "Failed to read pins with error=%d (%s)\n", err, ftdi_get_error_string(_ftdi));
    //rgy

    //update iters
    if (_blinkerIters[0].onIters || _blinkerIters[0].offIters) {
        if (v & LedColorRed) {
            if (++_blinkerCurrent[0].onIters >= _blinkerIters[0].onIters) {
                _blinkerCurrent[0].onIters = 0;
                v &= ~LedColorRed;
            }
        }else{
            if (++_blinkerCurrent[0].offIters >= _blinkerIters[0].offIters) {
                _blinkerCurrent[0].offIters = 0;
                v |= LedColorRed;
            }
        }
    }

    if (_blinkerIters[1].onIters || _blinkerIters[1].offIters) {
        if (v & LedColorGreen) {
            if (++_blinkerCurrent[1].onIters >= _blinkerIters[1].onIters) {
                _blinkerCurrent[1].onIters = 0;
                v &= ~LedColorGreen;
            }
        }else{
            if (++_blinkerCurrent[1].offIters >= _blinkerIters[1].offIters) {
                _blinkerCurrent[1].offIters = 0;
                v |= LedColorGreen;
            }
        }
    }
    
    if (_blinkerIters[2].onIters || _blinkerIters[2].offIters) {
        if (v & LedColorYellow) {
            if (++_blinkerCurrent[2].onIters >= _blinkerIters[2].onIters) {
                _blinkerCurrent[2].onIters = 0;
                v &= ~LedColorYellow;
            }
        }else{
            if (++_blinkerCurrent[2].offIters >= _blinkerIters[2].offIters) {
                _blinkerCurrent[2].offIters = 0;
                v |= LedColorYellow;
            }
        }
    }
    retassure(!(err = ftdi_set_bitmode(_ftdi, v, BITMODE_CBUS)), "Failed to set led with error=%d (%s)\n", err, ftdi_get_error_string(_ftdi));
    ul.unlock();
    usleep(_blinkerSleepTime);
    ul.lock();
}

#pragma mark public
void DCSDLed::enableLed(LedColor led, bool on){
    std::unique_lock<std::mutex> ul(_ledlock);
    int err = 0;
    uint8_t v = 0;
    retassure(!(err = ftdi_read_pins(_ftdi,&v)), "Failed to read pins with error=%d (%s)\n", err, ftdi_get_error_string(_ftdi));
    if (on) {
        v |= led << 4;
    }else{
        v &= ~(led << 4);
    }
    retassure(!(err = ftdi_set_bitmode(_ftdi, v, BITMODE_CBUS)), "Failed to enable led with error=%d (%s)\n", err, ftdi_get_error_string(_ftdi));
}

void DCSDLed::setLed(LedColor led, bool on){
    std::unique_lock<std::mutex> ul(_ledlock);
    int err = 0;
    uint8_t v = 0;
    retassure(!(err = ftdi_read_pins(_ftdi,&v)), "Failed to read pins with error=%d (%s)\n", err, ftdi_get_error_string(_ftdi));
    if (on) {
        v |= led;
    }else{
        v &= ~led;
    }
    retassure(!(err = ftdi_set_bitmode(_ftdi, v, BITMODE_CBUS)), "Failed to set led with error=%d (%s)\n", err, ftdi_get_error_string(_ftdi));
    
    //disable blinking for leds that we will update
    if (led & LedColorRed) {
        _blinkerIters[0] = {};
    }
    if (led & LedColorGreen) {
        _blinkerIters[1] = {};
    }
    if (led & LedColorYellow) {
        _blinkerIters[2] = {};
    }
}

void DCSDLed::enableAllLed(bool on){
    enableLed((DCSDLed::LedColor)(DCSDLed::LedColorRed | DCSDLed::LedColorGreen | DCSDLed::LedColorYellow), on);
}

void DCSDLed::setAllLed(bool on){
    setLed((DCSDLed::LedColor)(DCSDLed::LedColorRed | DCSDLed::LedColorGreen | DCSDLed::LedColorYellow), on);
}

void DCSDLed::blinkLed(LedColor led, uint64_t usec_on, uint64_t usec_off){
    //rgy
    if (!usec_off) {
        usec_off = usec_on;
    }
    
    //disable blinking for leds that we will update
    if (led & LedColorRed) {
        _blinkerIters[0] = {};
    }
    if (led & LedColorGreen) {
        _blinkerIters[1] = {};
    }
    if (led & LedColorYellow) {
        _blinkerIters[2] = {};
    }
    
    //maximize current sleep
    uint32_t gcd = 0;
    for (int i=0; i<2; i++) {
        uint64_t cgcd = GCD(_blinkerIters[i].onIters, _blinkerIters[i].offIters);
        gcd = (uint32_t)GCD(gcd,cgcd);
    }
    if (gcd > 1) {
        for (int i = 0; i<3; i++) {
            _blinkerIters[i].onIters *= gcd;
            _blinkerIters[i].offIters *= gcd;
        }
        _blinkerSleepTime /= gcd;
    }
    
    //adjust new sleep time
    uint32_t newMaximalSleep = (uint32_t)GCD(_blinkerSleepTime, GCD(usec_on, usec_off));
    if (newMaximalSleep != _blinkerSleepTime) {
        uint64_t factor = _blinkerSleepTime / newMaximalSleep;
        
        _blinkerSleepTime = newMaximalSleep;
        for (int i = 0; i<3; i++) {
            _blinkerIters[i].onIters *= factor;
            _blinkerIters[i].offIters *= factor;
        }
    }
    
    //finally write new iters
    if (led & LedColorRed) {
        _blinkerIters[0] = {
            .onIters = usec_on / newMaximalSleep,
            .offIters = usec_off / newMaximalSleep
        };
    }
    if (led & LedColorGreen) {
        _blinkerIters[1] = {
            .onIters = usec_on / newMaximalSleep,
            .offIters = usec_off / newMaximalSleep
        };
    }
    if (led & LedColorYellow) {
        _blinkerIters[2] = {
            .onIters = usec_on / newMaximalSleep,
            .offIters = usec_off / newMaximalSleep
        };
    }
    _blinkerUpdate.notifyAll();
}
