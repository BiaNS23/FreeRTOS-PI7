// modbus_crc.h (ou diretamente no seu main.c ou communications.c)

#include <stdint.h> // Para uint8_t, uint16_t

uint16_t calculate_crc16(uint8_t *buf, int len) {
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos]; // XOR byte into LSB of crc
        for (int i = 8; i != 0; i--) { // Loop over each bit
            if ((crc & 0x0001) != 0) { // If the LSB is set
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= 0xA001;
            } else { // Else LSB is not set
                crc >>= 1; // Just shift right
            }
        }
    }
    // Note: The Modbus RTU specification states that the CRC is transmitted
    // LSB first, then MSB. The function already returns the correct 16-bit value.
    // When placing into a byte array for transmission, split it.
    return crc;
}

/*
// Exemplo de uso e como obter os bytes LSB/MSB
void test_crc() {
    uint8_t test_data[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A}; // Ex: Read 10 registers from address 0x0000, Slave ID 1
    uint16_t crc_val = calculate_crc16(test_data, sizeof(test_data));
    printf("CRC C Calculado: 0x%04X\n", crc_val); // Deve ser 0xCD B5 (0xCD como MSB, 0xB5 como LSB)
                                                // Modbus RTU envia B5 CD (LSB primeiro)
    uint8_t crc_lsb = (uint8_t)(crc_val & 0xFF);
    uint8_t crc_msb = (uint8_t)((crc_val >> 8) & 0xFF);
    printf("CRC LSB: 0x%02X, CRC MSB: 0x%02X\n", crc_lsb, crc_msb);

    // Para 01060001000A, CRC deve ser 0xEA 99 (0xEA como LSB, 0x99 como MSB)
    uint8_t test_data_2[] = {0x01, 0x06, 0x00, 0x01, 0x00, 0x0A};
    uint16_t crc_val_2 = calculate_crc16(test_data_2, sizeof(test_data_2));
    printf("CRC C Calculado 2: 0x%04X\n", crc_val_2); // Deve ser 0x99EA
    uint8_t crc_lsb_2 = (uint8_t)(crc_val_2 & 0xFF);
    uint8_t crc_msb_2 = (uint8_t)((crc_val_2 >> 8) & 0xFF);
    printf("CRC LSB 2: 0x%02X, CRC MSB 2: 0x%02X\n", crc_lsb_2, crc_msb_2);
}
*/