#!/bin/bash
#GRE BRG1 BRGr
docker exec BRG1 ip link add GRE1 type gretap remote 140.113.0.1 local 140.114.0.1
docker exec BRGr ip link add GRE1 type gretap remote 140.114.0.1 local 140.113.0.1

docker exec BRG1 ip link set GRE1 up
docker exec BRG1 ip link show GRE1
docker exec BRGr ip link set GRE1 up
docker exec BRGr ip link show GRE1

docker exec BRG1 ip link add br0 type bridge
docker exec BRG1 ip link set BRG1h1veth master br0
docker exec BRG1 ip link set GRE1 master br0
docker exec BRG1 ip link set br0 up

docker exec BRGr ip link add br0 type bridge
docker exec BRGr ip link set BRGrGWrveth master br0
docker exec BRGr ip link set GRE1 master br0
docker exec BRGr ip link set br0 up

#GRE BRG2 BRGr
docker exec BRG2 ip link add GRE2 type gretap remote 140.113.0.1 local 140.115.0.1
docker exec BRGr ip link add GRE2 type gretap remote 140.115.0.1 local 140.113.0.1

docker exec BRG2 ip link set GRE2 up
docker exec BRG2 ip link show GRE2
docker exec BRGr ip link set GRE2 up
docker exec BRGr ip link show GRE2

docker exec BRG2 ip link add br0 type bridge
docker exec BRG2 ip link set BRG2h2veth master br0
docker exec BRG2 ip link set GRE2 master br0
docker exec BRG2 ip link set br0 up

docker exec BRGr ip link add br0 type bridge
docker exec BRGr ip link set BRGrGWrveth master br0
docker exec BRGr ip link set GRE2 master br0
docker exec BRGr ip link set br0 up

