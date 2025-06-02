/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * Author: Faker Moatamri <faker.moatamri@sophia.inria.fr>
 *
 */
/**
 * This is the test code for ipv4-l3-protocol.cc
 */


#include "ns3/inet-socket-address.h"

#include "ns3/log.h"
#include "ns3/ipv6-address.h"

#include "scion-address-test.h"

#include <compare>

using namespace ns3;


SCIONAddressTestCase::SCIONAddressTestCase()
    : TestCase("Verify SCION Address")
{
}

SCIONAddressTestCase::~SCIONAddressTestCase()
{
}

void
SCIONAddressTestCase::DoRun()
{
   
    {
        SCIONAddress a0{"19-ffaa:0:1067,127.0.0.1:8080"};
        {
            std::stringstream out;
            out << a0;

            std::string str;
            out >> str;

            NS_TEST_ASSERT_MSG_EQ(                
                Ipv4Address::IsMatchingType(a0.GetHostAddress()),
                true,
                "HostAddress parse failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
            NS_TEST_ASSERT_MSG_EQ(
                str,
                "19-ffaa:0:1067,127.0.0.1",
                "Address-To-String failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
        }
        {
            std::stringstream out;
            SCIONAddress a1;
            out << a0;
            out >> a1;

            bool resres{false};
            #if __cplusplus >= 202002L
            auto res = a0 <=> a1;
            resres = res == std::strong_ordering::equal;
            #else
            resres = a0 == a1;
            #endif
            NS_TEST_ASSERT_MSG_EQ(
                resres,
                true,
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");

            NS_TEST_ASSERT_MSG_EQ(
                a0.GetIA(),
                a1.GetIA(),
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
            NS_TEST_ASSERT_MSG_EQ(
                a0.GetAS(),
                a1.GetAS(),
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
            NS_TEST_ASSERT_MSG_EQ(
                a0.GetISD(),
                a1.GetISD(),
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
            NS_TEST_ASSERT_MSG_EQ(
                a0.GetHostAddress(),
                a1.GetHostAddress(),
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
        }

        NS_TEST_ASSERT_MSG_EQ(a0.GetIA().GetValue(),
                              5629130167029863,
                              "IA computation failed for: 19-ffaa:0:1067,127.0.0.1:8080");
        NS_TEST_ASSERT_MSG_EQ(a0.GetISD().GetValue(), 19, "ISD computation failed for 19-ffaa:0:1067");       
        NS_TEST_ASSERT_MSG_EQ(a0.GetHostAddress(),
                              Ipv4Address("127.0.0.1"),
                              "HostAddress computation failed for: 19-ffaa:0:1067,127.0.0.1:8080");
        NS_TEST_ASSERT_MSG_EQ(a0.GetAS().GetValue(),
                              281105609527399,
                              "AS computation failed for ffaa:0:1067! was: " << a0.GetAS());
    }


{
    SCIONAddress a0{"19-ffaa:0:1067,127.0.0.1:8080"};

    NS_TEST_ASSERT_MSG_EQ( Ipv4Address::IsMatchingType(a0.GetHostAddress()), true, "AddressType wrong");

       

    Buffer buf;
    [[maybe_unused]]auto iter = buf.Begin();
    // a0.Serialize(iter );

    uint8_t buffer[100];
    a0.Serialize( buffer );

    SCIONAddress a1;
    a1.Deserialize( buffer );

    bool resres = false;
    #if __cplusplus >= 202002L
    auto res = a0 <=> a1;
    resres = res == std::strong_ordering::equal;
    #else
    resres = a0 == a1;
    #endif
            NS_TEST_ASSERT_MSG_EQ(
                resres,
                true,
                "Address Serialization/Deserialization failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
               
  NS_TEST_ASSERT_MSG_EQ(
                a0.GetIA(),
                a1.GetIA(),
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
            NS_TEST_ASSERT_MSG_EQ(
                a0.GetAS(),
                a1.GetAS(),
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
            NS_TEST_ASSERT_MSG_EQ(
                a0.GetISD(),
                a1.GetISD(),
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
            NS_TEST_ASSERT_MSG_EQ(
                a0.GetHostAddress(),
                a1.GetHostAddress(),
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");
                 
{     
    
    SCIONAddress a0{"19-ffaa:0:1067,[2001:db8::2:1]:8080"};
 

    NS_TEST_ASSERT_MSG_EQ( Ipv6Address::IsMatchingType(a0.GetHostAddress()), true , "AddressType wrong");
    Buffer buf;
    [[maybe_unused]]auto iter = buf.Begin();
    // a0.Serialize(iter );

    uint8_t buffer[100];
    a0.Serialize( buffer );

    SCIONAddress a1;
    a1.Deserialize( buffer );

          
            NS_TEST_ASSERT_MSG_EQ(
                (a0 == a1),
                true,
                "Address To- and back From- stream failed for: 19-ffaa:0:1067,192.127.0.0.1:8080");}
}

    
}

