#include <stdbool.h>
#include <stdint.h>
//#include <stdio.h>
#include "boards/pico_w.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/error.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

const int task_delay = 200; // [jo:230915] inicialmente 500
const int led_delay = 1000; // [jo:230915] atraso para LED
const int task_size = 128;

SemaphoreHandle_t mutex;
SemaphoreHandle_t binary; // [jo:230915]
QueueHandle_t xQueue;     // [jo:230927] Queue para serial

//uint8_t rx_buffer[20];   // [jo:230927] buffer de recepção do UART0 e UART1

void vGuardedPrint(char *out){
  xSemaphoreTake(mutex, portMAX_DELAY);
  //puts(out);
  printf("%s", out);
  xSemaphoreGive(mutex);
}

void vTaskSMP_print_core(void *pvParameters){
  char *task_name = pcTaskGetName(NULL);
  TaskHandle_t handle = xTaskGetCurrentTaskHandle();
  UBaseType_t mask = vTaskCoreAffinityGet(handle);
  char out[32];
  for (;;) {
    //sprintf(out, "%s %d %d %d", task_name, get_core_num(), xTaskGetTickCount(), mask);
    //sprintf(out, "%s ", task_name);
    sprintf(out, ".");
    vGuardedPrint(out);
    vTaskDelay(task_delay);
  }
}

void vTaskSMP_delay(void *pvParameters){
  for (;;) {
    xSemaphoreGive(binary);
    //vGuardedPrint("delay");
    vTaskDelay(led_delay); //task_delay);
  }
}

void vTaskSMP_led(void *pvParameters){
  bool led_value = 0;
  for (;;) {
    if(xSemaphoreTake(binary, portMAX_DELAY)){
      if (led_value == 0) { // se for no core 0
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        led_value = 1;
      } else {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        led_value = 0;
      }
    //vGuardedPrint("led");
    }
  }
}

void vTaskSMP_getchar(void *pvParameters) { // [jo:230916]
  int value;
  char out[10];
  while (1) {
    value = getchar_timeout_us(0);
    if (value != PICO_ERROR_TIMEOUT) {
      sprintf(out, "%c\n", value);
      vGuardedPrint(out);
    }
    if (xQueueReceive(xQueue, &value, 0)) { // [jo:230927]
      sprintf(out, "%c", value);
      vGuardedPrint(out);
    }
    vTaskDelay(50); //task_delay);
  }
}

void vTaskSMP_uart0(void *pvParameters) { // [jo:230927] envia valor de contador para UART0 e recebe caracter pela UART0 e coloca numa fila
  int counter = 0;
  char out[20];
  uint8_t inFromSerial0;

  while (1) {

    if (uart_is_writable(uart0)) {
      sprintf(out, "%d\n", counter++);
      uart_puts(uart0, out);
    }

    if (uart_is_readable(uart0)) {
      inFromSerial0 = uart_getc(uart0);
      xQueueSendToBack(xQueue, &inFromSerial0, 0);
    }

    vTaskDelay(500);
  }
}

void vTaskSMP_uart1(void *pvParameters) { // [jo:230927] envia valor de contador para UART1
  int counter = 10000; // inicia em 10000 para diferenciar da UART0
  char out[20];
  while (1) {
    sprintf(out, "%d\n", counter++);
    uart_puts(uart1, out);
    vTaskDelay(500); 
  }
}

int main(){
  //stdio_init_all();
  stdio_usb_init(); // [jo:230927] só inicializa stdio pelo USB, não tenho certeza, mas não quero conflito com serial por hardware, por isso não quero stdio pela USART 

  cyw43_arch_init();
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // [jo:230915] precisei colocar isso senão não começa a piscar

  uart_init(uart0, 115200); // [jo:230927] Initialise UART 0
  // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);

  uart_init(uart1, 115200); // [jo:230927] Initialise UART 1
  // Set the GPIO pin mux to the UART - 4 is TX, 5 is RX
  gpio_set_function(4, GPIO_FUNC_UART);
  gpio_set_function(5, GPIO_FUNC_UART);

  mutex = xSemaphoreCreateMutex(); // Create the mutex
  binary = xSemaphoreCreateBinary(); // [jo:230915] para o LED
  xQueue = xQueueCreate(20, sizeof(uint8_t)); // [jo:230927] fila para recepção pela serial

  // Define the task handles
  TaskHandle_t handleA;
  TaskHandle_t handleB;
  TaskHandle_t handleG; // [jo:230916]
  TaskHandle_t handleLed; // [jo:230929] cyw43 precisa rodar num único core

  // Create 4x tasks with different names & 2 with handles
  xTaskCreate(vTaskSMP_print_core, "A", task_size, NULL, 1, &handleA);
  xTaskCreate(vTaskSMP_print_core, "B", task_size, NULL, 1, &handleB);
  xTaskCreate(vTaskSMP_print_core, "C", task_size, NULL, 1, NULL);
  xTaskCreate(vTaskSMP_print_core, "D", task_size, NULL, 1, NULL);

  // Meus tasks de teste
  xTaskCreate(vTaskSMP_delay, "E", task_size, NULL, 1, NULL);
  xTaskCreate(vTaskSMP_led, "F", task_size, NULL, 1, &handleLed);
  xTaskCreate(vTaskSMP_getchar, "G", task_size, NULL, 1, &handleG); // [jo:230916]
  xTaskCreate(vTaskSMP_uart0, "H", task_size, NULL, 1, NULL); // [jo:230927]
  xTaskCreate(vTaskSMP_uart1, "I", task_size, NULL, 1, NULL); // [jo:230927]

  // Pin Tasks
  vTaskCoreAffinitySet(handleA, (1 << 0)); // Core 0
  vTaskCoreAffinitySet(handleB, (1 << 1)); // Core 1
  vTaskCoreAffinitySet(handleG, (1 << 1)); // [jo:230916] Core 1
  vTaskCoreAffinitySet(handleLed, (1 << 0)); // [jo:230929] LED só num core por causa do cyw43

  // Start the scheduler
  vTaskStartScheduler();

  return 0;
}