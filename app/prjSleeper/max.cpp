#include <fcntl.h>
#include <unistd.h>
#include "CfgUser.h"

#include "MAX30105/MAX30105.h"
#include "MAX30105/heartRate.h"
#include "MAX30105/spo2_algorithm.h"
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

int max_hr_get(void)
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
#define MAX_BRIGHTNESS 255

static uint32_t irBuffer[100]; //infrared LED sensor data
static uint32_t redBuffer[100];  //red LED sensor data

static int32_t bufferLength; //data length
static int32_t spo2; //SPO2 value
static int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
static int32_t heartRate; //heart rate value
static int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

static void spo2_check(void)
{
    int cnt;
    bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps

    //read the first 100 samples, and determine the signal range
    for (byte i = 0 ; i < bufferLength ; i++)
    {
        while (max.available() == false) //do we have new data?
            max.check(); //Check the sensor for new data
        redBuffer[i] = max.getRed();
        irBuffer[i] = max.getIR();
        max.nextSample(); //We're finished with this sample so move to next sample
    }

    //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
    cnt = 4;
    while (cnt--)
    {
        //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
        for (byte i = 25; i < 100; i++)
        {
            redBuffer[i - 25] = redBuffer[i];
            irBuffer[i - 25] = irBuffer[i];
        }

        //take 25 sets of samples before calculating the heart rate.
        for (byte i = 75; i < 100; i++)
        {
            while (max.available() == false) //do we have new data?
                max.check(); //Check the sensor for new data
            redBuffer[i] = max.getRed();
            irBuffer[i] = max.getIR();
            max.nextSample(); //We're finished with this sample so move to next sample
        }

        //After gathering 25 new samples recalculate HR and SP02
        maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    }
}

int max_spo2_get(void)
{
    byte ledBrightness = 60; //Options: 0=Off to 255=50mA
    byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
    byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulseWidth = 411; //Options: 69, 118, 215, 411
    int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
    max.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
    spo2_check();
    max.softReset();
    if(!validSPO2) {
        spo2 = 0;
    }
    return spo2;
}

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
