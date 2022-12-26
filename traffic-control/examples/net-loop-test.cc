// The code taken from web:https://blog.csdn.net/Mary19920410/article/details/72817846
// author: 海阔天空sky1992
// only using p2p not csma
/*
           n8                     n9
           |                      |
          router2(2)------router4(4)
         /                        \
        /                          \
A(0)----router1(1)                 router6(6)----H(7)
        \                          /
         \                        /
          router3(3)------router5(5)
          |                      |
          n10                    n11
(i)为程序中NodeContainer nodes中节点的顺序号
*/
 
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
 
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
 
using namespace ns3;
 
NS_LOG_COMPONENT_DEFINE ("myFirstExample");
 
int
main (int argc, char * argv[])
{
 
 
 /* uint16_t nodesNum = 12;
  float dataRateAtoB = 100.0;//A——B
  float dataRateGtoH = 100.0;//G——H
  float dataRateBtoC = 100.0;
  float dataRateBtoD = 100.0;
  float dataRateGtoE = 100.0;
  float dataRateGtoF = 100.0;
  float dataRateNodetoaRouter = 100.0;
  float dataRateR1toR3 = 5.0;
  float dataRateR2toR4 = 5.0;
 */
  /*使用可视化工具 PyViz*/
  CommandLine cmd;
  cmd.Parse (argc,argv);
 
  //设置默认拥塞控制算法
  //Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpReno")); 
 
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1024));  //设置默认包的尺寸
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("50Mb/s"));  //设置默认发包速率
  /*创建节点
  1 client  (lefts)
  1 server  (rights)
  4 nodes to generate background flow
  6 routers = 1 封装&转发 + 1解封&转发 + 4转发
  */
  std::cout << "create nodes"<<std::endl;
  NodeContainer left, right, routers, othernodes;
  left.Create(1);
  right.Create (1);
  routers.Create (6);//多路径转发设备为1个路由
  othernodes.Create(4);//产生背景流量
  NodeContainer nodes = NodeContainer(left, routers,right, othernodes);
  //各条边的节点组合,一共12条边
  std::vector<NodeContainer> nodeAdjacencyList(12);  
  nodeAdjacencyList[0]=NodeContainer(nodes.Get(0),nodes.Get(1));//A(0)
  nodeAdjacencyList[1]=NodeContainer(nodes.Get(1),nodes.Get(2));  
  nodeAdjacencyList[2]=NodeContainer(nodes.Get(1),nodes.Get(3));  
  nodeAdjacencyList[3]=NodeContainer(nodes.Get(2),nodes.Get(4));  
  nodeAdjacencyList[4]=NodeContainer(nodes.Get(3),nodes.Get(5)); 
  nodeAdjacencyList[5]=NodeContainer(nodes.Get(4),nodes.Get(6)); 
  nodeAdjacencyList[6]=NodeContainer(nodes.Get(5),nodes.Get(6)); 
  nodeAdjacencyList[7]=NodeContainer(nodes.Get(6),nodes.Get(7)); //H(7)
  nodeAdjacencyList[8]=NodeContainer(nodes.Get(2),nodes.Get(8)); 
  nodeAdjacencyList[9]=NodeContainer(nodes.Get(4),nodes.Get(9)); 
  nodeAdjacencyList[10]=NodeContainer(nodes.Get(3),nodes.Get(10)); 
  nodeAdjacencyList[11]=NodeContainer(nodes.Get(5),nodes.Get(11));
 
  /*配置信道*/
  std::vector<PointToPointHelper> p2p(12);  
  //高速路段
  for(int i = 0; i < 3; i ++){
    p2p[i].SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));//设置带宽
    p2p[i].SetChannelAttribute ("Delay", StringValue ("5ms"));  //设置时延
  }
  //瓶颈路段1
  p2p[3].SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p[3].SetChannelAttribute ("Delay", StringValue ("10ms"));  
  //瓶颈路段2
  p2p[4].SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p[4].SetChannelAttribute ("Delay", StringValue ("20ms"));  
  //高速路段
  for(int i = 5; i < 8; i ++){
    p2p[i].SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));//设置带宽
    p2p[i].SetChannelAttribute ("Delay", StringValue ("5ms"));  //设置时延
  }
  //背景流量段
  for(int i = 8; i < 12; i ++){//上次错误，此处的i写成了3
    p2p[i].SetDeviceAttribute ("DataRate", StringValue ("5Mbps")); //背景流量段带宽可设置5,10,20
    p2p[i].SetChannelAttribute ("Delay", StringValue ("5ms"));  
  }
 
  std::vector<NetDeviceContainer> devices(12);
  for(int i = 0; i < 12; i ++){
    devices[i] = p2p[i].Install(nodeAdjacencyList[i]);
  }
  /*安装网络协议栈*/
  InternetStackHelper stack;  
  stack.Install (nodes);//安装协议栈，tcp、udp、ip等  
  /*上面1句等价于下面4句
  stack.install(left);
  stack.install(right);
  stack.install(routers);
  stack.install(othernodes);
  */
  //NS_LOG_INFO ("Assign IP address.");
  Ipv4AddressHelper address;  
  std::vector<Ipv4InterfaceContainer> interfaces(12);  
  for(uint32_t i=0; i<12; i++)  
  {  
    std::ostringstream subset;  
    subset<<"10.1."<<i+1<<".0"; 
    //n0:10.1.1.1    n1接口1:10.1.1.2   n1接口2:10.1.2.1  n2接口1:10.1.2.2  ...
    address.SetBase(subset.str().c_str (),"255.255.255.0");//设置基地址（默认网关）、子网掩码  
    interfaces[i]=address.Assign(devices[i]);//把IP地址分配给网卡,ip地址分别是10.1.1.1和10.1.1.2，依次类推  
  }  
  //分配给A的IP地址为10.1.1.1  H的IP地址为10.1.8.2（A和H为多路径的两端）
  /*接下来是建立通信的app和路由表的建立*/  
  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();//这里直接调用了ns3的路由实现，后期需要修改
  
  /*在n7,n9,n11的8080端口建立SinkApplication
  发数据 n0——n7   （背景流量 n8——n9  n10——n11）
  sink接收TCP流数据
  */
  //uint16_t servPort = 8080;
  //PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), servPort));
  //ApplicationContainer sinkApp = sinkHelper.Install (n);
  //sinkApp.Start (Seconds (0));
  //sinkApp.Stop (Seconds (30.0));
  //uint16_t serverSum = 3;  //n7,n9,n11上安装serverApp,所以server一共3个
  uint16_t servPort = 50000;  
  ApplicationContainer sinkApp;  
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), servPort));  //???
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);   //第二个参数为上一行中的Address
  sinkApp.Add(sinkHelper.Install(nodeAdjacencyList[7].Get(1)));  
  sinkApp.Add(sinkHelper.Install (nodeAdjacencyList[9].Get(1)));  
  sinkApp.Add(sinkHelper.Install(nodeAdjacencyList[11].Get(1)));  
  sinkApp.Start (Seconds (0.0));  
  sinkApp.Stop (Seconds (10.0));  //应该为10s
 
  /*在n0, n8, n10安装client*/
  OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());  
  //OnOffApplication类中的m_onTime和m_offTime分别为发送持续的时间和不发送持续的时间。如下表示一直发送。
  clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));  
  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]")); 
 
  ApplicationContainer clientApps1, clientApps2, clientApps3;  
  uint16_t port = 50000;
  //n0->n7  
  AddressValue remoteAddress(InetSocketAddress (interfaces[7].GetAddress (1), port));  //目的地址
  clientHelper.SetAttribute("Remote",remoteAddress);  
  clientApps1 = clientHelper.Install(nodeAdjacencyList[0].Get(0));  
  
  //n8->n9 
  remoteAddress=AddressValue(InetSocketAddress (interfaces[9].GetAddress (1), port));  
  clientHelper.SetAttribute("Remote",remoteAddress);  
  clientApps2 = clientHelper.Install(nodeAdjacencyList[8].Get(1));  
  
  //n10->n11
  remoteAddress=AddressValue(InetSocketAddress (interfaces[11].GetAddress (1), port));  
  clientHelper.SetAttribute("Remote",remoteAddress);  
  clientApps3 = clientHelper.Install(nodeAdjacencyList[10].Get(1));  
 
  clientApps1.Start(Seconds(1.0));  
  clientApps1.Stop(Seconds (10.0)); //10s
 
  clientApps2.Start(Seconds(2.0)); //2s
  clientApps2.Stop(Seconds (4.0)); //4s
 
  clientApps3.Start(Seconds(2.0)); //2s
  clientApps3.Stop(Seconds (4.0)); //4s
 
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
  //嗅探,记录所有节点相关的数据包  
  for(uint32_t i=0; i<11; i++)  
      p2p[i].EnablePcapAll("mytest");  
  
  //在TCP连接时间内统计PacketSink收到的TCP包，计算吞吐量并打印
 // Ptr<PacketSink>pktSink;
 // std::cout << "Rx = " << PacketSink-> GetTotalRx() << "bytes("
 // << pktSink->GetTotalRx()*8 / (stopTime - startTime) << "bps)," << endl;
  std::cout << "last"<<std::endl;
  Simulator::Run ();  
  std::cout << "lastlastlast"<<std::endl;
  Simulator::Destroy ();  
  std::cout << "lastlast-finish"<<std::endl;
  return 0;  
}