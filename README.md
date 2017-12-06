# OpenFlow Controller Benchmarking Tool

### Authors: Cavadia. Alberto & Tovar. Daniel

The objective is to develop a more extensive  benchmarking tool for SDN
controllers under OF 1.3, with C, than the existing tool Cbench.

NOTE: This tool was based on OFLOPS/cbench, so it has all the current options supported by them.

#### Features

1. Metrics for latency and performance tests:

	a. RTT.
	
	b. CPU usage percent.
	
	c. Memory usage.
	
	d. Packets sent.
	
	e. Packets received.
	
	f. Number of flows.

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

#### Requirements


1. SNMP-NET installe
2. SNMP-MIBS-DOWNLOADER installed.
	 1. Comment out "mibs:" from snmp.conf.
	 2. Run download-mibs.
3. Update snmpd.conf to accept foreigner requests:
	 1. Update to agentAddress udp:161.
	 2. Remove "-V systemonly" from "rocommunity public default -V systemonly".
4. Install gnuplot on your OS with local package install manager. e.g: sudo zypper install gnuplot in OpenSuse

#### Comments

1. The entire description of options is displayed with -h or --help flag.
2. I'will try to host a couple of VM (master and slave) to make testing easier.

#### Instructions

1. Generate an excutable running the makefile inside.
2. Run the Master node with statements indicating controller IP address, port and amount of nodes, similar to:
	./ofcB -C 192.168.1.108 -P 6653 -N 2
3. Run the Slave node(s) with statements indicating in addition Master Node IP address and options for actual test similar to:
	./ofcB -C 192.168.1.108 -P 6653 -s 16 -N 2 -n 192.168.1.109 -l 50





