#pragma once

/**
 * ============================================================================
 *  🌐 CanUniversal — En-tête principal
 * ============================================================================
 *  Incluez simplement :
 *
 *      #include <CanUniversal.h>
 *
 *  pour obtenir l'API complète de la bibliothèque :
 *    - CanConfig / CanConfigProvider  (configuration matérielle applicative)
 *    - CanInit                        (initialisation multi-bus)
 *    - CanBus                         (envoi / réception unifiés)
 *    - CanMsg                         (trame CAN universelle)
 *    - CanID                          (IDs 29 bits Locoduino / Chemins de Fer du Chablais (CFC))
 * ============================================================================
 */

#include "CanConfig.h"
#include "CanID.h"
#include "CanMsg.h"
#include "CanBus.h"
#include "CanInit.h"