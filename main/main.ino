#include <arduino-timer.h>

#include "src/wifi_driver.h"
#include "src/firebase.h"
#include "src/mpu6050.h"
#include "src/esp_functions.h"

//----------------------------------------------------------------

auto timer = timer_create_default();

TaskHandle_t collect_data_thread;
TaskHandle_t send_data_thread;
QueueHandle_t queue_acc;
QueueHandle_t queue_gyr;

//----------------------------------------------------------------

bool check_timer_callback(void *)
{
    mpu_collect_data(queue_acc, queue_gyr);
    return true;
}

void collect_data_code(void *pvParameters)
{

    while (1)
    {
        timer.tick();
    }
}

void send_data_code(void *pvParameters)
{

    while (1)
    {
        if (wifi_check_status() != WL_CONNECTED)
        {
            Serial.println("[Communication] Connection with WiFi network lost.");
            wifi_connect();
            firebase_connect();
        }
        sensors_event_t acc = esp_get_queue_element(queue_acc);
        sensors_event_t gyr = esp_get_queue_element(queue_gyr);
        firebase_send_data(acc, gyr);
    }
}

void setup()
{
    Serial.begin(115200);

    queue_acc = NULL;
    esp_create_data_queue(&queue_acc, &queue_gyr);
    timer.every(1, check_timer_callback);

    if (!wifi_connect() || !firebase_connect() || !mpu_initialize())
    {
        esp_reset();
    }

    Serial.println("[ESP] Creating thread Collect data.");

    xTaskCreatePinnedToCore(
        collect_data_code,    /* Task function. */
        "Collect data",       /* name of task. */
        10000,                /* Stack size of task */
        NULL,                 /* parameter of the task */
        2,                    /* priority of the task */
        &collect_data_thread, /* Task handle to keep track of created task */
        0);                   /* pin task to core 0 */

    Serial.println("[ESP] Creating thread Send data.");

    xTaskCreatePinnedToCore(
        send_data_code,    /* Task function. */
        "Send data",       /* name of task. */
        15000,             /* Stack size of task */
        NULL,              /* parameter of the task */
        1,                 /* priority of the task */
        &send_data_thread, /* Task handle to keep track of created task */
        1);                /* pin task to core 1 */

    Serial.println("[ESP] All threads created.");
}

void loop()
{
}
