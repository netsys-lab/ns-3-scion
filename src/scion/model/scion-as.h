
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
#include "ns3/ia.h"
#include "ns3/object.h"
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
    void SetIa(const Ia& ia);
    void SetBeaconPolicy(uint32_t policy);

    // Getters
    const Ia& GetIa() const;
    Isd GetIsd() const;
    Asn GetAs() const;

    uint32_t GetBeaconPolicy() const;

  private:
    Ia m_ia;                 // The full ISD-AS identifier.
    uint32_t m_beaconPolicy; // Beacon policy

    /**
     * \brief Attribute system setter for the IA.
     * \param ia The IA value received from the attribute system.
     */
    void SetIaAttribute(Ia ia); // Note: Takes Ia by value is common for attr setters

    /**
     * \brief Attribute system getter for the IA.
     * \return The current IA value.
     */
    Ia GetIaAttribute() const;

    /**
     * \brief Updates the AS number potentially stored in the BaseAs class.
     *
     * Called internally when m_ia is changed. The implementation depends
     * on how BaseAs manages its AS number.
     */
    void SynchronizeBaseAsNumber();
};

// --- AttributeValue Implementation for Ia ---

/**
 * \brief Hold an Ia value in an Attribute.
 */
class IaValue : public AttributeValue
{
  public:
    IaValue();
    IaValue(const Ia& value);

    /**
     * \brief Get the IA value.
     * \return the IA value.
     */
    Ia Get() const;

    /**
     * \brief Set the IA value.
     * \param value The IA value.
     */
    void Set(const Ia& value);

    // Implementation of AttributeValue interface
    Ptr<AttributeValue> Copy() const override;
    std::string SerializeToString(Ptr<const AttributeChecker> checker) const override;
    bool DeserializeFromString(std::string value, Ptr<const AttributeChecker> checker) override;

  private:
    Ia m_value; //!< Stored IA value.
};

/** \brief Stream insertion operator for IaValue. */
std::ostream& operator<<(std::ostream& os, const IaValue& value);
/** \brief Stream extraction operator for IaValue. */
std::istream& operator>>(std::istream& is, IaValue& value);

// Optional: Define Checker and Accessor if needed for more complex scenarios,
// but StringAccessor often suffices if parsing from string is the primary goal.
// MakeIaValue is useful for direct setting.

/**
 * \brief Make an AttributeValue for an Ia.
 * \param ia The Ia object.
 * \return A Ptr to an AttributeValue.
 */
Ptr<AttributeValue> MakeIaValue(const Ia& ia);

/**
 * \brief Make an AttributeChecker for an Ia (using StringChecker).
 * \return A Ptr to an AttributeChecker.
 *
 * This provides basic checking via string serialization/deserialization.
 * More specific checks could be implemented if needed.
 */
Ptr<const AttributeChecker> MakeIaChecker();

} // namespace ns3

#endif // SCION_MODEL_SCION_AS_H