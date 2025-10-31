#ifndef NETWORK_HANDLER_H
#define NETWORK_HANDLER_H

#include <ETH.h>

extern bool eth_connected;

void onEvent(arduino_event_id_t event);
void setupNetwork();

#endif // NETWORK_HANDLER_H