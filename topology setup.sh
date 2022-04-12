#!/bin/bash
<<comment
sudo docker run --privileged --cap-add NET_ADMIN --cap-add NET_BROADCAST -d -it --name h1 lab4_v2
sudo docker run --privileged --cap-add NET_ADMIN --cap-add NET_BROADCAST -d -it --name h2 lab4_v2
sudo docker run --privileged --cap-add NET_ADMIN --cap-add NET_BROADCAST -d -it --name R1 lab4_v2
sudo docker run --privileged --cap-add NET_ADMIN --cap-add NET_BROADCAST -d -it --name R2 lab4_v2
sudo docker run --privileged --cap-add NET_ADMIN --cap-add NET_BROADCAST -d -it --name BRG1 lab4_v2
sudo docker run --privileged --cap-add NET_ADMIN --cap-add NET_BROADCAST -d -it --name BRG2 lab4_v2
sudo docker run --privileged --cap-add NET_ADMIN --cap-add NET_BROADCAST -d -it --name BRGr lab4_v2
sudo docker run --privileged --cap-add NET_ADMIN --cap-add NET_BROADCAST -d -it --name GWr lab4_v2
comment

#h1 BRG1
sudo ip link add h1BRG1veth type veth peer name BRG1h1veth
sudo ip link set h1BRG1veth netns $(docker inspect -f {{.State.Pid}} h1)
sudo ip link set BRG1h1veth netns $(docker inspect -f {{.State.Pid}} BRG1)
docker exec h1 ip addr add 10.0.1.1/24 dev h1BRG1veth
docker exec h1 ip link set h1BRG1veth up
docker exec BRG1 ip link set BRG1h1veth up

#h2 BRG2
sudo ip link add h2BRG2veth type veth peer name BRG2h2veth
sudo ip link set h2BRG2veth netns $(docker inspect -f {{.State.Pid}} h2)
sudo ip link set BRG2h2veth netns $(docker inspect -f {{.State.Pid}} BRG2)
docker exec h2 ip addr add 10.0.1.2/24 dev h2BRG2veth
docker exec h2 ip link set h2BRG2veth up
docker exec BRG2 ip link set BRG2h2veth up

#GWr BRGr
sudo ip link add GWrBRGrveth type veth peer name BRGrGWrveth
sudo ip link set GWrBRGrveth netns $(docker inspect -f {{.State.Pid}} GWr)
sudo ip link set BRGrGWrveth netns $(docker inspect -f {{.State.Pid}} BRGr)
docker exec GWr ip addr add 10.0.1.254/24 dev GWrBRGrveth
docker exec BRGr ip addr add 10.0.1.5/24 dev BRGrGWrveth
docker exec GWr ip link set GWrBRGrveth up
docker exec BRGr ip link set BRGrGWrveth up

#R1 BRG1

sudo ip link add R1BRG1veth type veth peer name BRG1R1veth
sudo ip link set R1BRG1veth netns $(docker inspect -f {{.State.Pid}} R1)
sudo ip link set BRG1R1veth netns $(docker inspect -f {{.State.Pid}} BRG1)
docker exec R1 ip addr add 140.114.0.2/16 dev R1BRG1veth
docker exec BRG1 ip addr add 140.114.0.1/16 dev BRG1R1veth
docker exec R1 ip link set R1BRG1veth up
docker exec BRG1 ip link set BRG1R1veth up

#R1 BRG2
sudo ip link add R1BRG2veth type veth peer name BRG2R1veth
sudo ip link set R1BRG2veth netns $(docker inspect -f {{.State.Pid}} R1)
sudo ip link set BRG2R1veth netns $(docker inspect -f {{.State.Pid}} BRG2)
docker exec R1 ip addr add 140.115.0.2/16 dev R1BRG2veth
docker exec BRG2 ip addr add 140.115.0.1/16 dev BRG2R1veth
docker exec R1 ip link set R1BRG2veth up
docker exec BRG2 ip link set BRG2R1veth up


#R2 BRGr
sudo ip link add R2BRGrveth type veth peer name BRGrR2veth
sudo ip link set R2BRGrveth netns $(docker inspect -f {{.State.Pid}} R2)
sudo ip link set BRGrR2veth netns $(docker inspect -f {{.State.Pid}} BRGr)
docker exec R2 ip addr add 140.113.0.2/16 dev R2BRGrveth
docker exec BRGr ip addr add 140.113.0.1/16 dev BRGrR2veth
docker exec R2 ip link set R2BRGrveth up
docker exec BRGr ip link set BRGrR2veth up

#R1 R2
sudo ip link add R1R2veth type veth peer name R2R1veth
sudo ip link set R1R2veth netns $(docker inspect -f {{.State.Pid}} R1)
sudo ip link set R2R1veth netns $(docker inspect -f {{.State.Pid}} R2)
docker exec R1 ip addr add 20.0.0.1/16 dev R1R2veth
docker exec R2 ip addr add 20.0.0.2/16 dev R2R1veth
docker exec R1 ip link set R1R2veth up
docker exec R2 ip link set R2R1veth up
