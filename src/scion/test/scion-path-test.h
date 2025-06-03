
#include "ns3/scion-header.h"
#include "ns3/test.h"

namespace ns3{
/**
 * \ingroup scion-test
 *
 * \brief scion dataplane path Test
 */
class SCIONPathTestCase : public TestCase
{
    SCIONPath decoded_path;

    void Test00();
    void Test01();
    void Test02();
    void CheckPathEqual(const BasePath& a, const BasePath& b);
public:
    SCIONPathTestCase();
    ~SCIONPathTestCase() override;
    void DoRun() override;
};
}