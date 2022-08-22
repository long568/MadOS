#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "CfgUser.h"

#include "MAX30105/MAX30105.h"
#include "MAX30105/heartRate.h"
#include "max.h"

static MAX30105 max;

MadBool max_init(void)
{
    if(!max.begin()) {
        return MFALSE;
    }
    return MTRUE;
}

// HeartRate
static const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
static byte rates[RATE_SIZE]; //Array of heart rates
static byte rateSpot = 0;
static MadTime_t lastBeat = 0; //Time at which the last beat occurred

static long beatsPerMinute;
static int  beatAvg;

static void hr_check(void)
{
    volatile MadTime_t now, delta;
    long irValue;

    irValue = max.getIR();

    if (checkForBeat(irValue) == true)
    {
        //We sensed a beat!
        now = madTimeNow();
        if(now > lastBeat) {
            delta = now - lastBeat;
        } else {
            delta = (~(MadTime_t)0) - lastBeat + now;
        }
        lastBeat = now;

        beatsPerMinute = 60 * 1000 / delta;

        if (beatsPerMinute < 255 && beatsPerMinute > 20)
        {
            rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
            rateSpot %= RATE_SIZE; //Wrap variable

            //Take average of readings
            beatAvg = 0;
            for (byte x = 0 ; x < RATE_SIZE ; x++)
                beatAvg += rates[x];
            beatAvg /= RATE_SIZE;
        }
    }
}

int max_hr(void)
{
    int i;
    max.setup(); //Configure sensor with default settings
    max.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
    max.setPulseAmplitudeGreen(0);  //Turn off Green LED
    for(i=0; i<568; i++){
        hr_check();
    }
    max.softReset();
    return beatAvg;
}

// SPO2
#if 0
#define SPO2_BUF_SIZE 25

static float spo2_calc(uint32_t *ir_input_data, uint32_t *red_input_data, uint16_t cache_nums)
{
    uint32_t ir_max  = *ir_input_data , ir_min  = *ir_input_data;
    uint32_t red_max = *red_input_data, red_min = *red_input_data;
    float R;
    uint16_t i;
    for(i=1;i<cache_nums;i++) {

    }
    for(i=1;i<cache_nums;i++)
    {
        if(ir_max<*(ir_input_data+i)) {
            ir_max=*(ir_input_data+i);
        }
        if(ir_min>*(ir_input_data+i)) {
            ir_min=*(ir_input_data+i);
        }
        if(red_max<*(red_input_data+i)) {
            red_max=*(red_input_data+i);
        }
        if(red_min>*(red_input_data+i)) {
            red_min=*(red_input_data+i);
        }
    }
    R=((ir_max+ir_min)*(red_max-red_min))/(float)((red_max+red_min)*(ir_max-ir_min));
    return ((-45.060)*R*R + 30.354*R + 94.845);
}

static int32_t spo2_check(void)
{
    uint32_t *_buf;
    uint32_t *ir_dat, *red_dat;
    int32_t rc;

    _buf = (uint32_t*)malloc(sizeof(uint32_t) * SPO2_BUF_SIZE * 2);
    if(!_buf) {
        return -1;
    }

    red_dat = _buf;
    ir_dat  = _buf + SPO2_BUF_SIZE;

    for (byte i = 0 ; i < SPO2_BUF_SIZE ; i++)
    {
        while (max.available() == false) //do we have new data?
            max.check(); //Check the sensor for new data
        red_dat[i] = max.getRed();
        ir_dat[i]  = max.getIR();
        max.nextSample(); //We're finished with this sample so move to next sample
    }

    rc = (int32_t)spo2_calc(ir_dat, red_dat, SPO2_BUF_SIZE);
    free(_buf);
    return rc;
}

int max_spo2(void)
{
    volatile int32_t rc;
    byte ledBrightness = 60; //Options: 0=Off to 255=50mA
    byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
    byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulseWidth = 411; //Options: 69, 118, 215, 411
    int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
    max.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
    rc = spo2_check();
    max.softReset();
    return rc;
}
#endif

#if 0
#define MAX_ADDR   0x57
#define MAX_READ   ((MAX_ADDR << 1) | 0x01)
#define MAX_WRITE  ((MAX_ADDR << 1) & 0xFE)

static void max_handler(MadVptr exData);

MadBool max_init(void)
{
    madThreadCreate(max_handler, 0, 256, THREAD_PRIO_HR);
    return MTRUE;
}

/*
 * buf[0] = ADDR7 + R/Wn
 * buf[1] = reg
 * buf[2] = w_dat / r_len
 */
static void max_handler(MadVptr exData)
{
    int cnt;
    int dev;
    char wbuf[4] = {0};
    char rbuf[4] = {0};

    char *addr = &wbuf[0];
    char *reg  = &wbuf[1];
    char *dat  = &wbuf[2];
    char *rlen = &wbuf[2];

    while(1) {
        dev = open("/dev/i2c", 0);
        if(dev < 0) {
            madTimeDly(3000);
            continue;
        }
        madTimeDly(100);

        // Tx Test
        *addr    = MAX_WRITE;
        *reg     = 0x15;
        *dat     = 0xAA;
        *(dat+1) = 0x55;
        write(dev, wbuf, 4);

        // Rx Test
        *addr = MAX_READ;
        *reg  = 0x15;
        *rlen = 2;
        while (1)
        {
            *rbuf     = 0;
            *(rbuf+1) = 0;
            write(dev, wbuf, 3);
            cnt = read(dev, rbuf, 2);
            if(cnt > 0) {
                __NOP();
                __NOP();
                __NOP();
            }

            madTimeDly(20);
        }

        close(dev);
        madTimeDly(1000);
    }
}
#endif
