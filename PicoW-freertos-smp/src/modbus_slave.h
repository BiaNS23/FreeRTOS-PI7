#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <stdint.h> // Para uint8_t, uint16_t
#include "FreeRTOS.h" // Para tipos do FreeRTOS
#include "task.h"     // Para TaskHandle_t
#include "semphr.h"   // Para Semaphores (Mutex para acesso aos registradores)

// --- Configurações Modbus ---
#define MODBUS_SLAVE_ID 1 // ID do seu dispositivo Modbus Slave (PC Master usa este ID)
#define MODBUS_BAUD_RATE 9600
#define MODBUS_UART_ID uart0 // Qual UART usar (uart0 ou uart1)
#define MODBUS_UART_TX_PIN 0 // Pino GPIO para UART TX
#define MODBUS_UART_RX_PIN 1 // Pino GPIO para UART RX

// --- Buffers e Tempos ---
#define MODBUS_RX_BUFFER_SIZE 256 // Tamanho do buffer de recepção Modbus
// Tempo de silêncio para fim de pacote RTU em ms (3.5 caracteres).
// Para 9600 bps (10 bits/char): 10/9600 s/char = 1.04 ms/char.
// 3.5 * 1.04 ms = ~3.64 ms. Usamos um valor seguro de 5ms.
#define MODBUS_RTU_TIMEOUT_MS 5

// --- Mapeamento de Registradores Modbus (Holding Registers) ---
// Estes são os registradores que o Master (PC) vai ler/escrever.
// Defina seus endereços e quantidade conforme a aplicação do robô.
#define MODBUS_REG_TEST_VALUE       0x0000 // Exemplo: Registrador de teste
#define MODBUS_REG_X_COORD_SCALED   0x0001 // Coordenada X escalada (ex: x * 1000)
#define MODBUS_REG_Y_COORD_SCALED   0x0002 // Coordenada Y escalada (ex: y * 1000)
// #define MODBUS_REG_Z_COORD_SCALED   0x0003 // Coordenada Z escalada (se 3D)
#define MODBUS_REG_CONTROL_FLAG     0x0004 // Um flag para o Master sinalizar "novo ponto" ou "processar"

// Adapte o total de registradores com base nos seus mapeamentos acima
#define MODBUS_NUM_HOLDING_REGISTERS 5 // Exemplo: 5 registradores de 0x0000 a 0x0004

// --- Mapeamento de Arquivos/Registros (para FC 0x15) ---
// Se o FC 0x15 for usado para trajetórias ou configurações maiores.
#define MODBUS_MAX_FILES 2             // Exemplo: 2 arquivos
#define MODBUS_MAX_RECORDS_PER_FILE 10 // Exemplo: 10 registros por arquivo
#define MODBUS_MAX_WORDS_PER_RECORD 60 // Exemplo: 60 palavras (120 bytes) por registro

// --- Variáveis Globais Acessíveis (para outras tarefas lerem/escreverem) ---
extern uint16_t holding_registers[MODBUS_NUM_HOLDING_REGISTERS];
extern SemaphoreHandle_t xModbusRegMutex; // Mutex para proteger o acesso aos registradores

// Estrutura para um registro de arquivo Modbus (FC 0x15)
typedef struct {
    uint16_t data[MODBUS_MAX_WORDS_PER_RECORD];
    uint16_t current_length; // Número de palavras válidas neste registro
} modbus_file_record_t;

// Estrutura para um arquivo Modbus (FC 0x15)
typedef struct {
    modbus_file_record_t records[MODBUS_MAX_RECORDS_PER_FILE];
} modbus_file_t;

extern modbus_file_t modbus_files[MODBUS_MAX_FILES];
extern SemaphoreHandle_t xModbusFileMutex; // Mutex para proteger o acesso aos arquivos

// --- Protótipos de Funções ---
void modbus_slave_init();
void modbus_slave_task(void *pvParameters);
void uart_modbus_rx_isr();

// Funções para acessar os registradores de forma segura (usando mutex)
uint16_t modbus_get_holding_register(uint16_t address);
void modbus_set_holding_register(uint16_t address, uint16_t value);

#endif // MODBUS_SLAVE_H