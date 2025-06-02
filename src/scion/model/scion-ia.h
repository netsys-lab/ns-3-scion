#ifndef SCION_MODEL_IA_H
#define SCION_MODEL_IA_H

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/scion-types.h"

#include <cstdint>
#include <istream>
#include <limits> // Required for std::numeric_limits
#include <ostream>
#include <string>

namespace ns3
{

// Forward declarations
class As;
class Ia;

/**
 * \brief SCION ISolation Domain (ISD) identifier.
 *
 * Represents a 16-bit ISD number.
 */
class Isd
{
  public:
    /** \brief Default constructor (initializes to 0). */
    Isd();

    /** \brief Constructor from numeric value. */
    explicit Isd(Isd_t isd);

    /** \brief Get the numeric value. */
    Isd_t GetValue() const;

    /** \brief Convert to string representation (decimal). */
    std::string ToString() const;

    /**
     * \brief Parse an ISD from a decimal string.
     * \param s The string to parse.
     * \param[out] isd The parsed Isd object if successful.
     * \return true on success, false on failure (logs error).
     */
    static bool FromString(const std::string& s, Isd& isd);

    /** \brief Equality comparison. */
    bool operator==(const Isd& other) const;

    /** \brief Inequality comparison. */
    bool operator!=(const Isd& other) const;

    /** \brief Less than comparison (for use in maps/sets). */
    bool operator<(const Isd& other) const;

  private:
    Isd_t m_isd; //!< The ISD value.
};

/** \brief Stream insertion operator for Isd. */
std::ostream& operator<<(std::ostream& os, const Isd& isd);
/** \brief Stream extraction operator for Isd. */
std::istream& operator>>(std::istream& is, Isd& isd);

/**
 * \brief SCION Autonomous System (AS) identifier.
 *
 * Represents a 48-bit AS number. Can be parsed from:
 * - BGP-style decimal (up to 32 bits).
 * - SCION native hex format (e.g., "ffaa:1:2", 3 parts, 16 bits each).
 */
class Asn
{
  public:
    static constexpr uint32_t kAsBits = 48;
    static constexpr uint64_t kMaxAsVal = (1ULL << kAsBits) - 1;
    static constexpr uint32_t kBgpAsBits = 32;
    static constexpr uint64_t kMaxBgpAsVal = (1ULL << kBgpAsBits) - 1;

    /** \brief Default constructor (initializes to 0). */
    Asn();

    /**
     * \brief Constructor from numeric value.
     * \param as The AS value (must be within 48 bits).
     * \note Asserts if the value exceeds kMaxAsVal.
     */
    explicit Asn(Asn_t as);

    /** \brief Get the numeric value. */
    Asn_t GetValue() const;

    /** \brief Convert to string representation (SCION hex format). */
    std::string ToString() const;

    /**
     * \brief Parse an AS from a string.
     *
     * Handles both BGP decimal format (e.g., "65001") and
     * SCION hex format (e.g., "ffaa:1:2").
     *
     * \param s The string to parse.
     * \param[out] asValue The parsed As object if successful.
     * \return true on success, false on failure (logs error).
     */
    static bool FromString(const std::string& s, Asn& asValue);

    /** \brief Equality comparison. */
    bool operator==(const Asn& other) const;

    /** \brief Inequality comparison. */
    bool operator!=(const Asn& other) const;

    /** \brief Less than comparison (for use in maps/sets). */
    bool operator<(const Asn& other) const;

  private:
    static constexpr int kAsPartBits = 16;
    static constexpr int kAsParts = kAsBits / kAsPartBits; // Should be 3
    static constexpr char kSeparator = ':';

    /** \brief Helper to parse BGP AS number. */
    static bool ParseBgpAs(const std::string& s, uint64_t& value);
    /** \brief Helper to parse SCION native AS number. */
    static bool ParseScionAs(const std::string& s, uint64_t& value);

    Asn_t m_as; //!< The AS value (lowest 48 bits are used).
};

/** \brief Stream insertion operator for As. */
std::ostream& operator<<(std::ostream& os, const Asn& as);
/** \brief Stream extraction operator for As. */
std::istream& operator>>(std::istream& is, Asn& as);

/**
 * \brief SCION ISD-AS identifier.
 *
 * Combines a 16-bit ISD and a 48-bit AS into a single 64-bit value.
 */
class Ia
{
  public:
    static constexpr uint32_t kIsdBits = 16;
    static constexpr uint32_t kAsBits = Asn::kAsBits; // 48
    static constexpr uint64_t kAsMask = Asn::kMaxAsVal;

    /** \brief Default constructor (initializes to 0). */
    Ia();

    /**
     * \brief Constructor from Isd and As objects.
     * \param isd The ISD component.
     * \param as The AS component.
     */
    Ia(Isd isd, Asn as);

    explicit Ia(Ia_t ia): m_ia(ia){}

    /** \brief Get the ISD component. */
    Isd GetIsd() const;

    /** \brief Get the AS component. */
    Asn GetAsn() const;

    /** \brief Get the raw 64-bit combined value. */
    Ia_t GetValue() const;

    /** \brief Check if the IA is zero (0-0). */
    bool IsZero() const;

    /** \brief Check if either ISD or AS part is zero (wildcard). */
    bool IsWildcard() const;

    /** \brief Convert to string representation (e.g., "1-ffaa:1:2"). */
    std::string ToString() const;

    /**
     * \brief Parse an IA from a string "isd-as".
     * \param s The string to parse.
     * \param[out] ia The parsed Ia object if successful.
     * \return true on success, false on failure (logs error).
     */
    static bool FromString(const std::string& s, Ia& ia);

    /** \brief Equality comparison. */
    bool operator==(const Ia& other) const;

    /** \brief Inequality comparison. */
    bool operator!=(const Ia& other) const;

    /** \brief Less than comparison (for use in maps/sets). */
    bool operator<(const Ia& other) const;

  private:
    static constexpr char kSeparator = '-';
    Ia_t m_ia; //!< Combined ISD (high 16 bits) and AS (low 48 bits).
};

/** \brief Stream insertion operator for Ia. */
std::ostream& operator<<(std::ostream& os, const Ia& ia);
/** \brief Stream extraction operator for Ia. */
std::istream& operator>>(std::istream& is, Ia& ia);

} // namespace ns3

#endif // SCION_MODEL_IA_H