#include <stdio.h>
#include <string.h>

#include "../core/core.h"
#include "esp_common.h"
#include "delay.h"

void os_delay_ms(unsigned int value)
{
    vTaskDelay(1000*portTICK_RATE_MS);
}
