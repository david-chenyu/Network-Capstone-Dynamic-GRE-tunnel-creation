#!/bin/bash
<<comment
need to enable router forwarding at the first time
vim /etc/sysctl.conf
sysctl -p
comment

sudo docker exec -it h1 bash
ip route del default
ip route add default via 10.0.1.254
exit

sudo docker exec -it h2 bash
ip route del default
ip route add default via 10.0.1.254
exit

sudo docker exec -it R1 bash
ip route del default
ip route add 140.113.0.0/16 via 20.0.0.2
exit

sudo docker exec -it R2 bash
ip route del default
ip route add 140.114.0.0/16 via 20.0.0.1
ip route add 140.115.0.0/16 via 20.0.0.1
exit

sudo docker exec -it BRG1 bash

ip route del default
ip route add default via 140.114.0.2
exit

sudo docker exec -it BRG2 bash
ip route del default
ip route add default via 140.115.0.2
exit

sudo docker exec -it BRGr bash
ip route del default
ip route add default via 140.113.0.2
exit


