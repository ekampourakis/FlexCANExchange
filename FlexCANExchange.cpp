#include "FlexCANExchange.h"

FlexCANExchange::FlexCANExchange() {}

FlexCANExchange::FlexCANExchange(FlexCAN *CAN, uint8_t Containers) { Init(CAN, Containers); }

FlexCANExchange::~FlexCANExchange() {
    _MaxContainers = 0;
    _Containers = nullptr;
    _CAN = nullptr;
}

void FlexCANExchange::Init(FlexCAN *CAN, uint8_t Containers) {
    _CAN = CAN;
    _MaxContainers = Containers;
    _Containers = (FlexCANExchangeContainer *)malloc(sizeof(FlexCANExchangeContainer) * Containers);
}

bool FlexCANExchange::Transmit(CAN_message_t Message) {
    if (_CAN != nullptr) {
        if (_CAN->write(Message)) {
            return true;
        }
    }
    return false;
}

int16_t FlexCANExchange::SearchContainers(uint32_t ID) {
    for (uint8_t Container = 0; Container < _ActiveContainers; Container++) {
        if (_Containers[Container].ResponseID == ID) {
            return Container;
        }
    }
    return -1;
}

int8_t FlexCANExchange::Request(
    CAN_message_t TX, uint32_t ResponseID, uint8_t Retries, uint16_t Timeout, CAN_message_t *RX) {

    // If it does overwrite it?
    // Create the container and start initializing it
    // Search if another with same ID exists
    FlexCANExchangeContainer *Ptr;
    if (const int16_t PresentContainer = SearchContainers(ResponseID) == -1) {
        // Not found. Create a new container
        Ptr = &_Containers[_ActiveContainers++];
    } else {
        // Already exists
        Ptr = &_Containers[PresentContainer];
    }
    // Reset the container
    *Ptr = FlexCANExchangeContainer();
    Ptr->ResponseID = ResponseID;
    Ptr->Timeout = Timeout;
    Ptr->TriesLeft = Retries;
    // Ptr->LastTry = 0;
    Ptr->Request = TX;
    Ptr->ResponsePtr = RX;


    return -1;
}

bool FlexCANExchange::Handle(CAN_message_t *Message) {
    if (_Containers != nullptr) {
    }
    return false;
}

// if (_CAN == nullptr || _Containers != nullptr && ) {
// 	// Put it in the containers

// 	if (!_CAN.write(Message)) {
// 		// If the write wasn't successful, then add one more try
// 	}
// 	return true;
// }

// return 1;