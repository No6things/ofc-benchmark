# OpenFlow Controller Benchmarking Tool

### Authors: Cavadia. Alberto & Tovar. Daniel

The objective is to develop a more powerful benchmarking tool for SDN
controllers under OF 1.3, with C, than existing tools as Cbench.

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
	d. Inter-time of packets sending.
	e. Packet fields.
	f. Number of packets to configure.
	g. Sending order:
		i. Serial.
		ii. Random.

3. Controllers supported:
	a. Ryu.
	b. Floodlight.
	c. OpenDayLight.
	d. OpenMUL.

4. Virtual Switch supported:
	a. Open vSwitch.
	b. Mininet.
