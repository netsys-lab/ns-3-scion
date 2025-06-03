#pragma once
#include <stdint.h>

namespace ns3{

// MacLen is the size of the MAC of each HopField.
#define MACLEN 6

// MaxTTL is the maximum age of a HopField in seconds.
#define MaxTTL (24 * 60 * 60) // One day in seconds

#define expTimeUnit (MaxTTL / 256) // ~5m38s


// MetaLen is the length of the PathMetaHeader.
#define METALEN  4

// optimization which removes Timestamp and SegmentID from InfoFields
// as well as Expiry and MAC from HopFields
#define SCION_SIMPLIFIED_SIMULATION

}