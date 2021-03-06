#include "pci.hpp"
#include "Drivers/amd79.hpp"

PCIcontrollerDeviceDescriptor::PCIcontrollerDeviceDescriptor()
{
}

PCIcontrollerDeviceDescriptor::~PCIcontrollerDeviceDescriptor()
{
}

PCIcontroller::PCIcontroller()
    : dataport(0xCFC)
    , commandport(0xCF8)
{
}

PCIcontroller::~PCIcontroller()
{
}

Driver* PCIcontroller::GetDriver(PCIcontrollerDeviceDescriptor dev, InterruptManager* interrupts)
{
    Driver* driver = 0;
    switch (dev.vendor_id) {
    case 0x1022:
        switch (dev.device_id) {
        case 0x2000:
            driver = (AmdDriver*)kmalloc(sizeof(AmdDriver));
            if (driver != 0) {
                new (driver) AmdDriver(&dev, interrupts);
            }
            return driver;
        }
        break;
    }

    switch (dev.class_id) {
    case 0x03:
        switch (dev.subclass_id) {
        case 0x00:
            break;
        }
        break;
    }

    return driver;
}

BaseAddressRegister PCIcontroller::GetBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar)
{
    BaseAddressRegister result;
    uint32_t headertype = Read(bus, device, function, 0x0E) & 0x7F;
    int maxBARs = 6 - (4 * headertype);
    if (bar >= maxBARs)
        return result;

    uint32_t bar_value = Read(bus, device, function, 0x10 + 4 * bar);
    result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;
    uint32_t temp;

    if (result.type == MemoryMapping) {
        switch ((bar_value >> 1) & 0x3) {

        case 0: // 32 Bit Mode
        case 1: // 20 Bit Mode
        case 2: // 64 Bit Mode
            break;
        }

    } else // InputOutput
    {
        result.address = (uint8_t*)(bar_value & ~0x3);
        result.prefetchable = false;
    }
    return result;
}

uint32_t PCIcontroller::Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOfsset)
{
    uint32_t id = 0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registerOfsset & 0xFC);

    commandport.Write(id);
    uint32_t result = dataport.Read();
    return result >> (8 * (registerOfsset % 4));
}

void PCIcontroller::Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOfsset, uint32_t value)
{
    uint32_t id = 0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registerOfsset & 0xFC);
    commandport.Write(id);
    dataport.Write(value);
}

bool PCIcontroller::DeviceHasFunctions(uint16_t bus, uint16_t device)
{
    return Read(bus, device, 0, 0x0E) & (1 << 7);
}

void PCIcontroller::SelectDrivers(DriverManager* driverManager, InterruptManager* interrupts)
{
    for (int bus = 0; bus < 8; bus++) {
        for (int device = 0; device < 32; device++) {
            int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;
            for (int function = 0; function < numFunctions; function++) {
                dev = GetDeviceDescriptor(bus, device, function);

                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
                    break;

                for (int barNum = 0; barNum < 6; barNum++) {
                    BaseAddressRegister bar = GetBaseAddressRegister(bus, device, function, barNum);
                    if (bar.address && (bar.type == InputOutput))
                        dev.port_base = (uint32_t)bar.address;
                }

                Driver* driver = GetDriver(dev, interrupts);
                if (driver != 0) {
                    driverManager->AddDriver(driver);
                    klog("amd79 activated");
                }
            }
        }
    }
}

PCIcontrollerDeviceDescriptor* PCIcontroller::GetDescriptor()
{
    return &dev;
}

PCIcontrollerDeviceDescriptor PCIcontroller::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function)
{
    PCIcontrollerDeviceDescriptor result;

    result.bus = bus;
    result.device = device;
    result.function = function;

    result.vendor_id = Read(bus, device, function, 0x00);
    result.device_id = Read(bus, device, function, 0x02);

    result.class_id = Read(bus, device, function, 0x0b);
    result.subclass_id = Read(bus, device, function, 0x0a);
    result.interface_id = Read(bus, device, function, 0x09);

    result.revision = Read(bus, device, function, 0x08);
    result.interrupt = Read(bus, device, function, 0x3c);

    return result;
}
