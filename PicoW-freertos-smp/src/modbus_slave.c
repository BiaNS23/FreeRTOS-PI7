#include "modbus_slave.h"
#include "modbus_crc.h" // Inclua o CRC
#include <stdio.h>    // Para printf
#include <string.h>   // Para memcpy

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/time.h" // Para get_absolute_time, to_ms_since_boot

// --- Variáveis Internas do Módulo Modbus Slave ---
static uint8_t rx_buffer[MODBUS_RX_BUFFER_SIZE];
static volatile uint16_t rx_buffer_idx = 0;
static volatile uint32_t last_byte_time_ms = 0; // Timestamp do último byte recebido

// Variáveis Globais (declaradas em .h, definidas aqui)
uint16_t holding_registers[MODBUS_NUM_HOLDING_REGISTERS];
SemaphoreHandle_t xModbusRegMutex; // Mutex para proteger o acesso aos registradores

modbus_file_t modbus_files[MODBUS_MAX_FILES];
SemaphoreHandle_t xModbusFileMutex; // Mutex para proteger o acesso aos arquivos

// --- Protótipos de Funções Estáticas (internas ao módulo) ---
static void send_modbus_response(uint8_t *response, int len);
static void send_modbus_exception_response(uint8_t slave_id, uint8_t function_code, uint8_t exception_code);
static void handle_modbus_request(uint8_t *request, int len);

// --- Implementação das Funções Exportadas ---

void modbus_slave_init() {
    // Inicializar UART
    uart_init(MODBUS_UART_ID, MODBUS_BAUD_RATE);
    gpio_set_function(MODBUS_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(MODBUS_UART_RX_PIN, GPIO_FUNC_UART);

    // Desabilitar FIFO para facilitar o controle de timeout de byte a byte
    uart_set_fifo_enabled(MODBUS_UART_ID, false);

    // Habilitar interrupção de RX
    irq_set_exclusive_handler(MODBUS_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ, uart_modbus_rx_isr);
    irq_set_enabled(MODBUS_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ, true);
    uart_set_irq_enables(MODBUS_UART_ID, true, false); // Habilita RX, desabilita TX IRQ

    // Inicializar registradores com zero
    for (int i = 0; i < MODBUS_NUM_HOLDING_REGISTERS; i++) {
        holding_registers[i] = 0;
    }

    // Inicializar Mutex para os registradores
    xModbusRegMutex = xSemaphoreCreateMutex();
    if (xModbusRegMutex == NULL) {
        printf("Erro: Não foi possível criar o Mutex para registradores Modbus!\n");
        // Lidar com erro (reiniciar, logar, etc.)
    }

    // Inicializar Mutex para os arquivos
    xModbusFileMutex = xSemaphoreCreateMutex();
    if (xModbusFileMutex == NULL) {
        printf("Erro: Não foi possível criar o Mutex para arquivos Modbus!\n");
        // Lidar com erro
    }

    // Inicializar a estrutura de arquivos (para FC 0x15)
    for (int f = 0; f < MODBUS_MAX_FILES; f++) {
        for (int r = 0; r < MODBUS_MAX_RECORDS_PER_FILE; r++) {
            modbus_files[f].records[r].current_length = 0;
            // Opcional: inicializar dados com zero
            for (int w = 0; w < MODBUS_MAX_WORDS_PER_RECORD; w++) {
                modbus_files[f].records[r].data[w] = 0;
            }
        }
    }

    printf("Modbus Slave inicializado na UART%d @ %d bps.\n", MODBUS_UART_ID, MODBUS_BAUD_RATE);
}

// Tarefa FreeRTOS para o Modbus Slave
void modbus_slave_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1); // Checar a cada 1ms

    while (true) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Lógica de timeout para detecção de fim de pacote Modbus RTU
        if (rx_buffer_idx > 0 && (to_ms_since_boot(get_absolute_time()) - last_byte_time_ms) > MODBUS_RTU_TIMEOUT_MS) {
            // Mensagem completa recebida
            printf("Pacote Modbus recebido (len=%d): ", rx_buffer_idx);
            for(int i=0; i<rx_buffer_idx; i++) {
                printf("%02X ", rx_buffer[i]);
            }
            printf("\n");

            handle_modbus_request(rx_buffer, rx_buffer_idx);
            rx_buffer_idx = 0; // Resetar buffer para a próxima mensagem
        }
        // Se o buffer estourar sem timeout, também resetar para evitar travamentos
        if (rx_buffer_idx >= MODBUS_RX_BUFFER_SIZE) {
            printf("Erro: Buffer de RX Modbus estourou. Resetando.\n");
            rx_buffer_idx = 0;
        }
    }
}

// ISR de Recepção UART (chamada pela interrupção de hardware)
void uart_modbus_rx_isr() {
    while (uart_is_readable(MODBUS_UART_ID)) {
        uint8_t ch = uart_getc(MODBUS_UART_ID);
        if (rx_buffer_idx < MODBUS_RX_BUFFER_SIZE) {
            rx_buffer[rx_buffer_idx++] = ch;
        }
        last_byte_time_ms = to_ms_since_boot(get_absolute_time()); // Atualiza o timestamp do último byte
    }
}

// Funções para acesso seguro aos registradores (via mutex)
uint16_t modbus_get_holding_register(uint16_t address) {
    uint16_t value = 0;
    if (xSemaphoreTake(xModbusRegMutex, portMAX_DELAY) == pdTRUE) {
        if (address < MODBUS_NUM_HOLDING_REGISTERS) {
            value = holding_registers[address];
        }
        xSemaphoreGive(xModbusRegMutex);
    }
    return value;
}

void modbus_set_holding_register(uint16_t address, uint16_t value) {
    if (xSemaphoreTake(xModbusRegMutex, portMAX_DELAY) == pdTRUE) {
        if (address < MODBUS_NUM_HOLDING_REGISTERS) {
            holding_registers[address] = value;
        }
        xSemaphoreGive(xModbusRegMutex);
    }
}


// --- Implementação das Funções Estáticas (Internas) ---

static void send_modbus_response(uint8_t *response, int len) {
    uart_write_blocking(MODBUS_UART_ID, response, len);
    printf("Resposta Modbus enviada (len=%d): ", len);
    for(int i=0; i<len; i++) {
        printf("%02X ", response[i]);
    }
    printf("\n");
}

static void send_modbus_exception_response(uint8_t slave_id, uint8_t function_code, uint8_t exception_code) {
    // [SlaveID] [FC + 0x80] [Exception Code] [CRC L] [CRC H]
    uint8_t pdu[3] = {
        slave_id,
        function_code | 0x80, // Set high bit for error
        exception_code
    };
    uint16_t crc_val = calculate_crc16(pdu, 3);

    uint8_t response[5];
    memcpy(response, pdu, 3);
    response[3] = (uint8_t)(crc_val & 0xFF);      // CRC LSB
    response[4] = (uint8_t)((crc_val >> 8) & 0xFF); // CRC MSB

    send_modbus_response(response, 5);
    printf("Enviada exceção Modbus: FC %02X, ExCode %02X\n", function_code, exception_code);
}

static void handle_modbus_request(uint8_t *request, int len) {
    if (len < 4) { // Mínimo para um comando Modbus RTU (SlaveID + FC + CRC)
        printf("Requisição muito curta (%d bytes).\n", len);
        return;
    }

    uint8_t slave_id = request[0];
    uint8_t function_code = request[1];
    uint16_t received_crc = (request[len - 1] << 8) | request[len - 2]; // CRC MSB | LSB (Little-endian no frame)

    if (slave_id != MODBUS_SLAVE_ID) {
        printf("ID de Slave incorreto (recebido %d, esperado %d).\n", slave_id, MODBUS_SLAVE_ID);
        return; // Não é para este slave
    }

    // Calcular CRC para validação (exclui os 2 bytes de CRC recebidos)
    uint16_t calculated_crc = calculate_crc16(request, len - 2);

    if (received_crc != calculated_crc) {
        printf("CRC inválido na requisição. Recebido: 0x%04X, Calculado: 0x%04X. Ignorando.\n", received_crc, calculated_crc);
        return;
    }

    printf("Requisição Modbus válida para Slave ID %d, FC 0x%02X.\n", slave_id, function_code);

    // Processar o comando Modbus
    switch (function_code) {
        case 0x03: { // Read Holding Registers
            // Requisição: [SlaveID] [0x03] [Addr H] [Addr L] [Quantity H] [Quantity L] [CRC L] [CRC H]
            if (len != 8) {
                send_modbus_exception_response(slave_id, function_code, 0x03); // Illegal Data Value
                break;
            }
            uint16_t starting_address = (request[2] << 8) | request[3];
            uint16_t quantity = (request[4] << 8) | request[5];

            if (quantity == 0 || quantity > 125 || (starting_address + quantity) > MODBUS_NUM_HOLDING_REGISTERS) {
                send_modbus_exception_response(slave_id, function_code, 0x02); // Illegal Data Address
                break;
            }

            // Resposta: [SlaveID] [0x03] [Byte Count] [Reg1_H] [Reg1_L] ... [CRC L] [CRC H]
            uint8_t byte_count = quantity * 2;
            uint8_t response_pdu_len = 3 + byte_count; // SlaveID + FC + ByteCount + Data
            uint8_t response_buffer[MODBUS_RX_BUFFER_SIZE]; // Use um buffer local
            response_buffer[0] = slave_id;
            response_buffer[1] = 0x03;
            response_buffer[2] = byte_count;

            if (xSemaphoreTake(xModbusRegMutex, portMAX_DELAY) == pdTRUE) {
                for (int i = 0; i < quantity; i++) {
                    uint16_t reg_val = holding_registers[starting_address + i];
                    response_buffer[3 + (i * 2)] = (uint8_t)((reg_val >> 8) & 0xFF); // MSB
                    response_buffer[4 + (i * 2)] = (uint8_t)(reg_val & 0xFF);       // LSB
                }
                xSemaphoreGive(xModbusRegMutex);
            } else {
                 send_modbus_exception_response(slave_id, function_code, 0x04); // Slave Device Busy
                 break;
            }

            uint16_t crc_response = calculate_crc16(response_buffer, response_pdu_len);
            response_buffer[response_pdu_len] = (uint8_t)(crc_response & 0xFF);
            response_buffer[response_pdu_len + 1] = (uint8_t)((crc_response >> 8) & 0xFF);

            send_modbus_response(response_buffer, response_pdu_len + 2);
            printf("FC 0x03: Lidos %d registradores a partir de 0x%04X.\n", quantity, starting_address);
            break;
        }

        case 0x06: { // Write Single Register
            // Requisição: [SlaveID] [0x06] [Addr H] [Addr L] [Value H] [Value L] [CRC L] [CRC H]
            if (len != 8) {
                send_modbus_exception_response(slave_id, function_code, 0x03);
                break;
            }
            uint16_t reg_addr = (request[2] << 8) | request[3];
            uint16_t reg_value = (request[4] << 8) | request[5];

            if (reg_addr >= MODBUS_NUM_HOLDING_REGISTERS) {
                send_modbus_exception_response(slave_id, function_code, 0x02);
                break;
            }

            modbus_set_holding_register(reg_addr, reg_value); // Usa a função segura com mutex
            printf("FC 0x06: Escrito H_Reg[0x%04X] = %d.\n", reg_addr, reg_value);

            // Resposta de sucesso é o eco exato da requisição
            send_modbus_response(request, len);
            break;
        }

        case 0x10: { // Write Multiple Registers
            // Requisição: [SlaveID] [0x10] [Addr H] [Addr L] [Num Reg H] [Num Reg L] [Byte Count] [Reg1_H] [Reg1_L] ... [CRC L] [CRC H]
            if (len < 9) { // Mínimo: SlaveID+FC+Addr+NumReg+ByteCount+CRC
                send_modbus_exception_response(slave_id, function_code, 0x03);
                break;
            }
            uint16_t starting_address = (request[2] << 8) | request[3];
            uint16_t num_registers = (request[4] << 8) | request[5];
            uint8_t byte_count = request[6];

            if (byte_count != (num_registers * 2)) {
                send_modbus_exception_response(slave_id, function_code, 0x03);
                break;
            }

            if (num_registers == 0 || num_registers > 123 || (starting_address + num_registers) > MODBUS_NUM_HOLDING_REGISTERS) {
                send_modbus_exception_response(slave_id, function_code, 0x02);
                break;
            }

            if (xSemaphoreTake(xModbusRegMutex, portMAX_DELAY) == pdTRUE) {
                for (int i = 0; i < num_registers; i++) {
                    uint16_t reg_value = (request[7 + (i * 2)] << 8) | request[8 + (i * 2)];
                    holding_registers[starting_address + i] = reg_value;
                    printf("FC 0x10: Escrito H_Reg[0x%04X] = %d\n", starting_address + i, reg_value);
                }
                xSemaphoreGive(xModbusRegMutex);
            } else {
                 send_modbus_exception_response(slave_id, function_code, 0x04); // Slave Device Busy
                 break;
            }

            // Resposta de sucesso para FC 0x10: [SlaveID] [0x10] [Addr H] [Addr L] [Num Reg H] [Num Reg L] [CRC L] [CRC H]
            uint8_t response_buffer[8];
            response_buffer[0] = slave_id;
            response_buffer[1] = 0x10;
            response_buffer[2] = (uint8_t)((starting_address >> 8) & 0xFF);
            response_buffer[3] = (uint8_t)(starting_address & 0xFF);
            response_buffer[4] = (uint8_t)((num_registers >> 8) & 0xFF);
            response_buffer[5] = (uint8_t)(num_registers & 0xFF);

            uint16_t crc_response = calculate_crc16(response_buffer, 6);
            response_buffer[6] = (uint8_t)(crc_response & 0xFF);
            response_buffer[7] = (uint8_t)((crc_response >> 8) & 0xFF);

            send_modbus_response(response_buffer, 8);
            break;
        }

        case 0x15: { // Write File Record
            // Requisição: [SlaveID] [0x15] [Req Data Len] [RefType] [FileNum H/L] [RecNum H/L] [RecLen H/L] [Record Data...] [CRC L/H]
            // Comprimento mínimo (1 SlaveID + 1 FC + 1 ReqDataLen + 1 RefType + 2 FileNum + 2 RecNum + 2 RecLen + 2 CRC = 12 bytes)
            if (len < 12) {
                send_modbus_exception_response(slave_id, function_code, 0x03);
                break;
            }

            uint8_t request_data_length = request[2];
            if (len != (3 + request_data_length + 2)) { // SlaveID + FC + ReqDataLen + Data + CRC
                send_modbus_exception_response(slave_id, function_code, 0x03); // Dados inválidos ou comprimento incorreto
                break;
            }

            uint8_t ref_type = request[3];
            if (ref_type != 0x06) {
                send_modbus_exception_response(slave_id, function_code, 0x03); // Tipo de referência inválido
                break;
            }

            uint16_t file_num = (request[4] << 8) | request[5];
            uint16_t record_num = (request[6] << 8) | request[7];
            uint16_t record_len_words = (request[8] << 8) | request[9]; // Length in words

            // Offset para o início dos dados do registro
            uint16_t data_start_offset = 10;
            uint16_t expected_data_bytes = record_len_words * 2;

            if ((data_start_offset + expected_data_bytes) > (len - 2)) { // Verifica se há bytes de dados suficientes antes do CRC
                send_modbus_exception_response(slave_id, function_code, 0x03); // Dados insuficientes
                break;
            }

            // Validações de limites para os arquivos/registros
            if (file_num >= MODBUS_MAX_FILES || record_num >= MODBUS_MAX_RECORDS_PER_FILE || record_len_words > MODBUS_MAX_WORDS_PER_RECORD) {
                send_modbus_exception_response(slave_id, function_code, 0x02); // Illegal Data Address (ou Illegal Data Value)
                break;
            }

            if (xSemaphoreTake(xModbusFileMutex, portMAX_DELAY) == pdTRUE) {
                // Processar os dados do registro e armazená-los
                modbus_files[file_num].records[record_num].current_length = record_len_words;
                for (int i = 0; i < record_len_words; i++) {
                    uint16_t val = (request[data_start_offset + (i * 2)] << 8) | request[data_start_offset + 1 + (i * 2)];
                    modbus_files[file_num].records[record_num].data[i] = val;
                }
                xSemaphoreGive(xModbusFileMutex);
            } else {
                 send_modbus_exception_response(slave_id, function_code, 0x04); // Slave Device Busy
                 break;
            }

            printf("FC 0x15: Escrito File %d, Record %d, Length %d palavras.\n", file_num, record_num, record_len_words);

            // RESPOSTA DE SUCESSO: ECO DA REQUISIÇÃO COMPLETA (ADU)
            send_modbus_response(request, len);
            break;
        }

        default:
            // Função não suportada
            send_modbus_exception_response(slave_id, function_code, 0x01); // Illegal Function
            break;
    }
}