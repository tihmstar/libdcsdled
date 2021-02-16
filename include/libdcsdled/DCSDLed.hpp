//
//  DCSDLed.hpp
//  libdcsdled
//
//  Created by tihmstar on 16.02.21.
//

#ifndef DCSDLed_hpp
#define DCSDLed_hpp

#include <libftdi1/ftdi.h>
#include <libgeneral/Manager.hpp>
#include <libgeneral/Event.hpp>
#include <mutex>

#define USEC_PER_SEC 1000000

class DCSDLed : protected tihmstar::Manager{
public:
    enum LedColor{
        LedColorRed    = 1,
        LedColorGreen  = 2,
        LedColorYellow = 8
    };
    enum BlinkSpeed{
        BlinkSpeedIdle = (uint32_t)(USEC_PER_SEC*0.5),
        BlinkSpeedInProgress = (uint32_t)(USEC_PER_SEC*0.1),
        BlinkSpeedWorking = (uint32_t)(USEC_PER_SEC*0.05)
    };
private:
    struct BlinkTimes{
        uint64_t onIters;
        uint64_t offIters;
    };
private:
    struct ftdi_context *_ftdi;
    bool _isConnected;
    BlinkTimes _blinkerIters[3];//rgy
    BlinkTimes _blinkerCurrent[3];//rgy
    uint32_t _blinkerSleepTime;
    
    std::mutex _ledlock;
    tihmstar::Event _blinkerUpdate;
    
    void connect(bool waitForDevice);
    void disconnect();

    virtual void stopAction() noexcept override;
    virtual void loopEvent() override;
public:
    DCSDLed(bool waitForDevice = false);
    ~DCSDLed();
    
    void enableLed(LedColor led, bool on);
    void setLed(LedColor led, bool on);
    
    void enableAllLed(bool on);
    void setAllLed(bool on);
    
    void blinkLed(LedColor led, uint64_t usec_on = BlinkSpeedIdle, uint64_t usec_off = 0);
};

#endif /* DCSDLed_hpp */
