## Project 3
#### Created By: Winny Cheng and Jack Gerulskis

### Preface
This assignment is being submitted late because an extension was given by Prof. Lorenzo. Thank you for your accommodations

### Router functionality

#### TTL

If a packet is sent with a TTL of 1, it will not be fowarded and subsequently dropped. If a file needs two packets to send and has a TTL of 2
only 1 will be fowarded. Etc...

#### Fowarding

Packets are fowarded only if the source and destination ip are part of the fowarding table, otherwise they are dropped. Packets with expiring TTLs
are dropped as well.

### Host Functionality

Host wait to receive until they see tosend.bin. They write to a file with a source IP as the file name and the data is written in binary.
