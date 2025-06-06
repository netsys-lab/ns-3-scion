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
 * Author:
 *
 */


#include "ns3/scion-socket-address.h"

#include "ns3/log.h"
#include "ns3/scion-address.h"

#include "scion-socket-address-test.h"

#include <compare>

using namespace ns3;


SCIONSocketAddressTestCase::SCIONSocketAddressTestCase()
    : TestCase("Verify SCION Socket Address")
{
}


void
SCIONSocketAddressTestCase::DoRun()
{
   
        SCIONAddress a0{"19-ffaa:0:1067,127.0.0.1:8080"};

        SCIONSocketAddress a1{a0, 53};
        auto addr{a1.ConvertTo()};
        auto a2{SCIONSocketAddress::ConvertFrom(addr)};
        {
          
            NS_TEST_ASSERT_MSG_EQ(                
                SCIONSocketAddress::IsMatchingType(a2),
                true,
                "");
            NS_TEST_ASSERT_MSG_EQ(
                a1,
                a2,
                "Address From/To Conversion failed");
        }
       
    
}

