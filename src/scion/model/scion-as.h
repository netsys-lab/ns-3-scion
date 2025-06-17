/*
 * This class is responsible for holding SCION specific attributes. It extends the BaseAs class and
 * also only serves as an attribute store. SCION specific attributes are ISD, Beacon Policies, Core
 * AS settings, and potentially the TRC and crypto information in a future release.
 */

#ifndef SCION_MODEL_SCION_AS_H
#define SCION_MODEL_SCION_AS_H

#include "ns3/as.h"
#include "ns3/attribute-accessor-helper.h" // For accessor helpers
#include "ns3/attribute.h"                 // Needed for IaValue etc.
#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/scion-ia.h"
#include "ns3/string.h" // For StringValue

namespace ns3
{

class ScionAs : public BaseAs
{
  public:
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void DoDispose(void);

    // Constructor
    ScionAs();

    // Destructor
    virtual ~ScionAs();

    // Setters
    void SetIsCore(bool isCore);
    void SetIa(const Ia& ia);
    void SetBeaconPolicy(uint32_t policy);

    // Getters
    const Ia& GetIa() const;
    Isd GetIsd() const;
    Asn GetAs() const;

    bool IsCore() const;
    void SetCore(bool core);

    uint32_t GetBeaconPolicy() const;

  private:
    Ia m_ia; // The full ISD-AS identifier.
    bool m_isCore;
    uint32_t m_beaconPolicy; // Beacon policy
};

} // namespace ns3

#endif // SCION_MODEL_SCION_AS_H