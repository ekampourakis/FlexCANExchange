#include "FlexCANExchange.h"

void FlexCANExchange::Init(FlexCAN *CAN, uint8_t Containers) {
	_CAN = CAN;
	_Containers = Containers;
}

void FlexCANExchange::Handle(CAN_message_t *Message) {

}