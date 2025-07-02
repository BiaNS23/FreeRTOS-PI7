#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> // Mantenha para printf
#include "boards/pico_w.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/error.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
// #include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

// Incluir o seu módulo Modbus Slave
#include "modbus_slave.h"

// --- Suas definições de tarefas e semáforos existentes ---
const int task_delay = 200; // [jo:230915] inicialmente 500
const int led_delay = 1000; // [jo:230915] atraso para LED
const int task_size = 1024; // Aumentei o tamanho da stack para as tarefas, especialmente para ModbusSlave e ApplicationTask

#define LED_PIN 28

SemaphoreHandle_t mutex;
SemaphoreHandle_t binary;

void vGuardedPrint(char *out){
    xSemaphoreTake(mutex, portMAX_DELAY);
    printf("%s", out);
    xSemaphoreGive(mutex);
}

void vTaskSMP_print_core(void *pvParameters){
    char *task_name = pcTaskGetName(NULL);
    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    UBaseType_t mask = vTaskCoreAffinityGet(handle);
    char out[32];
    for (;;) {
        // Para evitar flood no serial, vamos comentar temporariamente ou usar com cautela
        // sprintf(out, "%s.", task_name);
        // vGuardedPrint(out);
        vTaskDelay(task_delay);
    }
}

void vTaskSMP_delay(void *pvParameters){
    for (;;) {
        xSemaphoreGive(binary);
        vTaskDelay(led_delay);
    }
}

void vTaskSMP_led(void *pvParameters){
    bool led_value = 0;
    for (;;) {
        if(xSemaphoreTake(binary, portMAX_DELAY)){
            if (led_value == 0) {
                gpio_put(LED_PIN, 1);
                led_value = 1;
            } else {
                gpio_put(LED_PIN, 0);
                led_value = 0;
            }
        }
    }
}

// --- Nova Tarefa de Aplicação para usar dados Modbus ---
// Esta tarefa seria a sua interface com a cinemática inversa, etc.
void application_task(void *pvParameters) {
    // Definir o fator de escalonamento que você usa no Python
    const float SCALING_FACTOR = 1000.0f; // Ex: se você envia 10.5 como 10500

    while (true) {
        // Exemplo de leitura segura de registradores Modbus
        // Verifica se a flag de controle foi definida pelo PC
        uint16_t control_flag = modbus_get_holding_register(MODBUS_REG_CONTROL_FLAG);

        if (control_flag == 1) {
            uint16_t x_scaled = modbus_get_holding_register(MODBUS_REG_X_COORD_SCALED);
            uint16_t y_scaled = modbus_get_holding_register(MODBUS_REG_Y_COORD_SCALED);

            float x_coord = (float)x_scaled / SCALING_FACTOR;
            float y_coord = (float)y_scaled / SCALING_FACTOR;

            printf("APLICACAO: Novo ponto recebido! X=%.3f, Y=%.3f\n", x_coord, y_coord);
            // *** AQUI VOCÊ CHAMARIA SUA FUNÇÃO DE CINEMÁTICA INVERSA ***
            // Ex: calculate_cable_lengths(x_coord, y_coord, 0.0f, &L1, &L2, &L3, &L4);
            // ... e então atualizaria os setpoints dos seus controladores PID

            // Resetar a flag após processar (para que não seja reprocessado continuamente)
            modbus_set_holding_register(MODBUS_REG_CONTROL_FLAG, 0);
        }

        // Exemplo de como acessar dados de arquivos (FC 0x15)
        // Você pode ter uma lógica mais complexa para processar as trajetórias ou configurações
        if (xSemaphoreTake(xModbusFileMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            // Exemplo de leitura do File 0, Record 0
            if (modbus_files[0].records[0].current_length > 0) {
                // Se a primeira palavra do registro for um "cabeçalho" para indicar dados novos
                // Ou se você tiver uma flag separada para "nova trajetória enviada"
                // printf("APLICACAO: Dados do File 0, Record 0: ");
                // for (int i = 0; i < modbus_files[0].records[0].current_length; i++) {
                //     printf("0x%04X ", modbus_files[0].records[0].data[i]);
                // }
                // printf("\n");
                // AQUI você processaria os dados da trajetória ou configurações salvas via FC 0x15
            }
            xSemaphoreGive(xModbusFileMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Checa a cada 50ms para ver se há novas requisições
    }
}


int main(){
    // stdio_init_all(); // Descomentar se você quiser o printf via USB (VSC ou terminal)
    stdio_usb_init(); // Inicializa stdio pelo USB. Essencial para ver os prints.
    printf("Serial init\n");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);

    // cyw43_arch_init();
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // LED aceso ao iniciar

    // Não inicialize UARTs aqui, modbus_slave_init() fará isso
    // uart_init(uart0, 115200); // [jo:230927]
    // gpio_set_function(0, GPIO_FUNC_UART);
    // gpio_set_function(1, GPIO_FUNC_UART);
    // uart_init(uart1, 115200); // [jo:230927]
    // gpio_set_function(4, GPIO_FUNC_UART);
    // gpio_set_function(5, GPIO_FUNC_UART);

    mutex = xSemaphoreCreateMutex(); // Create the mutex for guarded prints
    binary = xSemaphoreCreateBinary(); // For LED blinking

    // A fila xQueue não é mais necessária para a comunicação Modbus,
    // pois o Modbus Slave gerencia o buffer e as interrupções de UART diretamente.
    // Se você tiver outros usos para xQueue, mantenha, caso contrário, remova.
    // xQueue = xQueueCreate(20, sizeof(uint8_t)); // [jo:230927]

    // --- Inicializa o módulo Modbus Slave ---
    modbus_slave_init();

    // Define the task handles
    TaskHandle_t handleA;
    TaskHandle_t handleB;
    TaskHandle_t handleLed;
    TaskHandle_t handleModbusSlave; // Para a tarefa Modbus Slave
    TaskHandle_t handleApplication; // Para a tarefa de aplicação

    // Create tasks
    // As tarefas "A", "B", "C", "D" agora apenas atrasam, não imprimem para não poluir o console Modbus
    xTaskCreate(vTaskSMP_print_core, "A", task_size, NULL, 1, &handleA);
    xTaskCreate(vTaskSMP_print_core, "B", task_size, NULL, 1, &handleB);
    xTaskCreate(vTaskSMP_print_core, "C", task_size, NULL, 1, NULL);
    xTaskCreate(vTaskSMP_print_core, "D", task_size, NULL, 1, NULL);

    // Meus tasks de teste (delay e LED)
    xTaskCreate(vTaskSMP_delay, "E", task_size, NULL, 1, NULL);
    xTaskCreate(vTaskSMP_led, "F", task_size, NULL, 1, &handleLed);

    // --- Criar a tarefa Modbus Slave ---
    // Aumentei a prioridade para que ela responda rapidamente
    xTaskCreate(modbus_slave_task, "ModbusSlave", 2048, NULL, configMAX_PRIORITIES - 1, &handleModbusSlave); // Stack maior

    // --- Criar a tarefa de aplicação ---
    // Esta tarefa deve ter prioridade suficiente para reagir aos dados do Modbus,
    // mas talvez menor que a tarefa ModbusSlave se o processamento for longo.
    xTaskCreate(application_task, "ApplicationTask", 2048, NULL, 2, &handleApplication); // Stack maior

    // Pin Tasks
    vTaskCoreAffinitySet(handleA, (1 << 0)); // Core 0
    vTaskCoreAffinitySet(handleB, (1 << 1)); // Core 1
    vTaskCoreAffinitySet(handleLed, (1 << 0)); // LED só num core por causa do cyw43
    // O Modbus Slave pode rodar em qualquer core. Se quiser, force para um core específico:
    // vTaskCoreAffinitySet(handleModbusSlave, (1 << 0)); // Exemplo: rodar Modbus Slave no Core 0
    // A tarefa de aplicação pode rodar no core oposto ou no mesmo:
    // vTaskCoreAffinitySet(handleApplication, (1 << 1)); // Exemplo: rodar Aplicação no Core 1

    // Start the scheduler
    vTaskStartScheduler();

    // Se o escalonador retornar, houve um erro (memória, etc.)
    while (true) {
        // Loop infinito em caso de erro no FreeRTOS
    }

    return 0;
}

