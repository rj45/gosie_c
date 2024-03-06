#include "bus.h"

#include <memory.h>
#include <stdio.h>

void ioBusInit(IOBus *ioBus, Device *devices, int numDevices) {
  memset(ioBus, 0, sizeof(IOBus));
  ioBus->busDevices = devices;
  ioBus->numDevices = numDevices;
}

bool ioBusTransaction(IOBus *ioBus, uint16_t address, uint16_t data, bool WE) {
  ioBus->bus.address = address;
  ioBus->bus.data = data;
  ioBus->bus.WE = WE;
  for (int i = 0; i < ioBus->numDevices; i++) {
    uint16_t deviceAddress = ioBus->busDevices[i].address;
    uint16_t deviceSize = ioBus->busDevices[i].size;
    if (address >= deviceAddress && address < deviceAddress + deviceSize) {
      if (ioBus->busDevices[i].handler(ioBus->busDevices[i].context,
                                       &ioBus->bus, address - deviceAddress)) {
        return true;
      }
    }
  }
  return false;
}

bool stdoutWriter(void *context, Bus *bus, uint16_t address) {
#pragma unused(context)
#pragma unused(address)
  if (bus->WE) {
    char c = bus->data;
    if (c == '\r') {
      printf("\033D");
    } else {
      printf("%c", c);
    }
    return true;
  }
  return false;
}

bool memoryHandler(void *context, Bus *bus, uint16_t address) {
  uint16_t *mem = context;
  if (bus->WE) {
    mem[address] = bus->data;
    return true;
  }
  bus->data = mem[address];
  return true;
}