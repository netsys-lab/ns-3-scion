#ifndef SCION_ROUTING_DAEMON_H
#define SCION_ROUTING_DAEMON_H


#include "scion-header.h"
#include "scion.h"

#include "ns3/callback.h"
#include "ns3/scion-interface-address.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/scion-path.h"

namespace ns
{

    /**
     * a list of callbacks, and or criteria which the user can provide
     * to influence PathSelection
     */
    class RouteSpecification
    {
        // Fcn that selects a path from a set of candidates
        //using PathSelectorT = Callback<uint16_t, const std::vector<BasePath>& >;

        // expresses preferences between path choices
        using PathCompareLessT = Callback<bool, const BasePath&, const BasePath&>;
    };

    /** 
    \details  the counterpart of 'Ipv4RoutingProtocol' in a source-routed network architecture
              It is only required for EndHosts (that host applications, which need to 'dial' remote hosts and thus need paths),
               not BorderRouters.


        All the Notify Interface UP/Down, Address Added/Removed stuff or IP routing protocols,
        is not required here.               
    */
    class SCIONRoutingDaemon : public Object
    {
   /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    virtual void SetSCION(Ptr<SCION> scion);

    /// Callback for packets to be locally delivered
    typedef Callback<void, Ptr<const Packet>, const SCIONHeader&, uint32_t> LocalDeliverCallback;

    /// Callback for routing errors (e.g., no route found)
    typedef Callback<void, Ptr<const Packet>, const SCIONHeader&, Socket::SocketErrno> ErrorCallback;


    /** 
     * used for locally originated packets 
     * to compute or select(from cache) a dataplane path for the given packet.
     * This lookup is used by transport protocols.  It does not cause any
     * packet to be forwarded, and is synchronous.
     * \param p the packet to be sent (without a SCION header yet)
     * \param oif Output interface Netdevice.  May be zero, or may be bound via
     *            socket options to a particular output interface.
    */
    virtual Ptr<SCIONRoute> RouteOutput(Ptr<Packet> p,
                                       const SCIONHeader& header,
                                       Ptr<NetDevice> oif,
                                       const RouteSpecification& path_specs,
                                       Socket::SocketErrno& sockerr);
 

        /** \brief record the dataplane path of a packet, 
         *         before it gets delivered locally (passed up the stack).
         *        This allows to cache reply-paths and also path-usage information.
         *  \param p received packet
         */
       virtual bool RouteInput(Ptr<const Packet> p,
                            const SCIONHeader& header,
                            Ptr<const NetDevice> idev,                            
                            LocalDeliverCallback lcb,
                            ErrorCallback ecb);
    };
}


#endif