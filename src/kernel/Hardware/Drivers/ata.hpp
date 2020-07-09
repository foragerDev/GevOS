#ifndef ATA_HPP
#define ATA_HPP

#include "../port.hpp"
#include "stdio.hpp"
#include "string.hpp"
#include "types.hpp"

class AdvancedTechnologyAttachment {

protected:
    Port16Bit dataPort;
    Port8Bit errorPort;
    Port8Bit sectorCountPort;
    Port8Bit lbaLowPort;
    Port8Bit lbaMidPort;
    Port8Bit lbaHiPort;
    Port8Bit devicePort;
    Port8Bit commandPort;
    Port8Bit controlPort;

    bool master;

public:
    AdvancedTechnologyAttachment(bool master, uint16_t portBase);
    ~AdvancedTechnologyAttachment();

    void Identify();
    char* Read28(uint32_t sectorNum, int count = 512);
    void Write28(uint32_t sectorNum, uint8_t* data, uint32_t count);
    void Flush();
};

#endif