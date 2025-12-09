# Intelligence-Handover-Scheme-in-5G-HetNets

2025 Fall SKKU - Computer Networks class

## Abstract

This paper conducts experiments to measure TCP and UDP performance on 5G heterogeneous network (HetNet) environment by setting various numbers of User Equipments (UEs) from 10 to 50 and file sizes from 100B to 500B. The performance is evaluated by packet delay, throughput, inter-packet gap, and packet loss ratio. Additionally, this paper investigates handover recovery time and discusses the ping-pong effect. Experimental results demonstrate that TCP maintains high reliability and stable latency, while UDP shows high loss rate reaching up to 88% as the number of UEs increases. Finally, limitations of TCP and UDP protocols are identified, and QUIC is proposed as an optimized alternative through a literature review, highlighting its potential for reduced handover latency and ping-pong rate. 

## Implementation

- Environment: OMNeT++ & Simu5G framework

- TCP: TCP involves state management on both Receiver and Sender sides to handle connection-oriented communication. 
Receiver Side:
1.	initialize (): Binds the socket to a port and listens for incoming connections. 
2.	handleMessage (): Passes incoming messages to the socket.processMessage() method. This handles TCP state (SYN, ACK, FIN) and triggers specific callbacks, for example, socketDataArrived() function. 
3.	socketDataArrived (): If valid application data is delivered from previous function, then this function calls. In this function, it measures the statistical values, such as packet delay, throughput, inter-packet gap, and packet loss rate. 
4.	finish (): Closes the TCP socket and records final statistical results to csv files. 
Sender Side:
1.	initialize (): Sets up the connection parameters such as destination IP and port. Then, it calls socket.connect() to initiate the 3-way handshake with the receiver.
2.	handleMessage (): Checks if the message is a self-message. If the timer expires and the connection is established, it calls sendPacket() function and reschedules the next transmission event based on the defined sampling time. (In this simulation, the sampling time is 0.1 seconds.)
3.	sendPacket (): Encapsulates the payload into a packet. It sets the sequence ID and embeds the current simulation time. Then, it sends the packet through the TCP socket. 


- UDP: UDP protocol does not manage connection states. 
1.	initialize (): Sets up variables. 
2.	handleMessage (): Checks if the message is a self-message or data packet. If the message is data, then it extracts the file packet, updates statistics, and deletes the packet. 
3.	finish (): Records final statistics to csv files.

## Results

**1. UEs (10 to 50)**

<img width="940" height="265" alt="image" src="https://github.com/user-attachments/assets/5d045c3b-5656-44b8-b490-3c5f120276ce" />
(TCP)

<img width="940" height="291" alt="image" src="https://github.com/user-attachments/assets/a2df918a-2eec-473b-97d8-fe7a844494ed" />
(UDP)

> TCP maintained high stability with 0% packet loss even as UEs increased to 50. This proves that TCP flow control worked effectively. In contrast, UDP performance decreased sharply. As the load increased, the loss rate soared 88%. This indicates that UDP flooded the network, causing severe Congestion Collapse.

**2. Payload size (100B to 500B)**

<img width="940" height="288" alt="image" src="https://github.com/user-attachments/assets/83cd5c63-af3c-40de-9176-f409c9f5cbd7" />
(TCP)

<img width="940" height="277" alt="image" src="https://github.com/user-attachments/assets/86e17aa4-cd03-4a5a-b7d1-fe5e097e4238" />
(UDP)

> For TCP, as the payload size increased, the throughput increased linearly. However, for UDP, larger packets filled the gNB buffers more quickly, worsening the congestion and leading to higher packet loss.

**3. Handover Analysis**

<img width="875" height="316" alt="image" src="https://github.com/user-attachments/assets/6ba84db5-fe7a-436e-b283-24790aa85933" />
(TCP)

<img width="884" height="288" alt="image" src="https://github.com/user-attachments/assets/c3783800-d3ce-4d39-a91a-fe03ffeb4c6b" />
(UDP)


> There is distinct spike in the graph. This "silence period" represents the actual handover latency. The Ping-Pong Rate graph shows multiple oscillating drops in throughput. This "W-shaped" pattern visually proves that the UE was repeatedly switching between cells, confirming the Ping-Pong effect.

