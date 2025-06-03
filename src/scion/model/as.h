/*
 * The “BaseAs” class serves as an attribute store for common AS attributes like (ASN, MTU, etc).
 * It inherits from “Object” and acts as a super class for SCION ASes (and later also for BGP ASes
 * if desired). This provides a common interface for both routing architectures to (co)exist.
 */

#ifndef SCION_MODEL_AS_H
#define SCION_MODEL_AS_H

#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("BaseAs");

class BaseAs : public ns3::Object
{
  public:
    static ns3::TypeId GetTypeId(void);
    virtual ns3::TypeId GetInstanceTypeId(void) const;
    virtual void DoDispose(void);

    // Constructor
    BaseAs();

    // Destructor
    virtual ~BaseAs();

    // Setters
    void SetAsn(uint32_t asn);
    void SetMtu(uint32_t mtu);

    // Getters
    uint32_t GetAsn() const;
    uint32_t GetMtu() const;

  private:
    uint32_t m_asn; // Autonomous System Number
    uint32_t m_mtu; // Maximum Transmission Unit
};
} // namespace ns3
#endif // SCION_MODEL_AS_H