#ifndef FLEXCAN_EXCHANGE_H
#define FLEXCAN_EXCHANGE_H

#include <Arduino.h>
#include <FlexCAN.h>

class FlexCANExchange {
public:
    void Init(FlexCAN *CAN, uint8_t Containers = 10);
    void Handle(CAN_message_t *Message);
    void Request(const CAN_message_t *TX, const uint32_t ResponseID, CAN_message_t *RX = nullptr);
    bool Available(bool Remove = true);

private:
    FlexCAN *_CAN;
    uint8_t _Containers;
};

#endif