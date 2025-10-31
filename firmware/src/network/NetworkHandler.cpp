#include "NetworkHandler.h"
#include "../display/DisplayHandler.h"

bool eth_connected = false;

void onEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      ETH.setHostname("SAMSUNG-HVAC-ADAPTER");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      displayHandler.addNewLine();
      displayHandler.write("Network IP:");
      displayHandler.write(ETH.localIP().toString().c_str());
      displayHandler.addNewLine();
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      eth_connected = false;
      break;
    default:
      break;
  }
}

void setupNetwork() {

  Network.onEvent(onEvent);
  ETH.begin();

}