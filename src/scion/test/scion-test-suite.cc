#include "ns3/test.h"
#include "scion-address-test.h"
#include "scion-node-test.h"
#include "scion-header-test.h"
#include "scion-path-test.h"
#include "scmp/scion-scmp-test.h"
#include "scion-socket-address-test.h"
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
        AddTestCase(new SCIONPathTestCase(), TestCase::QUICK);
        AddTestCase(new SCIONSCMPTestCase(), TestCase::QUICK);
        AddTestCase(new SCIONSocketAddressTestCase(), TestCase::QUICK);
        //AddTestCase(new SCIONNodeTestCase(), TestCase::QUICK);
    }
};

static SCIONTestSuite g_TestSuite; //!< Static variable for test initialization
}