#include "ns3/scion-ia.h"
#include "ns3/object.h"
#include "ns3/abort.h" // For NS_ASSERT_MSG
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/attribute-accessor-helper.h"
#include <charconv> // C++17 for efficient parsing
#include <iomanip>
#include <sstream>
#include <system_error> // For std::from_chars error handling
#include <vector>

namespace ns3
{

// Define the log component (choose an appropriate name)
NS_LOG_COMPONENT_DEFINE("IA");

// --- Isd Implementation ---

Isd::Isd()
    : m_isd(0)
{
}

Isd::Isd(Isd_t isd)
    : m_isd(isd)
{
}

uint16_t
Isd::GetValue() const
{
    return m_isd;
}

std::string
Isd::ToString() const
{
    return std::to_string(m_isd);
}

bool
Isd::FromString(const std::string& s, Isd& isd)
{
    uint64_t val; // Use larger type for overflow check
    try
    {
        // Use std::stoul for simplicity, could use std::from_chars for performance
        size_t pos;
        val = std::stoul(s, &pos, 10);
        // Check if the entire string was consumed and if value is in range
        if (pos != s.length() || val > std::numeric_limits<uint16_t>::max())
        {
            NS_LOG_WARN("Invalid ISD string format or value out of range: '" << s << "'");
            return false;
        }
    }
    catch (const std::invalid_argument& e)
    {
        NS_LOG_WARN("Invalid ISD string format (not a number): '" << s << "'");
        return false;
    }
    catch (const std::out_of_range& e)
    {
        NS_LOG_WARN("ISD string value out of range: '" << s << "'");
        return false;
    }

    isd.m_isd = static_cast<uint16_t>(val);
    return true;
}

bool
Isd::operator==(const Isd& other) const
{
    return m_isd == other.m_isd;
}

bool
Isd::operator!=(const Isd& other) const
{
    return !(*this == other);
}

bool
Isd::operator<(const Isd& other) const
{
    return m_isd < other.m_isd;
}

std::ostream&
operator<<(std::ostream& os, const Isd& isd)
{
    os << isd.ToString();
    return os;
}

std::istream&
operator>>(std::istream& is, Isd& isd)
{
    std::string s;
    is >> s;
    if (!Isd::FromString(s, isd))
    {
        // Set failbit if parsing failed
        is.setstate(std::ios_base::failbit);
    }
    return is;
}

// --- As Implementation ---

Asn::Asn()
    : m_as(0)
{
}

Asn::Asn(Asn_t as)
    : m_as(as)
{
    NS_ASSERT_MSG(as <= kMaxAsVal, "AS value " << as << " exceeds maximum " << kMaxAsVal);
    // Keep only the lower 48 bits, although constructor ensures this via assert
    m_as &= kMaxAsVal;
}

Asn_t
Asn::GetValue() const
{
    return m_as;
}

// Helper to split string by delimiter
static std::vector<std::string>
split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

bool
Asn::ParseBgpAs(const std::string& s, uint64_t& value)
{
    uint64_t val;
    try
    {
        size_t pos;
        val = std::stoull(s, &pos, 10);
        if (pos != s.length() || val > kMaxBgpAsVal)
        {
            NS_LOG_WARN("Invalid BGP AS string format or value out of 32-bit range: '" << s << "'");
            return false;
        }
    }
    catch (const std::invalid_argument& e)
    {
        NS_LOG_WARN("Invalid BGP AS string format (not a number): '" << s << "'");
        return false;
    }
    catch (const std::out_of_range& e)
    {
        NS_LOG_WARN("BGP AS string value out of range: '" << s << "'");
        return false;
    }
    value = val;
    return true;
}

bool
Asn::ParseScionAs(const std::string& s, uint64_t& value)
{
    std::vector<std::string> parts = split(s, kSeparator);
    if (parts.size() != kAsParts)
    {
        NS_LOG_WARN("Invalid SCION AS string format: wrong number of parts in '"
                    << s << "' (expected " << kAsParts << ")");
        return false;
    }

    uint64_t parsed_as = 0;
    for (const auto& part : parts)
    {
        parsed_as <<= kAsPartBits;
        uint64_t part_val;
        try
        {
            size_t pos;
            part_val = std::stoull(part, &pos, 16); // Parse hex
            if (pos != part.length() || part_val >= (1ULL << kAsPartBits))
            {
                NS_LOG_WARN("Invalid SCION AS part format or value out of 16-bit range: '"
                            << part << "' in '" << s << "'");
                return false;
            }
        }
        catch (const std::invalid_argument& e)
        {
            NS_LOG_WARN("Invalid SCION AS part format (not hex): '" << part << "' in '" << s
                                                                    << "'");
            return false;
        }
        catch (const std::out_of_range& e)
        {
            NS_LOG_WARN("SCION AS part value out of range: '" << part << "' in '" << s << "'");
            return false;
        }
        parsed_as |= part_val;
    }

    // Double check range, though individual parts limit should ensure this
    if (parsed_as > kMaxAsVal)
    {
        NS_LOG_WARN("Parsed SCION AS value out of 48-bit range: " << parsed_as << " from '" << s
                                                                  << "'");
        return false; // Should be unreachable if part parsing is correct
    }

    value = parsed_as;
    return true;
}

bool
Asn::FromString(const std::string& s, Asn& asValue)
{
    uint64_t val = 0;
    // If ':' is present, assume SCION format, otherwise BGP
    if (s.find(kSeparator) != std::string::npos)
    {
        if (!ParseScionAs(s, val))
        {
            return false;
        }
    }
    else
    {
        if (!ParseBgpAs(s, val))
        {
            return false;
        }
    }
    asValue.m_as = val;
    return true;
}

std::string
Asn::ToString() const
{
    std::stringstream ss;
    ss << std::hex << ((m_as >> (kAsPartBits * 2)) & ((1ULL << kAsPartBits) - 1)) << kSeparator
       << ((m_as >> kAsPartBits) & ((1ULL << kAsPartBits) - 1)) << kSeparator
       << (m_as & ((1ULL << kAsPartBits) - 1));
    return ss.str();
}

bool
Asn::operator==(const Asn& other) const
{
    return m_as == other.m_as;
}

bool
Asn::operator!=(const Asn& other) const
{
    return !(*this == other);
}

bool
Asn::operator<(const Asn& other) const
{
    return m_as < other.m_as;
}

std::ostream&
operator<<(std::ostream& os, const Asn& as)
{
    os << as.ToString();
    return os;
}

std::istream&
operator>>(std::istream& is, Asn& as)
{
    std::string s;
    is >> s;
    if (!Asn::FromString(s, as))
    {
        // Set failbit if parsing failed
        is.setstate(std::ios_base::failbit);
    }
    return is;
}

// --- Ia Implementation ---

Ia::Ia()
    : m_ia(0)
{
}

Ia::Ia(Isd isd, Asn as)
    : m_ia((static_cast<uint64_t>(isd.GetValue()) << kAsBits) | (as.GetValue() & kAsMask))
{
    // AS value range is already checked in As constructor/parsing
}

Isd
Ia::GetIsd() const
{
    return Isd(static_cast<uint16_t>(m_ia >> kAsBits));
}

Asn
Ia::GetAsn() const
{
    // Use As constructor which has range assertion
    return Asn(m_ia & kAsMask);
}

Ia_t
Ia::GetValue() const
{
    return m_ia;
}

bool
Ia::IsZero() const
{
    return m_ia == 0;
}

bool
Ia::IsWildcard() const
{
    // Check if ISD is 0 or AS is 0
    return (m_ia >> kAsBits == 0) || (m_ia & kAsMask) == 0;
}

std::string
Ia::ToString() const
{
    std::stringstream ss;
    ss << GetIsd().ToString() << kSeparator << GetAsn().ToString();
    return ss.str();
}

bool
Ia::FromString(const std::string& s, Ia& ia)
{
    size_t sep_pos = s.find(kSeparator);
    if (sep_pos == std::string::npos || sep_pos == 0 || sep_pos == s.length() - 1)
    {
        NS_LOG_WARN("Invalid IA string format: missing or misplaced separator '-' in '" << s
                                                                                        << "'");
        return false;
    }

    std::string isd_str = s.substr(0, sep_pos);
    std::string as_str = s.substr(sep_pos + 1);

    Isd parsed_isd;
    if (!Isd::FromString(isd_str, parsed_isd))
    {
        // Error already logged by Isd::FromString
        return false;
    }

    Asn parsed_as;
    if (!Asn::FromString(as_str, parsed_as))
    {
        // Error already logged by As::FromString
        return false;
    }

    ia = Ia(parsed_isd, parsed_as); // Use constructor
    return true;
}

bool
Ia::operator==(const Ia& other) const
{
    return m_ia == other.m_ia;
}

bool
Ia::operator!=(const Ia& other) const
{
    return !(*this == other);
}

bool
Ia::operator<(const Ia& other) const
{
    return m_ia < other.m_ia;
}

std::ostream&
operator<<(std::ostream& os, const Ia& ia)
{
    os << ia.ToString();
    return os;
}

std::istream&
operator>>(std::istream& is, Ia& ia)
{
    std::string s;
    is >> s;
    if (!Ia::FromString(s, ia))
    {
        // Set failbit if parsing failed
        is.setstate(std::ios_base::failbit);
    }
    return is;
}

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


ATTRIBUTE_CHECKER_IMPLEMENT(Ia);
 // ATTRIBUTE_VALUE_IMPLEMENT(Ia); // TODO: this would save the boilerplate code

} // namespace ns3