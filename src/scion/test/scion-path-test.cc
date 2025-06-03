#include "scion-path-test.h"

#include "ns3/scion-path.h"
#include "ns3/buffer.h"
#include "ns3/test.h"
#include "ns3/log.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE("SCIONPathTest");


SCIONPathTestCase::SCIONPathTestCase()
    : TestCase("Verify SCION dataplane path")   
{
    
#ifndef SCION_SIMPLIFIED_SIMULATION
std::vector<InfoField> testInfoFields{InfoField{
                                              .Peer = false,
                                              .ConsDir = false,
                                              .SegID = 0x111,
                                              .Timestamp = 0x100,
                                          },
                                          InfoField{
                                              .Peer = false,
                                              .ConsDir = true,
                                              .SegID = 0x222,
                                              .Timestamp = 0x100,
                                          }};
 std::vector<HopField> testHopFields{HopField{
                                            .ExpTime = 63,
                                            .ConsIngress = 1,
                                            .ConsEgress = 0,
                                            .Mac = {1, 2, 3, 4, 5, 6},
                                        },
                                        HopField{
                                            .ExpTime = 63,
                                            .ConsIngress = 3,
                                            .ConsEgress = 2,
                                            .Mac = {1, 2, 3, 4, 5, 6},
                                        },
                                        HopField{
                                            .ExpTime = 63,
                                            .ConsIngress = 0,
                                            .ConsEgress = 2,
                                            .Mac = {1, 2, 3, 4, 5, 6},
                                        },
                                        HopField{
                                            .ExpTime = 63,
                                            .ConsIngress = 1,
                                            .ConsEgress = 0,
                                            .Mac = {1, 2, 3, 4, 5, 6},
                                        }};
#else
std::vector<InfoField> testInfoFields{InfoField{
                                              .Peer = false,
                                              .ConsDir = false                                           
                                          },
                                          InfoField{
                                              .Peer = false,
                                              .ConsDir = true
                                             
                                          }};
 std::vector<HopField> testHopFields{HopField{
                                         
                                            .ConsIngress = 1,
                                            .ConsEgress = 0,
                                       
                                        },
                                        HopField{
                                        
                                            .ConsIngress = 3,
                                            .ConsEgress = 2,
                                          
                                        },
                                        HopField{
                                      
                                            .ConsIngress = 0,
                                            .ConsEgress = 2,
                                        
                                        },
                                        HopField{
                                        
                                            .ConsIngress = 1,
                                            .ConsEgress = 0,
                                          
                                        }};
#endif
 decoded_path = SCIONPath(testInfoFields,testHopFields, {2,2,0});
}

SCIONPathTestCase::~SCIONPathTestCase()
{
}

void SCIONPathTestCase::CheckPathEqual(const BasePath& a, const BasePath& b)
{
    NS_TEST_ASSERT_MSG_EQ(a.GetCurrHF(), b.GetCurrHF(), "CurrHF");
    NS_TEST_ASSERT_MSG_EQ(a.GetCurrINF(), b.GetCurrINF(),"CurrINF");
    NS_TEST_ASSERT_MSG_EQ(a.GetNumHF(), b.GetNumHF(), "");
    NS_TEST_ASSERT_MSG_EQ(a.GetNumINF(), b.GetNumINF(), "");

    for(uint8_t i{0}; i< a.GetNumHF(); ++i)
    {
        NS_TEST_ASSERT_MSG_EQ(a.GetHopField(i),b.GetHopField(i), "");
    }

    for(uint8_t i{0}; i< a.GetNumINF(); ++i)
    {
        NS_TEST_ASSERT_MSG_EQ(a.GetInfoField(i),b.GetInfoField(i), "");
    }
}
/* serialize and deserialize a default constructed 'empty' SCIONPath
 */
void SCIONPathTestCase::Test00()
{


    SCIONPath pp;
    pp.SetCurrHF(0);
    pp.SetCurrINF(0);
    auto pathlen{pp.Len() };
    NS_LOG_DEBUG("pathlen: " << pathlen);

    Buffer buffer1{};
    buffer1.AddAtStart(pathlen);
    pp.Serialize( buffer1.Begin() );

    SCIONPath pp2;
    pp2.Deserialize(buffer1.Begin());
    CheckPathEqual(pp, pp2);    

}

/* serialize and deserialize a  SCIONPath
 */
void SCIONPathTestCase::Test01()
{
    auto pathlen{decoded_path.Len() };
    NS_LOG_DEBUG("pathlen: " << pathlen);

    Buffer buffer1{};
    buffer1.AddAtStart(pathlen);
    decoded_path.Serialize( buffer1.Begin() );

    SCIONPath pp2;
    pp2.Deserialize(buffer1.Begin());
    CheckPathEqual(decoded_path, pp2);    

}

void SCIONPathTestCase::Test02()
{
    {
    HopField hf{.ConsIngress=2, .ConsEgress=1 };

    Buffer buff{};

    buff.AddAtStart(HopField::Len());
    hf.Serialize(buff.Begin());

    HopField hf2;

    hf2.Deserialize(buff.Begin());

    NS_TEST_ASSERT_MSG_EQ(hf2, hf, "");
    }
    {
        InfoField inf{.Peer=false, .ConsDir=true};
        Buffer buff{};
        buff.AddAtStart(InfoField::Len());
        inf.Serialize(buff.Begin());
        InfoField inf2;
        inf2.Deserialize(buff.Begin());

        NS_TEST_ASSERT_MSG_EQ(inf,inf2, "");
    }
}

void
SCIONPathTestCase::DoRun()
{
    
    Test00();
    Test01();
    Test02();
}
}