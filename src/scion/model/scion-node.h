/*
 * Copyright (c) 2025
 *
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
 *
 * Authors: 
 *          
 */
#ifndef SCIONNODE_H
#define SCIONNODE_H
#include "ns3/scion-ia.h"
#include "ns3/node.h"

namespace ns3 {
    /**
     * \brief a special Node for SCION simulations
     *  It has additional Attributes to ease the distinction between 
     *  EndHosts and BorderRouters (which have at least one Link/Channel connected to a Node of another AS)
     */
    class SCIONNode : public Node 
    {

  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    SCIONNode();
    /**
     * \param systemId a unique integer used for parallel simulations.
     */
    SCIONNode(uint32_t systemId);

    ~SCIONNode() override = default;

    virtual TypeId GetInstanceTypeId() const override;

        /**
     * \brief Attribute system setter for the IA.
     * \param ia The IA value received from the attribute system.
     */
    void SetIaAttribute(Ia ia); // Note: Takes Ia by value is common for attr setters

    /**
     * \brief Attribute system getter for the IA.
     * \return The current IA value.
     */
    Ia GetIaAttribute() const;
private:
        /// @brief this Node's IA (single-valued: currently no multi-homing is supported)
        Ia m_ia;
    };
}

#endif