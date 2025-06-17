#include "ns3/scion-as-context.h"

#include "ns3/abort.h"
#include "ns3/attribute-accessor-helper.h"
#include "ns3/log.h"

namespace ns3
{

// --- ScionAs Implementation ---

TypeId
ScionAsContext::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::ScionAsContext")
            .SetGroupName("Scion")
            .AddConstructor<ScionAsContext>()
            //.AddAttribute("Ia", // Use the Ia class
            //              "The ISD-AS identifier (e.g., '1-ffaa:1:2').",
            // Initial value: Default Ia (0-0)
            //              IaValue(Ia()), // Or StringValue("0-0:0:0") if using StringAccessor
            // Accessor: Use our custom IaValue helpers
            //              ns3::MakeAccessor(&ScionAs::GetIaAttribute,  // Pointer to getter
            //                           &ScionAs::SetIaAttribute), // Pointer to setter
            // Checker: Use the Ia checker (can be simple StringChecker)
            //              MakeIaChecker())
            .AddAttribute("BeaconPolicy",
                          "Identifier for the beaconing policy used by this AS.",
                          UintegerValue(0), // Default value 0
                          MakeUintegerAccessor(&ScionAsContext::m_beaconPolicy),
                          MakeUintegerChecker<uint32_t>());
    return tid;
}

TypeId
ScionAsContext::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

ScionAsContext::ScionAsContext()
    : m_beaconPolicy(0),
      m_topology()
{
    NS_LOG_FUNCTION(this);
}

ScionAsContext::~ScionAsContext()
{
    NS_LOG_FUNCTION(this);
}

void
ScionAsContext::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    // Clean up resources if any
}

void
ScionAsContext::SetIsCore(bool isCore)
{
    NS_LOG_FUNCTION(this << isCore);
    m_topology.isCore = isCore;
}

void
ScionAsContext::SetTopology(ScionAsTopology topology)
{
    m_topology = topology;
}

void
ScionAsContext::SetIa(const Ia& ia)
{
    NS_LOG_FUNCTION(this << ia);
    m_topology.isdAs = ia;
}

void
ScionAsContext::SetBeaconPolicy(uint32_t policy)
{
    NS_LOG_FUNCTION(this << policy);
    m_beaconPolicy = policy;
}

const Ia&
ScionAsContext::GetIa() const
{
    NS_LOG_FUNCTION(this);
    return m_topology.isdAs;
}

uint32_t
ScionAsContext::GetBeaconPolicy() const
{
    NS_LOG_FUNCTION(this);
    return m_beaconPolicy;
}

Isd
ScionAsContext::GetIsd() const
{
    NS_LOG_FUNCTION(this);
    return m_topology.isdAs.GetIsd();
}

Asn
ScionAsContext::GetAs() const
{
    NS_LOG_FUNCTION(this);
    return m_topology.isdAs.GetAsn();
}

} // namespace ns3