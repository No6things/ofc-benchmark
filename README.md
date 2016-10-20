# OpenFlow Controller Benchmarking Tool

### Authors: Cavadia. Alberto & Tovar. Daniel

The objective is to develop a more extensive  benchmarking tool for SDN
controllers under OF 1.3, with C, than the existing tool Cbench.

#### Features

1. Metrics:

	a. RTT.

	b. CPU usage percent.

	c. Memory usage.

	d. Packets sent.

	e. Packets received.

	f. Packets processed.

	g. Number of flows.

2. Parameters:

	a. Number of switches.

	b. Number of packets.

	c. Size of packet.

	d. Interpacket delay sending.

	e. Packet fields.

	f. Number of packets to configure.

	g. Sending order:

		i. Serial.

		ii. Random.

3. Controllers supported:

	a. Ryu.

	b. FloodLight.

	c. OpenDayLight.

	d. OpenMUL.

4. Virtual Switch supported:

	a. Open vSwitch.

	b. Mininet.

#### Requirements

1. SNMP-NET installed.
2. SNMP-MIBS-DOWNLOADER installed.
	 1. Comment out "mibs:" from snmp.conf.
	 2. Run download-mibs.
3. Update snmpd.conf to accept foreigner requests:
	 1. Update to agentAddress udp:161.
	 2. Remove "-V systemonly" from "rocommunity public default -V systemonly".
