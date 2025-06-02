
#include "ns3/hop-field.h"

#if __cplusplus >= 202002L
#include <format>
#endif
#include <sstream>

namespace ns3
{
// DecodeFromBytes populates the fields from a raw buffer.
// The buffer must be of length >= path.HopLen.
// @ requires  len(raw) >= HopLen
// DecodeFromBytes modifies the fields of *h and reads (but does not modify) the contents of raw.
// @ preserves acc(h) && acc(raw, 1/2)
// When a call that satisfies the precondition (len(raw) >= HopLen) is made,
// the return value is guaranteed to be nil.
// @ ensures   err == nil
// Calls to DecodeFromBytes are always guaranteed to terminate.
// @ decreases


// SerializeTo writes the fields into the provided buffer.
// The buffer must be of length >= path.HopLen.
// @ requires  len(b) >= HopLen
// SerializeTo reads (but does not modify) the fields of *h and writes to the contents of b.
// @ preserves acc(h, 1/2) && acc(b)
// When a call that satisfies the precondition (len(b) >= HopLen) is made,
// the return value is guaranteed to be nil.
// @ ensures   err == nil
// Calls to SerializeTo are guaranteed to terminate.
// @ decreases

// in bytes (in serialized form)
constexpr uint8_t HopField::Len(){
    return sizeof(HopField); 
}

HopField::operator std::string() const
{
    #ifndef SCION_SIMPLIFIED_SIMULATION
    #if __cplusplus >= 202002L
    return std::format( "[EgressRouterAlert: {}, IngressRouterAlert: {}, ExpTime: {} , ConsIngress: {}, ConsEgress: {} ]"
                ,EgressRouterAlert, IngressRouterAlert, ExpTime, ConsIngress, ConsEgress ) ;
    #else
    std::stringstream ss;
    ss << "[EgressRouterAlert: " << EgressRouterAlert;
    ss << ", IngressRouterAlert: " << IngressRouterAlert;
    ss << ", ConsIngress: "  << ConsIngress;
    ss << ", ConsEgress: " << ConsEgress << ", ExpTime: " << ExpTime <<  "]";
    return ss.str();
    #endif                
    #else
    std::stringstream ss;
    ss << "[EgressRouterAlert: " << EgressRouterAlert;
    ss << ", IngressRouterAlert: " << IngressRouterAlert;
    ss << ", ConsIngress: "  << ConsIngress;
    ss << ", ConsEgress: " << ConsEgress << "]";
    #endif
 
}

uint32_t
HopField::Deserialize( Buffer::Iterator start)
{
    auto fstByte = start.ReadU8();
    EgressRouterAlert = (fstByte & 0x1 )== 0x1;
    IngressRouterAlert =( fstByte & 0x2 )== 0x2;
    #ifndef SCION_SIMPLIFIED_SIMULATION
    ExpTime = start.ReadU8();
    #endif
    ConsIngress = start.ReadNtohU16();
    ConsEgress = start.ReadNtohU16();
    #ifndef SCION_SIMPLIFIED_SIMULATION
    start.Read(Mac, MACLEN);
    #endif
    return sizeof(HopField);
}

bool HopField::operator!=(const HopField& other) const
{
    return ! this->operator==(other);
}

bool HopField::operator==(const HopField& other) const
{
    return ConsIngress==other.ConsIngress && ConsEgress == other.ConsEgress;
}

void
HopField::Serialize( Buffer::Iterator start) const
{
    NS_ASSERT_MSG( start.GetRemainingSize() >= Len(),
         "Not enough buffer space to serialize HopField" );

    uint8_t fstByte = 0;
    if (EgressRouterAlert)
    {
            fstByte |= 0x1;
    }
    if (IngressRouterAlert )
    {
            fstByte |= 0x2;
    }
    start.WriteU8(fstByte);
    #ifndef SCION_SIMPLIFIED_SIMULATION
    start.WriteU8(ExpTime);
    #endif
    start.WriteHtonU16(ConsIngress);
    start.WriteHtonU16(ConsEgress);
    #ifndef SCION_SIMPLIFIED_SIMULATION
    start.Write(Mac, MACLEN);
    #endif
}

}