#include "scion-header-test.h"
#include "ns3/udp-header.h"
#include "ns3/packet.h"
#include "ns3/scion-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/test.h"

using namespace ns3;


SCIONHeaderTestCase::SCIONHeaderTestCase()
    : TestCase("Verify SCION Header")
{
}

SCIONHeaderTestCase::~SCIONHeaderTestCase()
{
}


void SCIONHeaderTestCase::CheckSCIONHeaderEqual( const SCIONHeader& scionLayer, const SCIONHeader& scion )
{
NS_TEST_ASSERT_MSG_EQ(scionLayer.GetDstIA(), scion.GetDstIA(), "d");
    NS_TEST_ASSERT_MSG_EQ(scionLayer.GetSrcIA(), scion.GetSrcIA(), "d");


    NS_TEST_ASSERT_MSG_EQ(scionLayer.GetFlowID(),
                          scion.GetFlowID(),
                          "Decoding of SCIONHeader failed (field: FlowID)");

    NS_TEST_ASSERT_MSG_EQ(scionLayer.GetTrafficClass(),
                          scion.GetTrafficClass(),
                          "Decoding of SCIONHeader failed (field: TrafficClass)");
    NS_TEST_ASSERT_MSG_EQ(scionLayer.GetVersion(),
                          scion.GetVersion(),
                          "Decoding of SCIONHeader failed (field: Version)");
    NS_TEST_ASSERT_MSG_EQ(scionLayer.DstAddress(),
                          scion.DstAddress(),
                          "Decoding of SCIONHeader failed (field: DstAddress)");
    NS_TEST_ASSERT_MSG_EQ(scionLayer.SrcAddress(),
                          scion.SrcAddress(),
                          "Decoding of SCIONHeader failed (field: SrcAddress)");
    NS_TEST_ASSERT_MSG_EQ(scionLayer.NextHeader(),
                          scion.NextHeader(),
                          "Decoding of SCIONHeader failed (field: NextHeader)");
    NS_TEST_ASSERT_MSG_EQ(scionLayer.GetPayloadLength(),
                          scion.GetPayloadLength(),
                          "Decoding of SCIONHeader failed (field: PayloadLen)");
    NS_ASSERT_MSG(scionLayer.GetDstAddrType() == scion.GetDstAddrType(), "Destination AddressType wrong");
    NS_ASSERT_MSG(scionLayer.GetSrcAddrType() == scion.GetSrcAddrType(), "Source AddressType wrong");



    NS_TEST_ASSERT_MSG_EQ(scionLayer.SCIONSrcAddress(),
                          scion.SCIONSrcAddress(),
                          "Decoding of SCIONHeader failed (field: SCIONSrcAddress)");
    NS_TEST_ASSERT_MSG_EQ(scionLayer.SCIONDstAddress(),
                          scion.SCIONDstAddress(),
                          "Decoding of SCIONHeader failed (field: SCIONDstAddress)");
}


void SCIONHeaderTestCase::Test00()
{

    using namespace std::literals;
    auto payload = "<payload>I am a small string payload!</payload>"sv;

    auto pkt = Create<Packet>((uint8_t*)payload.data(), payload.size());

        
    UdpHeader udp;
    udp.SetSourcePort( 3041);
    udp.SetDestinationPort(53);

    pkt->AddHeader(udp);   
  
    SCIONAddress a0{"19-ffaa:0:1067,127.0.0.1"};    
    SCIONAddress a1{"17-ffaa:1:fe4,[2001:db8:85a3:8d3:1319:8a2e:370:7348]"};  

    SCIONHeader scion;
    //scion.SetPath( rawTestPath);
    scion.SetDstAddress(a1);
    scion.SetSrcAddress(a0);

    scion.SetNextHeader(L4ProtocolType_t::L4UDP );
    scion.SetPayloadLength( payload.size()+udp.GetSerializedSize() );


    Ipv4Header ipv4;
    ipv4.SetSource(Ipv4Address("127.0.0.1"));
    ipv4.SetDestination(Ipv4Address("192.168.2.1")); // underlay next hop

    pkt->AddHeader(scion);
    pkt->AddHeader(ipv4);

    pkt->RemoveHeader(ipv4);

    SCIONHeader scion2;
    pkt->RemoveHeader(scion2);
    //NS_TEST_ASSERT_MSG_EQ(scion2.)

    CheckSCIONHeaderEqual(scion2,scion);


}

void
SCIONHeaderTestCase::DoRun()
{
    Test00();
}