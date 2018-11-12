/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Stanford University
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
 * Author: Stephen Ibanez <sibanez@stanford.edu>
 */

#ifndef P4_QUEUE_DISC_H
#define P4_QUEUE_DISC_H

#include "ns3/queue-disc.h"
#include "ns3/p4-pipeline.h"
#include <array>
#include <string>

namespace ns3 {


/**
 * \ingroup traffic-control
 *
 * The P4 qdisc is configured by a P4 program. It contains qdisc classes
 * which actually perform the queueing and scheduling. This qdisc is
 * intended to be the root qdisc that simply runs the user's P4 program
 * and then passes the modified packet to the appropriate qdisc class
 * (or drops the packet if the P4 program says to do so).
 */
class P4QueueDisc : public QueueDisc {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief P4QueueDisc constructor
   */
  P4QueueDisc ();

  virtual ~P4QueueDisc();

  /// Get the P4 source file
  std::string GetP4File (void) const;

  /// Set the P4 source file
  void SetP4File (std::string p4file);

  static constexpr const char* P4_DROP = "P4 drop";      //!< P4 program said to drop packet before enqueue

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);
  virtual bool CheckConfig (void);
  virtual void InitializeParams (void);

  /// The P4 source file
  std::string m_p4file;

  /// The P4 pipeline
  SimplePipe m_p4_pipe;

};

} // namespace ns3

#endif /* P4_QUEUE_DISC_H */
