
#include "ns3/scion-header.h"
#include "ns3/test.h"

namespace ns3{
/**
 * \ingroup scion-test
 *
 * \brief scion address Test
 */
class SCIONHeaderTestCase : public TestCase
{
    void Test00();
    void CheckSCIONHeaderEqual( const SCIONHeader& scionLayer, const SCIONHeader& scion );
  public:
    SCIONHeaderTestCase();
    ~SCIONHeaderTestCase() override;
    void DoRun() override;
};
}