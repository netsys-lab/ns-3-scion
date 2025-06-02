#include "ns3/scion-address.h"

#include "ns3/address-utils.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/scion-address.h"
#include "ns3/scion-ia.h"
#include "ns3/tag-buffer.h"

#include <arpa/inet.h>
#include <assert.h>
#include <iomanip>
#include <regex>
#include <sstream>

namespace ns3
{

    Isd SCIONAddress::GetISD() const
    {
        return _ia.GetIsd();
    }

    Asn SCIONAddress::GetAS() const
    {
        return _ia.GetAsn();
    }
/*
Address
SetPort(const Address& address, uint16_t port)
{
    if (Ipv4Address::IsMatchingType(address))
    {
        InetSocketAddress a{Ipv4Address::ConvertFrom(address)};
        a.SetPort(port);
        return a;
    }
    else if (Ipv6Address::IsMatchingType(address))
    {
        Inet6SocketAddress a{Ipv6Address::ConvertFrom(address)};
        a.SetPort(port);
        return a;
    }
    else
    {
        NS_ASSERT(false);
    }
}
    */

AddrType_t
hostAddrToAddrType(const Address& host)
{
    if (Ipv4Address::IsMatchingType(host))
    {
        return AddrType_t::T4Ip;
    }
    else if (Ipv6Address::IsMatchingType(host))
    {
        return AddrType_t::T16Ip;
    }
    else
    {
        assert(false);
    }
    // TODO service addresses ...
}



/**
 */
SCIONAddress::SCIONAddress(Isd isd, Asn as, const Address& host)
    : _ia(isd, as)
{
    if (Ipv4Address::IsMatchingType(host))
    {
        _hostAddr = host;
    }
    else if (Ipv6Address::IsMatchingType(host))
    {
        _hostAddr = host;
    }
    else if (InetSocketAddress::IsMatchingType(host))
    {
        InetSocketAddress a{InetSocketAddress::ConvertFrom(host)};        
        _hostAddr = a.GetIpv4();
    }
    else if (Inet6SocketAddress::IsMatchingType(host))
    {
        Inet6SocketAddress a{Inet6SocketAddress::ConvertFrom(host)};        
        _hostAddr = a.GetIpv6();
    }
    else
    {
        NS_ASSERT(false);
    }

    addrType = hostAddrToAddrType(_hostAddr);
}

SCIONAddress&
SCIONAddress::Deserialize(const uint8_t* buffer)
{
    [[maybe_unused]] auto _type = buffer[0];
    [[maybe_unused]] auto _len = buffer[1];

    //  memcpy( &_ia,  buffer+2,  8 ); // THIS IS NOT IN NETWORK BYTE ORDER
    _ia = Ia(*(Ia_t*)(buffer + 2));
   

    addrType = (AddrType_t)buffer[10];
    // buffer[10] = (uint8_t(addrType & 0xF) << 4);
    //  | uint8_t( srcAddrType & 0xF)  ); could fit a second type in 8bit

    // _hostAddr.CopyFrom(buffer + 11, AddrTypeLength(addrType) +2  ); // oder len -11
    // THIS IS NOT IN NETWORK BYTE ORDER?!

    [[maybe_unused]] auto bytes = _hostAddr.CopyAllFrom(buffer + 11, 100);
    return *this;
}

void
SCIONAddress::Serialize(uint8_t* buffer) const
{
    buffer[0] = GetType();
    buffer[1] = GetLength();
    memcpy(buffer + 2, &_ia, 8);    // THIS IS NOT IN NETWORK BYTE ORDER
    buffer[10] = addrType;

    // (uint8_t(addrType & 0xF) << 4);
    // | uint8_t( srcAddrType & 0xF)  ); could fit a second type in 8bit

    [[maybe_unused]] auto bytes =
        _hostAddr.CopyAllTo(buffer + 11, 100); // THIS IS NOT IN NETWORK BYTE ORDER?!
}

/**
 * Serialize this address to a byte buffer
 *
 * \param buf output buffer to which this address gets overwritten
 * with this SCIONAddress  (IA, hostAddressType, followed by hostAddr)
 */
void
SCIONAddress::Serialize(TagBuffer start) const
{
    start.WriteU8(GetType());
    start.WriteU8(GetLength());
    start.WriteU64(_ia.GetValue());
    start.WriteU8(addrType);
    //(uint8_t(addrType & 0xF) << 4));
    // | uint8_t( srcAddrType & 0xF)  ); could fit a second type in 8bit

    uint8_t* buf = (uint8_t*)malloc(_hostAddr.GetSerializedSize());
    _hostAddr.CopyTo(buf);
    start.Write(buf, _hostAddr.GetSerializedSize());
    free(buf);
    // WriteTo(start, hostAddr);
}

void
SCIONAddress::Serialize(Buffer::Iterator start) const
{
    start.WriteU8(GetType());
    start.WriteU8(GetLength());
    start.WriteHtonU64(_ia.GetValue());  
    start.WriteU8(addrType);
    //(uint8_t(addrType & 0xF) << 4));
    // | uint8_t( srcAddrType & 0xF)  ); could fit a second type in 8bit

    /* uint8_t * buf = (uint8_t*) malloc( _hostAddr.GetSerializedSize() );
     _hostAddr.CopyTo(buf );
     start.Write( buf, _hostAddr.GetSerializedSize() );
     free( buf);
     */
    WriteTo(start, _hostAddr);
}

/**
 * \param address a polymorphic address
 * \return a new SCIONAddress from the polymorphic address
 *
 * This function performs a type check and asserts if the
 * type of the input address is not compatible with a SCIONAddress
 */
SCIONAddress
SCIONAddress::ConvertFrom(const Address& address)
{
    SCIONAddress tmp;
    if (address.IsMatchingType(GetType()))
    {
        uint8_t* buf = (uint8_t*)malloc(address.GetSerializedSize());
        address.CopyAllTo(buf, address.GetSerializedSize());
        tmp.Deserialize(buf);
        free(buf);
    }
    return tmp;
}

const Ia& SCIONAddress::GetIA()const
{
    return _ia; 
}

 #if __cplusplus >= 202002L
  std::strong_ordering SCIONAddress::operator <=>( const SCIONAddress &other ) const
  {
    auto res = _ia <=> other.GetIA();
    auto hostEq = _hostAddr == other.GetHostAddress();
    auto hostLess = _hostAddr < other.GetHostAddress();
    return (res == std::strong_ordering::equal ) ?  ( hostEq ? (_port <=> other.GetPort() ) 
                                   : (hostLess ? std::strong_ordering::less
                                               : std::strong_ordering::greater ) )
                       : res  ;
  }
  #endif

/**
 * \param buf buffer to read address from
 * \return a SCIONAddress
 *
 * The input address is expected to be in network byte order format.
 */
SCIONAddress&
SCIONAddress::Deserialize(TagBuffer start)
{
    [[maybe_unused]] uint8_t type = start.ReadU8(); // the value returned by GetType()
    [[maybe_unused]] uint8_t len = start.ReadU8();  // return value of GetLength()
    _ia = Ia(start.ReadU64());   
    addrType = (AddrType_t)start.ReadU8();
    // auto byte9 = start.ReadU8();
    // addrType = AddrType_t(byte9 >> 4 & 0xF);
    //  srcAddrType = AddrType_t(byte9 & 0xF); // retrievest the second type

    auto addrBytes = AddrTypeLength(
        addrType); // this could be infered from 'len' by subtracting 8byte for the IA

    // RawDstAddr = Address(uint8_t type, const uint8_t* buffer, uint8_t len);
    // RawDstAddr = data[offset : offset+dstAddrBytes]
    uint8_t* tmp = (uint8_t*)malloc(addrBytes);
    start.Read(tmp, addrBytes);
    _hostAddr.CopyFrom(tmp, addrBytes);
    return *this;
}

SCIONAddress&
SCIONAddress::Deserialize(Buffer::Iterator start)
{
    [[maybe_unused]] uint8_t type = start.ReadU8(); // the value returned by GetType()
    [[maybe_unused]] uint8_t len = start.ReadU8();  // return value of GetLength()
    _ia = Ia(start.ReadU64());   
    auto byte9 = start.ReadU8();
    addrType = AddrType_t(byte9 >> 4 & 0xF);
    // srcAddrType = AddrType_t(byte9 & 0xF); // retrievest the second type

    auto addrBytes = AddrTypeLength(
        addrType); // this could be infered from 'len' by subtracting 8byte for the IA

    // RawDstAddr = Address(uint8_t type, const uint8_t* buffer, uint8_t len);
    // RawDstAddr = data[offset : offset+dstAddrBytes]
    uint8_t* tmp = (uint8_t*)malloc(addrBytes);
    start.Read(tmp, addrBytes);
    _hostAddr.CopyFrom(tmp, addrBytes);
    return *this;
}

namespace
{

bool
isValidIPv4(const char* IPAddress)
{
    int a, b, c, d;
    return sscanf(IPAddress, "%d.%d.%d.%d", &a, &b, &c, &d) == 4;
}

/*
bool
checkIPv4(std::string s)
{
    // Store the count of occurrence
    // of '.' in the given string
    int cnt = 0;

    // Traverse the string s
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == '.')
            cnt++;
    }

    // Not a valid IP address
    if (cnt != 3)
        return false;

    // Stores the tokens
    vector<string> tokens;

    // stringstream class check1
    stringstream check1(s);
    string intermediate;

    // Tokenizing w.r.t. '.'
    while (getline(check1, intermediate, '.'))
    {
        tokens.push_back(intermediate);
    }

    if (tokens.size() != 4)
        return false;

    // Check if all the tokenized strings
    // lies in the range [0, 255]
    for (int i = 0; i < tokens.size(); i++)
    {
        int num = 0;

        // Base Case
        if (tokens[i] == "0")
            continue;

        if (tokens[i].size() == 0)
            return false;

        for (int j = 0; j < tokens[i].size(); j++)
        {
            if (tokens[i][j] > '9' || tokens[i][j] < '0')
                return false;

            num *= 10;
            num += tokens[i][j] - '0';

            if (num == 0)
                return false;
        }

        // Range check for num
        if (num > 255 || num < 0)
            return false;
    }

    return true;
}

// Function to check if the string
// represents a hexadecimal number
bool
checkHex(std::string s)
{
    // Size of string s
    int n = s.length();

    // Iterate over string
    for (int i = 0; i < n; i++)
    {
        char ch = s[i];

        // Check if the character
        // is invalid
        if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'F') && (ch < 'a' || ch > 'f'))
        {
            return false;
        }
    }

    return true;
}

// Function to check if the given
// string S is IPv6 or not
bool
checkIPv6(std::string s)
{
    // Store the count of occurrence
    // of ':' in the given string
    int cnt = 0;

    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == ':')
            cnt++;
    }

    // Not a valid IP Address
    if (cnt != 7)
        return false;

    // Stores the tokens
    std::vector<std::string> tokens;

    // stringstream class check1
    std::stringstream check1(s);
    std::string intermediate;

    // Tokenizing w.r.t. ':'
    while (getline(check1, intermediate, ':'))
    {
        tokens.push_back(intermediate);
    }

    if (tokens.size() != 8)
        return false;

    // Check if all the tokenized strings
    // are in hexadecimal format
    for (int i = 0; i < tokens.size(); i++)
    {
        int len = tokens[i].size();

        if (!checkHex(tokens[i]) || len > 4 || len < 1)
        {
            return false;
        }
    }
    return true;
}

// Function to check if the string
// S is IPv4 or IPv6 or Invalid
void
checkIPAddress(std::string s)
{
    // Check if the string is IPv4
    if (checkIPv4(s))
        cout << "IPv4";

    // Check if the string is IPv6
    else if (checkIPv6(s))
        std::cout << "IPv6";

    // Otherwise, print "Invalid"
    else
        cout << "Invalid";
}
*/
bool
is_ipv6_address(const char* str)
{
    struct sockaddr_in6 sa;
    return inet_pton(AF_INET6, str, &(sa.sin6_addr)) != 0;
}

// this should move to string-utils.h
std::vector<std::string>
tokenize(const std::string str, const std::regex re)
{
    std::sregex_token_iterator it{str.begin(), str.end(), re, -1};
    std::vector<std::string> tokenized{it, {}};

    // Additional check to remove empty strings
    tokenized.erase(std::remove_if(tokenized.begin(),
                                   tokenized.end(),
                                   [](const std::string& s) { return s.size() == 0; }),
                    tokenized.end());

    return tokenized;
}

// converts the AS part of a SCION Address i.e. 19-faa:1:1067,192.168.127.1
// "ffaa:1:106" first into a hexadecimal number 0xffaa00010106 and then to integer

Asn
AsFromDottedHex(const std::string& str)
{
    auto token = tokenize(str, std::regex{R"([:]+)"});

    std::stringstream ss;

    std::stringstream hexStr;
    hexStr << "0x";
    hexStr << std::setfill('0') << std::setw(4); //<< "0x";
    for (const auto& t : token)
    {      
        hexStr << t;
        if (t == "0")
        {
            hexStr << '0' << '0'
                   << '0'; // somehow std::setw(4) doesnt seem to work with with fillchar
        }
    }
    Asn_t x;    
    ss << std::hex << hexStr.str();
    ss >> x;
    return Asn(x);
}


std::string
ASToDottedHex(Asn as)
{
    std::stringstream result;
    std::stringstream ss;
    ss << std::hex << (as.GetValue());

    bool begin = true;
    int encounteredZerosInRow = 0;
    int pos = 0;
    for (auto s : ss.str())
    {
        // the !begin is for the codepath that we last encountered 4 zeros in a row( and dont emit a
        // second ':' )
        if (pos != 0 && pos % 4 == 0 && !begin)
        {
            result << ":";
            encounteredZerosInRow = 0;
            begin = true;
            // ++pos;

            // continue;
        }
        // trim leading zeros
        if (begin)
        {
            if (s == '0')
            {
                ++pos;
                ++encounteredZerosInRow;               
                if (encounteredZerosInRow == 4)
                {                   
                    result << '0' << ':';
                    begin = true;
                    encounteredZerosInRow = 0;
                }
                // do not emit leading zeros
                // and leave begin at true
                // break;
                continue;
            }
            else
            {
                result << s;
                encounteredZerosInRow = 0;
                begin = false;
                pos++;
            }
        }
        else
        {
            // leave begin at false
            result << s;
            ++pos;
        }
    }

    return result.str();
}

} // namespace

/*
 host may be an Ipv4 or 6 Address, Inet or Inet6SocketAddr
*/
SCIONAddress::SCIONAddress(Ia ia, const Address& host)
{
    _ia = ia;

    if (Ipv4Address::IsMatchingType(host))
    {
        _hostAddr = host;
    }
    else if (Ipv6Address::IsMatchingType(host))
    {
        _hostAddr = host;
    }
    else if (InetSocketAddress::IsMatchingType(host))
    {
        InetSocketAddress a{InetSocketAddress::ConvertFrom(host)};
        _hostAddr = a.GetIpv4();
    }
    else if (Inet6SocketAddress::IsMatchingType(host))
    {
        Inet6SocketAddress a{Inet6SocketAddress::ConvertFrom(host)};     
        _hostAddr = a.GetIpv6();
    }
    else
    {
        NS_ASSERT(false);
    }
    
    addrType = ConvAddrType(_hostAddr);
}

/*
  19-ffaa:0:1067,192.168.1.1:8080
  submatch 0: 19-ffaa:0:1067,192.168.1.1:8080
  submatch 1: 19
  submatch 2: ffaa:0:1067
  submatch 3: 192.168.1.1
  submatch 4: 8080*/
SCIONAddress::SCIONAddress(const std::string& hostScionAddr)
{ //  std::string ipv6reg =
  //  "((?:((?:[0-9A-Fa-f]{1,4}:){1,6}:)|(?:(?:[0-9A-Fa-f]{1,4}:){7}))(?:[0-9A-Fa-f]{1,4}))";
    //"((([0-9A-Fa-f]{1,4}:){1,6}:)|(([0-9A-Fa-f]{1,4}:){7}))([0-9A-Fa-f]{1,4})";
    //    std::regex scionRegex{"^(\\d+)-([\\d:A-Fa-f]+),([^:]+|\\[" +ipv6reg
    //    +"\\]])(?::(\\d+))?$"}; // (?<=:)(\\d+)?$

    std::regex scionRegex{
        "^(?:(\\d+)-([\\d:A-Fa-f]+)),(?:\\[([^\\]]+)\\]|([^\\[\\]:]+))(?::(\\d+))?$"};
    // host part  (?:\[([^\]]+)\]|([^\[\]:]+))(?::(\d+))

    std::smatch pieces_match;

    if (std::regex_match(hostScionAddr, pieces_match, scionRegex))
    {
        auto isd = pieces_match[1].str();
        auto as = AsFromDottedHex(pieces_match[2].str());
        uint16_t iisd = std::stoi(isd);

        _ia = Ia(Isd(iisd), Asn(as));

        auto host = pieces_match[3].str();
        if (host.empty())
        {
            host = pieces_match[4].str();
        }

        if (isValidIPv4(host.c_str()))
        {
            /*  std::stringstream ss;
              ss << host;
              Ipv4Address ip;
              ss >>ip;
              _hostAddr = ip;
              // _hostAddr = Ipv4Address(host.c_str());
           */
            _hostAddr = Ipv4Address(ntohl(inet_addr(host.c_str())));

            addrType = AddrType_t::T4Ip;
        }
        else if (is_ipv6_address(host.c_str()))
        {
            _hostAddr = Ipv6Address(host.c_str());
            addrType = AddrType_t::T16Ip;
        }
        else // TODO service addresses
        {
            assert(false);
        }
 
    }
    else
    {
        NS_ASSERT_MSG(false," cannot construct SCIONAddress from invalid String: " << hostScionAddr );
    }
    
}

/**
 * Get the number of bytes needed to serialize the underlying Address
 * Typically, this is GetLength () + 2
 *
 * \returns the number of bytes required for an Address in serialized form
 */
uint32_t
SCIONAddress::GetSerializedSize() const
{ // 1b for type, 1b for len, 1 byte for hostAddrType, 8 for IA + hostAddress
    return 1 + 1 + 1 + 8 + _hostAddr.GetSerializedSize();
}

/**
 * \brief Get the length of the underlying address.
 * \returns the length of the underlying address.
 */
uint8_t
SCIONAddress::GetLength() const
{
    return 8 + 1 + 8 + _hostAddr.GetSerializedSize();
}

/**
 * \param address an address to compare type with
 *
 * \return true if the type of the address stored internally
 * is compatible with the type of the input address, false otherwise.
 */
bool
SCIONAddress::IsMatchingType(const Address& address)
{
    return address.IsMatchingType(GetType());
}

/**
 * Convert an instance of this class to a polymorphic Address instance.
 *
 * \return a new Address instance
 */
SCIONAddress::operator Address() const
{
    return ConvertTo();
}

/**
 * \brief Get the underlying address type (automatically assigned).
 *
 * \returns the address type
 */
uint8_t
SCIONAddress::GetType()
{
    static uint8_t type = Address::Register();
    return type;
}

/**
 * \brief Convert to an Address type
 * \return the Address corresponding to this object.
 */
Address
SCIONAddress::ConvertTo() const
{
    uint8_t* buf = (uint8_t*)malloc(GetSerializedSize());
    Serialize(buf);
    auto tmp = Address(GetType(), buf, GetSerializedSize());
    free(buf);
    return tmp;
}


AddrType_t
ConvAddrType(const Address & addr)
{
    if (Ipv4Address::IsMatchingType(addr))
    {
        return AddrType_t::T4Ip;
    }
    else if (Ipv6Address::IsMatchingType(addr))
    {
        return AddrType_t::T16Ip;
    }
    /*case AddrType_t::T4Svc:
    {
      return 0; // TODO implement ServiceAddress class
    }*/

    return AddrType_t::T4Ip;
}

/**
 * \brief Print this address to the given output stream
 *
 * The print format is in the typical "192.168.1.1"
 * \param os The output stream to which this Ipv4Address is printed
 */
void
SCIONAddress::Print(std::ostream& os) const
{
    os << GetISD() << "-" << ASToDottedHex(GetAS()) << std::dec << ",";

    if (addrType == AddrType_t::T4Ip)
    {
        auto ipv4 = Ipv4Address::ConvertFrom(_hostAddr);
        ipv4.Print(os);
    }
    else if ( addrType == AddrType_t::T16Ip )
    {
        Ipv6Address::ConvertFrom(_hostAddr).Print(os);
    }
    else
    {
        assert(false);
    }
    // TODO  Service Addresses ...
   
}

/**
 * \brief Stream insertion operator.
 *
 * \param os the stream
 * \param address the address
 * \returns a reference to the stream
 */
std::ostream&
operator<<(std::ostream& os, const SCIONAddress& address)
{
    address.Print(os);
    return os;
}

/**
 * \brief Stream extraction operator.
 *
 * \param is the stream
 * \param address the address
 * \returns a reference to the stream
 */
std::istream&
operator>>(std::istream& is, SCIONAddress& address)
{
    std::string str;
    is >> str;
    address = SCIONAddress(str);
    return is;
}

} // namespace ns3