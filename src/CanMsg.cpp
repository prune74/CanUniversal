// ============================================================================
//  CanMsg — constructeurs
//  CanUniversal v3.0.0
// ============================================================================

#include "CanMsg.h"

CanMsg::CanMsg()
    : id(0), dlc(0)
{
}

CanMsg::CanMsg(uint16_t id11, std::initializer_list<uint8_t> payload)
    : id(id11), dlc(payload.size())
{
    if (dlc > 8) dlc = 8;
    int i = 0;
    for (uint8_t b : payload) {
        if (i >= 8) break;
        data[i++] = b;
    }
}

CanMsg::CanMsg(uint32_t id29, std::initializer_list<uint8_t> payload)
    : id(id29), dlc(payload.size())
{
    if (dlc > 8) dlc = 8;
    int i = 0;
    for (uint8_t b : payload) {
        if (i >= 8) break;
        data[i++] = b;
    }
}