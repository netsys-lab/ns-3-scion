#include "ns3/scion-path.h"
#include "ns3/log.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("ScionPath");

bool BasePath::IncPath()
{
    if( NumINF == 0 )
    {
      NS_LOG_ERROR("empty path cannot be increased");
      return false;
    }

   if( CurrHF >= NumHops-1 )
   {
		CurrHF = NumHops - 1;
		NS_LOG_ERROR("path already at end");
        return false;
	}
	CurrHF++;
	// Update CurrINF
	CurrINF = infIndexForHF(CurrHF);

	return true;
}

// IsXover returns whether we are at a crossover point. This includes
// all segment switches, even over a peering link. Note that handling
// of a regular segment switch and handling of a segment switch over a
// peering link are fundamentally different. To distinguish the two,
// you will need to extract the information from the info field.
bool BasePath::IsXover() const
{
	return CurrHF + 1 < uint8_t(NumHops) &&
		CurrINF != infIndexForHF(CurrHF + 1 );
}

// IsFirstHopAfterXover returns whether this is the first hop field after a crossover point.
bool BasePath::IsFirstHopAfterXover() const
{
	return CurrINF > 0 && CurrHF > 0 &&
		CurrINF-1 == infIndexForHF(CurrHF-1);
}

/*
 writes the fields into the provided buffer.
 The buffer must be of length >= scion.MetaLen.
*/
 void BasePath::Serialize( Buffer::Iterator start ) const
 {
    NS_ASSERT_MSG( start.GetRemainingSize()>= GetSerializedSize() , "Not Enough buffer space to serialize MetaHdr");
	 
	uint32_t line = uint32_t(CurrINF)<<30 | uint32_t(CurrHF&0x3F)<<24;
	line |= uint32_t(SegLen[0]&0x3F) << 12;
	line |= uint32_t(SegLen[1]&0x3F) << 6;
	line |= uint32_t(SegLen[2] & 0x3F);
	start.WriteHtonU32( line );
 }

    /*
    populates the fields from a raw buffer.
    The buffer must be of length >= scion.MetaLen.
    */
    uint32_t BasePath::Deserialize(Buffer::Iterator start)
    {
    
    NS_ASSERT_MSG( !( start.GetRemainingSize() < METALEN ), "insufficient buffer" );	

	auto line = start.ReadNtohU32();
	CurrINF = uint8_t(line >> 30);
	CurrHF = uint8_t(line>>24) & 0x3F;
	SegLen[0] = uint8_t(line>>12) & 0x3F;
	SegLen[1] = uint8_t(line>>6) & 0x3F;
	SegLen[2] = uint8_t(line) & 0x3F;

    NumINF = 0;
	NumHops = 0;
	for ( int i = 2; i >= 0; i-- )
    {
		NS_ASSERT_MSG( !(SegLen[i] == 0 && NumINF > 0 ), "Meta.SegLen["<< i<< "] == 0, but Meta.SegLen[" << NumINF-1 << "] > 0");
					
		if ( SegLen[i] > 0 && NumINF == 0 )
        {
			NumINF = static_cast<uint8_t>(i) + 1;
		}
		NumHops += SegLen[i];

		
	}

    return METALEN;
    }

uint8_t BasePath::infIndexForHF( uint8_t hf)  const
{

	if( hf < SegLen[0])
		return 0;
	if( hf < SegLen[0] + SegLen[1] )
		return 1;
	else
		return 2;

}


    // Len returns the length of the path in bytes.
    uint16_t BasePath::Len() const
    {
        return METALEN + NumINF * INFO_FIELD_LEN + NumHops * HOPLEN;
    }

constexpr uint32_t BasePath::GetSerializedSize(){ return METALEN; }

 std::ostream&
operator<<(std::ostream& os, const SCIONPath& dp)
{
    os << dp;

    for (const auto& info : dp.InfoFields)
    {
        os << "info: " << info;
    }

    for (const auto& hop : dp.HopFields)
    {
        os << "hop: " << hop;
    }

    return os;
}


InfoField SCIONPath::GetInfoField( int index) const
{
 return InfoFields.at(index);
}


HopField SCIONPath::GetHopField(int index ) const
{
return HopFields.at(index);
}

bool BasePath::IsLastHop() const
{
    return CurrHF == NumHops - 1;
}


// IsPenultimateHop returns whether the current hop is the penultimate hop on the path.
bool
BasePath::IsPenultimateHop() const
{
    return CurrHF == (NumHops - 2);
}

// IsFirstHop returns whether the current hop is the first hop on the path.
bool BasePath::IsFirstHop() const
{
    return CurrHF == 0;
}

BasePath& BasePath::Reverse()
{
 NS_ASSERT_MSG(NumINF > 0, "empty decoded path is invalid and cannot be reversed" );
        for( int j = NumINF-1, i=0 ; i < j; --j, ++i )
		{                        
            std::swap(SegLen[i], SegLen[j] );
		  
        }
    // Update CurrINF and CurrHF and SegLens
	CurrINF = NumINF - CurrINF - 1;
	CurrHF = NumHops - CurrHF - 1;
    return *this;
}

/** \brief reverse the path in-place */
SCIONPath& SCIONPath::Reverse()
{	
   this->BasePath::Reverse();

	// Reverse order of InfoFields
//	for( int i = 0; ; i++ )
    {
        for( int j = GetNumINF()-1, i=0 ; i < j; --j, ++i )
		{
            std::swap( InfoFields[i], InfoFields[j] );          
        }
   //     break;
	}
	// Reverse cons dir flags
	for ( int i = 0; i < GetNumINF(); i++ )
    {
		auto& info = InfoFields[i];
		info.ConsDir = !info.ConsDir;
	}
	// Reverse order of hop fields
//	for ( int i = 0; ;++i )
    {
        for( int j= GetNumHF()-1, i = 0; i < j; j--, ++i )
            std::swap( HopFields[i], HopFields[j]);
		
	}
	

	return *this;
}

    InfoField BasePath::GetCurrentInfoField()const
    {
        return GetInfoField(GetCurrINF());
    }

    HopField BasePath::GetCurrentHopField() const
    {
        return GetHopField(GetCurrHF());
    }



// SerializeTo writes the path to a slice.
// The slice must be big enough to hold the entire data,
// otherwise an error is returned.
void
SCIONPath::Serialize(Buffer::Iterator start) const
{
    this->BasePath::Serialize(start);
    start.Next(this->BasePath::GetSerializedSize());

    for (auto& info : InfoFields)
    {
        info.Serialize(start);
        start.Next(sizeof(InfoField));
    }
    for (auto& hop : HopFields)
    {
        hop.Serialize(start);
        start.Next(sizeof(HopField));
    }
}

// DecodeFromBytes fully decodes the SCION path into the corresponding fields.
uint32_t
SCIONPath::Deserialize(Buffer::Iterator start)
{
    this->BasePath::Deserialize(start);
    start.Next(BasePath::GetSerializedSize());

    InfoFields.clear();
    InfoFields.resize( GetNumINF() );
    for (auto& infoField : InfoFields)
    {
        infoField.Deserialize(start);
        start.Next(sizeof(InfoField));
    }

    HopFields.clear();
    HopFields.resize( GetNumHF());
    for (auto& hf : HopFields)
    {
        hf.Deserialize(start);
        start.Next(sizeof(HopField));
    }
    return Len();
}


/*
void
SCIONPath::Serialize( buffer_iterator start) const
{
NS_ASSERT_MSG( start.GetRemainingSize() >= Len() ,
 "not enough buffer space to serialize SCIONPath" );

 base.PathMeta.Serialize(start);
 start+=base.PathMeta.GetSerializedSize();

    for (const auto& info : InfoFields)
    {
        info.Serialize(start);
        start += INFO_FIELD_LEN;
    }

    for ( const auto& hop : HopFields)
    {
        hop.Serialize(start);
        start += HOPLEN;
    }
}

std::expected<uint32_t,error>
SCIONPath::Deserialize( const_buffer_iterator start)
{

    uint32_t bytes;
    if( auto tmp = base.Deserialize(start); tmp)
    {bytes = *tmp;
    }else
    {
        return std::unexpected( tmp.error() );
    }

// vielleicht muss die start+= *tmp zeile  hinter den '< minLen' check verschoben werden
    
    	if(auto minLen = Len(); start.GetRemainingSize() < minLen )
        {
		return std::unexpected( basic_error{"SCIONPath raw too short",
         "expected", std::to_string(minLen),
          "actual", std::to_string( start.GetRemainingSize() )
          } );
	}
    
    start += bytes;
    InfoFields.clear();
    InfoFields.resize( base.NumINF);
    for (auto& infoField : InfoFields)
    {
        infoField.Deserialize(start);
        start+= INFO_FIELD_LEN;
    }

    HopFields.clear();
    HopFields.resize(base.NumHops);
    for (auto& hf : HopFields)
    {
        hf.Deserialize(start);
        start += HOPLEN;
    }
    //return HOPLEN;
    return base.Len();
}
    */


}; // namespace ns3