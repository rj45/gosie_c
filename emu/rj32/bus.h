#ifndef BUS_H
#define BUS_H

#include <stdbool.h>
#include <stdint.h>

// Bus is a 16 bit data bus.
typedef struct Bus {
  uint16_t data;
  uint16_t address;
  bool WE;
} Bus;

// BusHandler is a function that handles a bus transaction. It returns true
// if the transaction was handled, false otherwise. If it returns false,
// any "underlying" device can be called to handle the transaction.
typedef bool (*BusHandler)(void *context, Bus *bus, uint16_t subAddress);

// Device is a device on the bus. The address is the start of the address
// range that the device responds to, and the size is the number of words
// in the address range. context is used to pass data to the handler.
// Devices can have overlapping address ranges, and if the handler returns
// false then next device in the list will be called, otherwise the
// transaction ends and no further devices will be called.
typedef struct Device {
  uint16_t address;
  uint16_t size;
  BusHandler handler;
  void *context;
} Device;

// IOBus is a bus with devices attached to it.
typedef struct IOBus {
  Bus bus;
  Device *busDevices;
  int numDevices;
} IOBus;

// ioBusInit initializes an IOBus with a list of devices, listed in
// priority order.
void ioBusInit(IOBus *ioBus, Device *devices, int numDevices);

// ioBusTransaction handles a bus transaction. If the transaction is
// handled it returns true, otherwise it returns false. Devices are
// tried in order, the first to return true is the last device tried.
bool ioBusTransaction(IOBus *ioBus, uint16_t address, uint16_t data, bool WE);

// stdoutWriter is a busHandler that writes to stdout, handling \r
// specially to emulate the same behaviour as telnet / serial terminals.
bool stdoutWriter(void *context, Bus *bus, uint16_t address);

// memoryHandler is a busHandler for RAM memory. The context is a pointer
// to the memory array. The memory size is not checked, so it's best to
// make the memory 64K words, or 128kb in size.
bool memoryHandler(void *context, Bus *bus, uint16_t address);

#endif
