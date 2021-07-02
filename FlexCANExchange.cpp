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

int16_t FlexCANExchange::Request(CAN_message_t TX, uint32_t ResponseID, uint8_t Retries,
    uint16_t Timeout, CAN_message_t *RX, bool Overwrite) {
    // Create the container pointer
    FlexCANExchangeContainer *Ptr;
    // Search if another with same ID exists
    // If it does overwrite it
    if (const int16_t PresentContainer = SearchContainers(ResponseID) == -1) {
        // Not found. Create a new container
        if (_ActiveContainers >= _MaxContainers) {
            return -2;
        }
        Ptr = &_Containers[_ActiveContainers++];
    } else {
        // Already exists
        if (Overwrite) {
            Ptr = &_Containers[PresentContainer];
        } else {
            if (_Containers[PresentContainer].Success) {
                return PresentContainer;
            } else {
                Tick(PresentContainer);
                return -3;
            }
        }
    }
    // Reset the container
    *Ptr = FlexCANExchangeContainer();
    Ptr->ResponseID = ResponseID;
    Ptr->Timeout = Timeout;
    Ptr->TriesLeft = Retries;
    Ptr->Request = TX;
    Ptr->ResponsePtr = RX;
    if (Transmit(Ptr->Request)) {
        Ptr->LastTry = millis();
        return -1;
    }
    return -4;
}

void FlexCANExchange::Tick(int16_t Container) {
    if (_Containers != nullptr && _CAN != nullptr && _ActiveContainers > 0) {
        if (Container == -1) {
            for (uint8_t Index = 0; Index < _ActiveContainers; Index++) {
                Tick(Index);
            }
        } else {
            if (Container < _ActiveContainers) {
                // Handle the sending of the container
                FlexCANExchangeContainer *Ptr = &_Containers[Container];
                if (Ptr->TriesLeft > 0) {
                    if (millis() > Ptr->LastTry + Ptr->Timeout) {
                        if (Transmit(Ptr->Request)) {
                            Ptr->TriesLeft--;
                            Ptr->LastTry = millis();
                        }
                    }
                } else {
                    Ptr->Expired = true;
                    GarbageCollection();
                }
            } else {
                // index out of bounds
            }
        }
    }
}

CAN_message_t FlexCANExchange::GetResponse(uint8_t Container) {
    CAN_message_t Response;
    if (Container < _ActiveContainers) {
        if (_Containers[Container].Success) {
            Response = _Containers[Container].Response;
            _Containers[Container].Processed = true;
            GarbageCollection();
        }
    }
    return Response;
}

void FlexCANExchange::GarbageCollection() {
    for (uint8_t Index = 0; Index < _ActiveContainers; Index++) {
        if (_Containers[Index].Expired || _Containers[Index].Processed) {
            for (uint8_t ReplaceIndex = Index; ReplaceIndex < (_ActiveContainers - 1); ReplaceIndex++) {
                _Containers[ReplaceIndex] = _Containers[ReplaceIndex + 1];
            }
            _ActiveContainers--;
        }
    }
}

bool FlexCANExchange::Process(CAN_message_t RX) {
    const int16_t Container = SearchContainers(RX.id);
    if (Container == -1) {
        return false;
    }
    // TODO: Check this condition
    FlexCANExchangeContainer *Ptr = &_Containers[Container];
    if (Container > -1 && !(Ptr->Expired || Ptr->Success)) {
        Ptr->Success = true;
        Ptr->Response = RX;
        if (Ptr->ResponsePtr != nullptr) {
            *(Ptr->ResponsePtr) = RX;
        }
    }
    return true;
}