#include "firebase.h"
#include "print_string.h"
#include "tags.h"
#include "data.h"

//----------------------------------------------------------------

FirebaseData firebase_data;
FirebaseAuth firebase_auth;
FirebaseConfig firebase_config;
FirebaseJson acc_X;
FirebaseJson acc_Y;
FirebaseJson acc_Z;

//----------------------------------------------------------------

bool firebase_connect(void)
{
    String uid;
    String path;
    firebase_config.host = FIREBASE_HOST;
    firebase_config.api_key = API_KEY;

    firebase_auth.user.email = USER_EMAIL;
    firebase_auth.user.password = USER_PASSWORD;

    Serial << TAG_FIREBASE << "Connecting to database...\n";
    int retry_count = FIREBASE_MAXIMUM_RETRY;
    while (retry_count--)
    {
        Firebase.begin(&firebase_config, &firebase_auth);
        Firebase.reconnectWiFi(true);

        Serial << TAG_FIREBASE << "Getting User UID\n";
        while (firebase_auth.token.uid == "")
        {
            Serial.print('.');
            delay(1000);
        }

        uid = firebase_auth.token.uid.c_str();
        Serial << TAG_FIREBASE << "User UID: " << uid << "\n";
        path = "esp/" + uid;

        if (!Firebase.RTDB.beginStream(&firebase_data, path.c_str()))
            Serial << TAG_FIREBASE << "Error during stream connecting: " << firebase_data.errorReason().c_str() << ".\n";
        else
        {
            Serial << TAG_FIREBASE << "Successful connecting to stream.\n";
            return true;
        }
    }
    return false;
}

void firebase_send_data(sensors_event_t acc, sensors_event_t gyr)
{
    Serial << TAG_FIREBASE << "Sending data to firebase.\n";
    if (!Firebase.RTDB.pushFloat(&firebase_data, "/acc/ox", acc.acceleration.x) ||
        !Firebase.RTDB.pushFloat(&firebase_data, "/acc/oy", acc.acceleration.y) ||
        !Firebase.RTDB.pushFloat(&firebase_data, "/acc/oz", acc.acceleration.z))
        Serial << TAG_FIREBASE << "Sample dropped.\n";
    if (!Firebase.RTDB.pushFloat(&firebase_data, "/gyr/ox", gyr.gyro.x) ||
        !Firebase.RTDB.pushFloat(&firebase_data, "/gyr/oy", gyr.gyro.y) ||
        !Firebase.RTDB.pushFloat(&firebase_data, "/gyr/oz", gyr.gyro.z))
        Serial << TAG_FIREBASE << "Sample dropped.\n";
}