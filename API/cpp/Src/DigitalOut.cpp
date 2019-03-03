#include "DigitalOut.hpp"

DigitalOut::DigitalOut(PinName pin, PullType pull, PinMode mode, PinSpeed speed,
        bool invert) : pin(pin), invert(invert) {
    GPIO_InitTypeDef pinStructure;
    pinStructure.Pin = pin.pin;
    pinStructure.Pull = pull;
    pinStructure.Mode = mode;
    pinStructure.Speed = speed;

    HAL_GPIO_Init(pin.port, &pinStructure);
}
DigitalOut::~DigitalOut() {
    HAL_GPIO_DeInit(pin.port, pin.pin);
}

void DigitalOut::write(bool state) {
    HAL_GPIO_WritePin(pin.port, pin.pin, (GPIO_PinState) (state != invert));
}

void DigitalOut::toggle() {
    HAL_GPIO_TogglePin(pin.port, pin.pin);
}

bool DigitalOut::read() {
    return HAL_GPIO_ReadPin(pin.port, pin.pin);
}
