#ifndef __SRV_MQTT_TOPIC__H__
#define __SRV_MQTT_TOPIC__H__

// 47.92.39.234:1883

#define TOPIC_STATION_UP     "YCSD/Bosch/Station/Up"
#define TOPIC_STATION_DOWN   "YCSD/Bosch/Station/Down"
#define TOPIC_AUTOSTORE_UP   "YCSD/Bosch/AutoStore/Up"
#define TOPIC_AUTOSTORE_DOWN "YCSD/Bosch/AutoStore/Down"
#define TOPIC_AGV_UP         "YCSD/Bosch/AGV/Up"
#define TOPIC_AGV_DOWN       "YCSD/Bosch/AGV/Down"

#define TOPIC_X_DOWN         "YCSD/Bosch/+/Down"

enum {
    TOPIC_ID_STATION = 0,
    TOPIC_ID_AUTOSTORE,
    TOPIC_ID_AGV,
    TOPIC_ID_UNKNOWN = 0xFF
};

#endif
