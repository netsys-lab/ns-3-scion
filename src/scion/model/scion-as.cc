#include "ns3/scion-as.h"

#include "ns3/abort.h"
#include "ns3/attribute-accessor-helper.h" // <--- ADD THIS LINE
#include "ns3/log.h"

namespace ns3
{

// --- IaValue Implementation ---

IaValue::IaValue()
    : m_value() // Default construct Ia (0-0)
{
}

IaValue::IaValue(const Ia& value)
    : m_value(value)
{
}

Ia
IaValue::Get() const
{
    return m_value;
}

void
IaValue::Set(const Ia& value)
{
    m_value = value;
}

Ptr<AttributeValue>
IaValue::Copy() const
{
    return Create<IaValue>(*this);
}

std::string
IaValue::SerializeToString(Ptr<const AttributeChecker> checker) const
{
    // Delegate checking? String checker doesn't do much here.
    return m_value.ToString();
}

bool
IaValue::DeserializeFromString(std::string value, Ptr<const AttributeChecker> checker)
{
    // Delegate checking?
    Ia parsed_ia;
    if (Ia::FromString(value, parsed_ia))
    {
        m_value = parsed_ia;
        return true;
    }
    NS_LOG_WARN("IaValue::DeserializeFromString: Failed to parse IA from string '" << value << "'");
    return false;
}

std::ostream&
operator<<(std::ostream& os, const IaValue& value)
{
    os << value.Get(); // Use Ia's stream operator
    return os;
}

std::istream&
operator>>(std::istream& is, IaValue& value)
{
    std::string s;
    is >> s;
    Ia ia;
    if (!Ia::FromString(s, ia))
    {
        // Match behavior of other Value types: set failbit on stream
        is.setstate(std::ios::failbit);
    }
    value.Set(ia);
    return is;
}

// --- Ia Helpers for Attributes ---

Ptr<AttributeValue>
MakeIaValue(const Ia& ia)
{
    return Create<IaValue>(ia);
}

// Use the existing StringChecker as Ia is represented by string
Ptr<const AttributeChecker>
MakeIaChecker()
{
    return MakeStringChecker();
}

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
    // Potentially initialize BaseAs AS number here if needed, derived from default m_ia
    SynchronizeBaseAsNumber();
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
ScionAs::SetIaAttribute(Ia ia)
{
    NS_LOG_FUNCTION(this << ia);
    // This method is called by the attribute system.
    // It might receive an Ia object created from a string via IaValue::DeserializeFromString.
    m_ia = ia;
    SynchronizeBaseAsNumber(); // Ensure consistency with base class
}

Ia
ScionAs::GetIaAttribute() const
{
    NS_LOG_FUNCTION(this);
    // This method is called by the attribute system.
    // It should return the current value.
    return m_ia;
}

void
ScionAs::SetIa(const Ia& ia)
{
    NS_LOG_FUNCTION(this << ia);
    m_ia = ia;
    // *** Important: Synchronize with BaseAs if it also stores AS number ***
    SynchronizeBaseAsNumber();
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
    return m_ia.GetAs();
}

void
ScionAs::SynchronizeBaseAsNumber()
{
    NS_LOG_FUNCTION(this);
    // --- TODO: Implement this based on BaseAs interface ---
    // Example: If BaseAs has a SetAsNumber(uint64_t) method:
    // As as_part = m_ia.GetAs();
    // this->SetAsNumber(as_part.GetValue());
    //
    // Or if BaseAs stores its number in a protected member m_asNumber:
    // As as_part = m_ia.GetAs();
    // this->m_asNumber = as_part.GetValue(); // Adjust type if needed

    // For now, just log a reminder if BaseAs integration is needed
    NS_LOG_INFO("SynchronizeBaseAsNumber called for IA "
                << m_ia << ". Implement BaseAs update if required.");
}

} // namespace ns3