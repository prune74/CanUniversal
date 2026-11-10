// ============================================================================
//  CanInit — initialisation tolérante aux erreurs
//  CanUniversal v3.0.0
// ============================================================================

#pragma once
#include "CanConfig.h"

struct CanInit
{
    /// Initialise tous les bus décrits par le provider.
    /// Toujours true : un bus raté est marqué invalide côté CanBus.
    static bool begin(const CanConfigProvider &provider);
};