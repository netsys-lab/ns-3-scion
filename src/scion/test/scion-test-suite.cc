#include "ns3/test.h"
#include "scion-address-test.h"
#include "scion-header-test.h"
namespace ns3
{
/**
 * \ingroup scion-test
 *
 * \brief SCION TestSuite
 */
class SCIONTestSuite : public TestSuite
{
  public:
    SCIONTestSuite()
        : TestSuite("SCION", UNIT)
    {
        AddTestCase(new SCIONAddressTestCase(), TestCase::QUICK);
           AddTestCase(new SCIONHeaderTestCase(), TestCase::QUICK);
    }
};

static SCIONTestSuite g_TestSuite; //!< Static variable for test initialization
}