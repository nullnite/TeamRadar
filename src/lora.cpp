#include "lora.h"

#define RF_FREQUENCY 868300000   // Hz
#define TX_OUTPUT_POWER 22       // dBm
#define LORA_BANDWIDTH 0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 9  // [SF7..SF12]
#define LORA_CODINGRATE 4        // [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 1000
#define TX_TIMEOUT_VALUE 1000

static RadioEvents_t RadioEvents;

static uint8_t RcvBuffer[64];
static uint8_t TxdBuffer[64];

coords receivedLocations[teamSize] = {0};

void OnRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr);
void OnRxTimeout(void);
void OnTxDone(void);
void OnTxTimeout(void);

void getTeamLocations(coords* locations) {
    memcpy(locations, &receivedLocations, sizeof(receivedLocations));
}

void sendLocation(coords location) {
    lora_packet packet;
    packet.location = location;

    memcpy(TxdBuffer, &packet, sizeof(packet));

    Radio.Standby();
    Radio.Send(TxdBuffer, sizeof(packet));
}

void initLora() {
    lora_rak4630_init();

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = NULL;
    RadioEvents.CadDone = NULL;

    Radio.Init(&RadioEvents);

    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

    Radio.Rx(0);
}

void OnRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr) {
    memcpy(RcvBuffer, payload, size);

    lora_packet packet;
    memcpy(&packet, RcvBuffer, sizeof(packet));

    if (packet.ID < teamSize) {
        Serial.printf("LoRa packet received from ID %d\r\n", packet.ID);
        receivedLocations[packet.ID] = packet.location;
    }

    Radio.Rx(0);
}

void OnRxTimeout(void) {
    Radio.Rx(0);
}

void OnTxDone(void) {
    Serial.println("LoRa packet sent");

    Radio.Rx(0);
}

void OnTxTimeout(void) {
}