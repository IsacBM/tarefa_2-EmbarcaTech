#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define BUTTON_PIN 5   // Pino do Botão
#define LED_PIN_AZ 12  // Pino do LED Azul
#define LED_PIN_VD 11  // Pino do LED Vermelho

QueueHandle_t botaoQueue; // Fila que vai guardar o estado do botão

void init_setup() {     
    stdio_init_all();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_init(LED_PIN_AZ);
    gpio_set_dir(LED_PIN_AZ, GPIO_OUT);
    gpio_init(LED_PIN_VD);
    gpio_set_dir(LED_PIN_VD, GPIO_OUT);
}

void verificaBotao(void *pvParameters) { 
    bool botaoEstado;  
    for (;;) {
        botaoEstado = gpio_get(BUTTON_PIN);
        xQueueSend(botaoQueue, &botaoEstado, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100)); // Tempo
    }
}

void processaBotao(void *pvParameters) {
    bool botaoEstado;
    for (;;) {
        if (xQueueReceive(botaoQueue, &botaoEstado, portMAX_DELAY)) {
            xQueueSend(botaoQueue, &botaoEstado, portMAX_DELAY);
        }
    }
}

void controlarLED(void *pvParameters) {
    bool botaoEstado;
    for (;;) {
        if (xQueueReceive(botaoQueue, &botaoEstado, portMAX_DELAY)) { // Verifica se há algo na fila e add no estado do botão
            if (botaoEstado == false) { 
                gpio_put(LED_PIN_AZ, 1);  
                gpio_put(LED_PIN_VD, 1); 
            } else {
                gpio_put(LED_PIN_AZ, 0); 
                gpio_put(LED_PIN_VD, 0);  
            } 
        }
    }
}

int main() {
    init_setup();

    botaoQueue = xQueueCreate(5, sizeof(bool));

    xTaskCreate(verificaBotao, "verificaBotao", 256, NULL, 1, NULL);
    xTaskCreate(processaBotao, "processaBotao", 256, NULL, 2, NULL);
    xTaskCreate(controlarLED, "controlarLED", 256, NULL, 3, NULL);
    
    vTaskStartScheduler();
    for(;;);
}
