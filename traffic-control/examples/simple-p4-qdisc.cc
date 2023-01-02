/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 */

/**
 * @brief The program reduces network congestion by dropping packets on the switch. 
 * The switch is controlled by the p4 program, which implements the probability of 
 * packet loss by relative queue length.
 * 
 *  Network topology
 * 
 *           n0
 *           |
 *     --------------
 *     |  (router)  |
 *     |            |
 *     | [p4-qdisc] |
 *     --------------
 *           | 
 *           n1
 *
 * CBR/UDP flow from n0 to n1
 * P4 qdisc at egress link of router 
 * Tracing of queues and packet receptions to file "router.tr"
 * 
 * NOTE: these should be the Directory "./trace-data/" for saving the trace file 
 * "./trace-data/tc-qsize.txt", if not: these will be an error.
 */

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/traffic-control-module.h"

#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleP4QdiscExample");

void
TcBytesInQueueTrace (Ptr<OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
  //*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << newValue << std::endl;
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "," << newValue << "," << oldValue << std::endl;
}

void
TcDropTrace (Ptr<const QueueDiscItem> item)
{
  std::cout << "TC(Traffic Control) dropped packet!" << std::endl;
}

void
DeviceBytesInQueueTrace (Ptr<OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
  //*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << newValue << std::endl;
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "," << newValue << "," << oldValue << std::endl;
}

void
DeviceDropTrace (Ptr<const Packet> p)
{
  std::cout << "Device dropped packet!" << std::endl;
}

int 
main (int argc, char *argv[])
{
  LogComponentEnable ("SimpleP4QdiscExample", LOG_LEVEL_INFO);

  CommandLine cmd;
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1024));

  NS_LOG_INFO ("Create nodes");
  Ptr<Node> n0 = CreateObject<Node> ();
  Ptr<Node> n1 = CreateObject<Node> ();
  Ptr<Node> router = CreateObject<Node> ();

  NS_LOG_INFO ("Build Topology");
  CsmaHelper csma;
  // factors of inftuence: data_rate, delay
  csma.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0.2))); // the delay will impact the queue length. // default 2 ms

  // Create the csma links, from each terminal to the router
  NetDeviceContainer n0rDevices = csma.Install (NodeContainer (n0, router));
  NetDeviceContainer n1rDevices = csma.Install (NodeContainer (n1, router));

  Ptr<NetDevice> n1Device = n1rDevices.Get (0);
  Ptr<NetDevice> rDevice = n1rDevices.Get (1);

  // Add internet stack to the all nodes 
  InternetStackHelper stack;
  stack.Install (NodeContainer (n0, n1, router));

  // Add traffic control alg with p4 / tranditional approach
  TrafficControlHelper tch;
  //tch.SetRootQueueDisc ("ns3::RedQueueDisc"); // RED, the tranditional approach, test with this success!
  tch.SetRootQueueDisc ("ns3::P4QueueDisc",
                        "JsonFile", StringValue("src/traffic-control/examples/p4-src/simple-p4-qdisc/build/simple-p4-qdisc.json"),
                        "CommandsFile", StringValue("src/traffic-control/examples/p4-src/simple-p4-qdisc/commands.txt"),
                        "QueueSizeBits", UintegerValue (16), // # bits used to represent range of values
                        "QW", DoubleValue (0.002), // Queue weight related to the exponential weighted moving average (EWMA)
                        "MeanPktSize", UintegerValue (500),
                        "LinkBandwidth", DataRateValue (DataRate ("10Mbps")) // The P4 queue disc link bandwidth
                        );
  
  // Install Queue Disc on the router interface towards n1
  // QueueDiscContainer qdiscs = tch.Install (rDevice);
  QueueDiscContainer qdiscs = tch.Install (n0rDevices);
  tch.Install (n1rDevices);

  // We've got the "hardware" in place. Now we need to add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer n0Interfaces;
  n0Interfaces = ipv4.Assign (n0rDevices);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer n1Interfaces;
  n1Interfaces = ipv4.Assign (n1rDevices);
  
  // Initialize routing database and set up the routing tables in the nodes. 
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /*
  // echo test, whether the network is connect right.
  uint16_t servPort = 9090;
  UdpEchoServerHelper echoServer (servPort);
  ApplicationContainer serverApps = echoServer.Install (n1);
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (Ipv4Address ("10.1.2.1"), servPort);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (n0);
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  
  csma.EnablePcap ("second", n1Device, true);
  */

  NS_LOG_INFO ("Create Applications for receiver");
  uint16_t servPort = 9093;
  //Address sinkLocalAddress (InetSocketAddress (Ipv4Address ("10.1.2.1"), servPort));
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), servPort));
  // Start the receive(sink) server first
  PacketSinkHelper sink ("ns3::UdpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sink.Install (n1);

  sinkApp.Start (Seconds (1.0));
  sinkApp.Stop (Seconds (10.0));

  NS_LOG_INFO ("Create Applications for sender");
  // Start the send client second
  uint16_t port = 9093;
  Address remoteAddress (InetSocketAddress ("10.1.2.1", port)); // remote ip for n1 with 10.1.2.1
  OnOffHelper onoff ("ns3::UdpSocketFactory", remoteAddress);
  onoff.SetConstantRate (DataRate ("5Mbps"));
  ApplicationContainer app0 = onoff.Install (n0);

  app0.Start (Seconds (2.0));
  app0.Stop (Seconds (10.0));

  // Configure tracing of both TC queue and NetDevice Queue at bottleneck
  NS_LOG_INFO ("Configure Tracing.");
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> tcStream = asciiTraceHelper.CreateFileStream ("./trace-data/tc-qsize.txt"); // make sure that dir "trace-data" exist.
  Ptr<QueueDisc> qdisc = qdiscs.Get (0);
  qdisc->TraceConnectWithoutContext ("BytesInQueue", MakeBoundCallback (&TcBytesInQueueTrace, tcStream));
  qdisc->TraceConnectWithoutContext ("Drop", MakeCallback (&TcDropTrace));

  Ptr<OutputStreamWrapper> devStream = asciiTraceHelper.CreateFileStream ("./trace-data/dev-qsize.txt");
  Ptr<CsmaNetDevice> csmaNetDev = DynamicCast<CsmaNetDevice> (rDevice);
  Ptr<Queue<Packet>> queue = csmaNetDev->GetQueue ();
  queue->TraceConnectWithoutContext ("BytesInQueue", MakeBoundCallback (&DeviceBytesInQueueTrace, devStream));
  queue->TraceConnectWithoutContext ("Drop", MakeCallback (&DeviceDropTrace));

  // Setup pcap capture on n0's NetDevice.
  // Can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  csma.EnablePcap ("trace-data/n1device", n1Device);
  
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
