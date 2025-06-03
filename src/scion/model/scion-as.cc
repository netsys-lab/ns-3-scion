#include "ns3/scion-as.h"

#include "ns3/abort.h"
#include "ns3/attribute-accessor-helper.h" // <--- ADD THIS LINE
#include "ns3/log.h"

namespace ns3
{

// --- ScionAs Implementation ---

TypeId
ScionAs::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::ScionAs")
            .SetParent<BaseAs>() // Set appropriate parent
            .SetGroupName("Scion")
            .AddConstructor<ScionAs>()
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
                          MakeUintegerAccessor(&ScionAs::m_beaconPolicy),
                          MakeUintegerChecker<uint32_t>());
    return tid;
}

TypeId
ScionAs::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

ScionAs::ScionAs()
    : m_ia(), // Default construct Ia (0-0)
      m_beaconPolicy(0)
{
    NS_LOG_FUNCTION(this);
    SetAsn(m_ia.GetAsn().GetValue()); // Synchronize BaseAs AS number
}

ScionAs::~ScionAs()
{
    NS_LOG_FUNCTION(this);
}

void
ScionAs::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    // Clean up resources if any
    BaseAs::DoDispose(); // Call parent class Dispose
}

void
ScionAs::SetIsCore(bool isCore)
{
    NS_LOG_FUNCTION(this << isCore);
    m_isCore = isCore;
}

void
ScionAs::SetIa(const Ia& ia)
{
    NS_LOG_FUNCTION(this << ia);
    m_ia = ia;
    SetAsn(ia.GetAsn().GetValue()); // Synchronize BaseAs AS number
}

void
ScionAs::SetBeaconPolicy(uint32_t policy)
{
    NS_LOG_FUNCTION(this << policy);
    m_beaconPolicy = policy;
}

const Ia&
ScionAs::GetIa() const
{
    NS_LOG_FUNCTION(this);
    return m_ia;
}

uint32_t
ScionAs::GetBeaconPolicy() const
{
    NS_LOG_FUNCTION(this);
    return m_beaconPolicy;
}

Isd
ScionAs::GetIsd() const
{
    NS_LOG_FUNCTION(this);
    return m_ia.GetIsd();
}

Asn
ScionAs::GetAs() const
{
    NS_LOG_FUNCTION(this);
    return m_ia.GetAsn();
}

} // namespace ns3