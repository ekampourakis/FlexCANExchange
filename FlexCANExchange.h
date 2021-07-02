#ifndef FLEXCAN_EXCHANGE_H
#define FLEXCAN_EXCHANGE_H

#include <Arduino.h>
#include <FlexCAN.h>

class FlexCANExchange {
    struct FlexCANExchangeContainer {
        uint32_t ResponseID;
        uint16_t Timeout;
        uint8_t TriesLeft;
        unsigned long LastTry;
        bool Expired;
        bool Success;
        bool Processed;
        CAN_message_t Request;
        CAN_message_t Response;
        CAN_message_t *ResponsePtr = nullptr;
    };

public:
    FlexCANExchange();
    FlexCANExchange(FlexCAN *CAN, uint8_t Containers = 10);
    ~FlexCANExchange();
    void Init(FlexCAN *CAN, uint8_t Containers = 10);
    int16_t Request(CAN_message_t TX, uint32_t ResponseID, uint8_t Retries = 3,
        uint16_t Timeout = 100, CAN_message_t *RX = nullptr, bool Overwrite = false);
    void Tick(int16_t Container = -1);
    bool Process(CAN_message_t RX);
    CAN_message_t GetResponse(uint8_t Container);

private:
    bool Transmit(CAN_message_t Message);
    int16_t SearchContainers(uint32_t ID);
    void GarbageCollection();

    FlexCAN *_CAN = nullptr;
    FlexCANExchangeContainer *_Containers = nullptr;
    uint8_t _ActiveContainers;
    uint8_t _MaxContainers;
};

#endif