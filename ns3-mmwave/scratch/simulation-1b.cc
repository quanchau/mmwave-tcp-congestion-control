/**
* Simulation 1b
* 
* @author: Quan Chau (quananhchau@gmail.com)
* Dickinson College 2020
**/ 


#include "ns3/point-to-point-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include <ns3/buildings-helper.h>
#include <ns3/buildings-module.h>
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"

using namespace ns3;
using namespace mmwave;

Ptr<PacketSink> sink;
uint64_t lastTotalRx = 0; 
float lastSecond = 0;

/**
 * A script to simulate the DOWNLINK TCP data over mmWave links
 * with the mmWave devices and the LTE EPC.
 */
NS_LOG_COMPONENT_DEFINE ("mmWaveTCPExample");

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  static int send_num = 1;
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);
  NS_LOG_DEBUG ("Sending:    " << send_num++ << "\t" << Simulator::Now ().GetSeconds ());

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << packet->GetSize () << std::endl;
}


void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

 static void
 RxDrop (Ptr<const Packet> p)
 {
   NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
 }

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () << "\t" << newRtt.GetSeconds () << std::endl;
}


void
ChangeSpeed (Ptr<Node> n, Vector speed)
{
  n->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (speed);
  NS_LOG_UNCOND ("************************--------------------Change Speed-------------------------------*****************");
}


void
CalculateThroughput()
{
  Time now = Simulator::Now ();                                       /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     /* Convert Application RX Packets to MBits. */
  lastSecond = now.GetSeconds ();
  std::cout << lastSecond  << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
}

static void
RtoChange (Ptr<OutputStreamWrapper> stream, Time oldRto, Time newRto)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRto.GetSeconds () << "\t" << newRto.GetSeconds () << std::endl;
}

static void
SegnumChange (Ptr<OutputStreamWrapper> stream, SequenceNumber32 old, SequenceNumber32 nextTx)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << nextTx << std::endl;
}


int
main (int argc, char *argv[])
{
  /**
  TCP Parameters
  **/
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000)); // Standard MSS
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (131072 * 40)); // 5 MB
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (131072 * 40)); // 5 MB
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHybla::GetTypeId ()));

  /**
  General Parameters
  **/
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024 * 1024));
  Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue (1024 * 1024));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue (72));
  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveBeamforming::LongTermUpdatePeriod", TimeValue (MilliSeconds (100.0)));
  Config::SetDefault ("ns3::LteRlcAm::PollRetransmitTimer", TimeValue (MilliSeconds (4.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReorderingTimer", TimeValue (MilliSeconds (2.0)));
  Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue (MilliSeconds (1.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MilliSeconds (4.0)));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (10 * 1024 * 1024)); // 10 MB
  Config::SetDefault ("ns3::MmWave3gppChannel::DirectBeam", BooleanValue (true)); // Set true to perform the beam in the exact direction of receiver node.
  Config::SetDefault ("ns3::MmWave3gppChannel::Blockage", BooleanValue (true)); // use blockage or not 
  Config::SetDefault ("ns3::MmWavePhyMacCommon::NumHarqProcess", UintegerValue (100));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDelay", TimeValue (MicroSeconds (500)));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDataRate", DataRateValue (DataRate ("1000Gb/s")));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkMtu",  UintegerValue (10000));
  
  /**
  3GPP Parameters
  **/
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue (true)); // enable or disable the shadowing effect
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue ("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue ("a"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::OptionalNlos", BooleanValue (true));
  Config::SetDefault ("ns3::MmWave3gppBuildingsPropagationLossModel::UpdateCondition", BooleanValue (true)); // enable or disable the LOS/NLOS update when the UE moves

  /** OUTPUT FILES **/
  Config::SetDefault ("ns3::McStatsCalculator::LteOutputFilename", StringValue ("LTE.txt"));
  Config::SetDefault ("ns3::MmWaveBearerStatsConnector::LteSinrOutputFilename", StringValue ("LTESinr.txt"));

  double stopTime = 4.5;
  double simStopTime = 4.5;
  Ipv4Address remoteHostAddr;

  // Command line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
  mmwaveHelper->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
  Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmwaveHelper->SetEpcHelper (epcHelper);
  mmwaveHelper->SetHarqEnabled (true);

  mmwaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));
  mmwaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppBuildingsPropagationLossModel"));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1400));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (1)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

  // interface 0 is localhost, 1 is the p2p device
  remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // Create building(s)
  Ptr < Building > building;
  building = Create<Building> ();
  building->SetBoundaries (Box (4.0,8.0,
                                  2, 5,
                                  0.0, 6.0));
  building->SetBuildingType (Building::Residential);
  building->SetExtWallsType (Building::ConcreteWithWindows);
  building->SetNFloors (3);
  building->SetNRoomsX (3);
  building->SetNRoomsY (2);
    
  // Create UE, mmWave eNB(s) and LTE eNB(s)
  NodeContainer ueNodes;
  ueNodes.Create (1);

  NodeContainer lteEnbNodes;
  NodeContainer mmWaveEnbNodes;
  NodeContainer allEnbNodes;
  lteEnbNodes.Create (1);
  mmWaveEnbNodes.Create (1);
  allEnbNodes.Add (lteEnbNodes);
  allEnbNodes.Add (mmWaveEnbNodes);

  // Install Mobility Model
  MobilityHelper enbmobility;
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 1.0, 50.0)); // LTE
  enbPositionAlloc->Add (Vector (6.0, 10.0, 4.0)); // mmWave
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (allEnbNodes);
  BuildingsHelper::Install (allEnbNodes);

  MobilityHelper uemobility;
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  uePositionAlloc->Add (Vector (0.0, 0.0, 1.6)); 
  uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  uemobility.SetPositionAllocator (uePositionAlloc);
  uemobility.Install (ueNodes);

  ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));
  BuildingsHelper::Install (ueNodes);

  // Install LTE Devices, mmWave Devices, and McUE Devices to the nodes
  NetDeviceContainer lteEnbDevs = mmwaveHelper->InstallLteEnbDevice (lteEnbNodes);
  NetDeviceContainer mmWaveEnbDevs = mmwaveHelper->InstallEnbDevice (mmWaveEnbNodes);
  NetDeviceContainer ueDevs = mmwaveHelper->InstallMcUeDevice (ueNodes);

  // Install the IP stack on the UEs
  // Assign IP address to UEs, and install applications
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

  mmwaveHelper->AddX2Interface (lteEnbNodes, mmWaveEnbNodes);
  mmwaveHelper->AttachToClosestEnb (ueDevs, mmWaveEnbDevs, lteEnbDevs);

  // Set the default gateway for the UE
  Ptr<Node> ueNode = ueNodes.Get (0);
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  /**
  Create application at UE and 
  sink application at the remote host (Uplink) 
  **/

  // Install and start applications on UEs and remote host
  uint16_t sinkPort = 20000;

  Address sinkAddress (InetSocketAddress (remoteHostAddr, sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (remoteHostContainer.Get (0));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (simStopTime));

  sink = StaticCast<PacketSink> (sinkApps.Get (0));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (ueNodes.Get (0), TcpSocketFactory::GetTypeId ());
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 900, 500000000, DataRate ("1000Mb/s"));
  ueNodes.Get (0)->AddApplication (app);

  // Tracing setup
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-window.txt");
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream1));

  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-data.txt");
  sinkApps.Get (0)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx, stream2));

  Ptr<OutputStreamWrapper> stream3 = asciiTraceHelper.CreateFileStream ("mmWave-tcp-rtt.txt");
  ns3TcpSocket->TraceConnectWithoutContext ("RTT", MakeBoundCallback (&RttChange, stream3));

  // TCP timeout period change
  Ptr<OutputStreamWrapper> stream4 = asciiTraceHelper.CreateFileStream ("rto.txt");
  ns3TcpSocket->TraceConnectWithoutContext ("RTO", MakeBoundCallback (&RtoChange, stream4));

  Ptr<OutputStreamWrapper> stream5 = asciiTraceHelper.CreateFileStream ("segnum.txt");
  ns3TcpSocket->TraceConnectWithoutContext ("NextTxSequence", MakeBoundCallback (&SegnumChange, stream5));

  ueDevs.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

  // Application at UE starts at 0.1 second
  app->SetStartTime (Seconds (0.1));
  app->SetStopTime (Seconds (stopTime));

  Simulator::Schedule (Seconds (1), &ChangeSpeed, ueNodes.Get (0), Vector (30, 0, 0)); // start UE movement after Seconds(0.0)
  Simulator::Schedule (Seconds (0.1), &CalculateThroughput);

  p2ph.EnablePcapAll ("mmwave-sgi-capture");
  BuildingsHelper::MakeMobilityModelConsistent ();

  mmwaveHelper->EnableTraces (); // Enable mmWave traces (most important outputs 
                                  // are in the file RxPacketTrace.txt in ns3-mmwave folder)

  Simulator::Stop (Seconds (simStopTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;

}
