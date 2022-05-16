#include <iostream>
#include <pcap.h>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

bool bridge_up=false;
int pkt_num = 1;

pcap_t *handler;
string IP;
char errbuf[PCAP_ERRBUF_SIZE];
string filter_exp;
bpf_u_int32 subnet_mask, ip;

struct grehdr
{   
    u_int16_t op;
    u_int16_t ether_type;
    u_int32_t key;
};

void call_back_func(u_char *args, const struct pcap_pkthdr *packet_header, const u_char *packet){
    struct ether_header *sniff_ethernet, *sniff_ethernet_inside;
    struct ip *sniff_ip;
    unsigned short ether_type, ether_type_inside;
    
    printf("Packet Num [%d]\n",pkt_num);

    for(int j=0;j<packet_header->len;j++){
        printf((j%2 ? "%02X " : "%02X"), (unsigned char) packet[j]);
        if(j%16==15){
            cout<<endl;
        }
    }
    cout<<endl;
    // Outer Eth header
    sniff_ethernet = (struct ether_header *)packet;
    printf("Outer Ethernet Header Src MAC:  ");
    for (int i=0; i<ETHER_ADDR_LEN; i++){
        printf((i == ETHER_ADDR_LEN -1 ? "%02X\n" : "%02X:"), sniff_ethernet->ether_shost[i]);
    }
    printf("Outer Ethernet Header Dst MAC:  ");
    for (int i=0; i<ETHER_ADDR_LEN; i++){
        printf((i == ETHER_ADDR_LEN -1 ? "%02X\n" : "%02X:"), sniff_ethernet->ether_dhost[i]);
    }

    // Outter Eth protocol type.
    ether_type = ntohs(sniff_ethernet->ether_type);
    printf("Outer Ether Type: 0x%04X\n",ether_type);
    packet += sizeof(struct ether_header);

    // Outer IP header
    if (ether_type == ETHERTYPE_IP) {
        sniff_ip = (struct ip *)packet;
        packet+=sizeof(struct ip);
        printf("Outer IP Header Src IP:  %s\n", inet_ntoa(sniff_ip->ip_src));
        printf("Outer IP Header Dst IP:  %s\n", inet_ntoa(sniff_ip->ip_dst));
        printf("next protocol number : %d  (47 stands for GRE protocol)\n",int(sniff_ip->ip_p));

    }

    // GRE header
    packet+=2;
    const char t1 = (unsigned char) *packet;
    packet++;
    const char t2 = (unsigned char) *packet;
    packet++;
    printf("GRE Header Protocol type:  0x%02X%02X\n",t1,t2);

    // Inner Ethernet header
    sniff_ethernet_inside = (struct ether_header *)packet;
    printf("Inner Ethernet Header Src MAC:  ");
    for (int i=0; i<ETHER_ADDR_LEN; i++){
        printf((i == ETHER_ADDR_LEN -1 ? "%02X\n" : "%02X:"), sniff_ethernet_inside->ether_shost[i]);
    }
    printf("Inner Ethernet Header  Dst MAC:  ");
    for (int i=0; i<ETHER_ADDR_LEN; i++){
        printf((i == ETHER_ADDR_LEN -1 ? "%02X\n" : "%02X:"), sniff_ethernet_inside->ether_dhost[i]);
    }
    // Inner Ethernet protocol type
    ether_type_inside = ntohs(sniff_ethernet_inside->ether_type);
    printf("Inner Ether Type: 0x%04X\n",ether_type_inside);
    // Dynamically create Tunnel 
    char* tmp_IP = inet_ntoa(sniff_ip->ip_src);
    string srcIP(tmp_IP);
    tmp_IP = inet_ntoa(sniff_ip->ip_dst);
    string dstIP(tmp_IP);
    //cmd1 docker exec BRGr ip link add GRE1 type gretap remote 140.114.0.1 local 140.113.0.1
    //cmd2 docker exec BRGr ip link set GRE1 up
    //cmd3 docker exec BRGr ip link show GRE1
    //   docker exec BRGr ip link add br0 type bridge
    //cmd4 docker exec BRGr ip link set BRGrGWrveth master br0
    //cmd5 docker exec BRGr ip link set GRE1 master br0
    //   docker exec BRGr ip link set br0 up
    string cmd1 = "ip link add GRE"+ std::to_string(pkt_num)+"_new type gretap remote "+ srcIP.c_str() +" local "+dstIP.c_str();
    FILE * fopen = popen(cmd1.c_str(),"r");
    pclose(fopen);
    string cmd2 = "ip link set GRE"+std::to_string(pkt_num)+"_new up";
    fopen = popen(cmd2.c_str(),"r");
    pclose(fopen);
    string cmd3 = "ip link show GRE" + std::to_string(pkt_num)+"_new";
    fopen = popen(cmd2.c_str(),"r");
    pclose(fopen);
    string t = "ip link add br0 type bridge";
    fopen = popen(t.c_str(),"r");
    fclose(fopen);
    string cmd4 ="ip link set BRGrGWrveth master br0";
    fopen = popen(cmd2.c_str(),"r");
    pclose(fopen);    
    string cmd5 = "ip link set GRE"+ std::to_string(pkt_num)+"_new master br0";
    fopen = popen(cmd5.c_str(),"r");
    pclose(fopen);
    t = "ip link set br0 up";
    fopen = popen(t.c_str(),"r");
    fclose(fopen);
    bridge_up = true;

    // Update filter expression
    filter_exp += " && host not "+srcIP;
    struct bpf_program filter;
    if (pcap_compile(handler, &filter, filter_exp.c_str(), 0, ip) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp.c_str(), pcap_geterr(handler));
        exit(1);
    }
    if (pcap_setfilter(handler, &filter) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp.c_str(), pcap_geterr(handler));
        exit(1);
    }
    cout<<"Tunnel finish!"<<endl;
    cout<<endl;
    pkt_num++;
}

int main(){
    // List all the interface and get the interface selected by the user

    vector<string> interface_list;
    interface_list.clear();
    int cnt=0;
    pcap_if_t *interface = NULL;

    // Get the interface list
    if(pcap_findalldevs(&interface, errbuf) == -1) {
        fprintf(stderr, "error when findalldevs: %s\n", errbuf);
        return(2);
    }

    // iterate through the interface list
    for(pcap_if_t *it = interface ; it != NULL ; it = it->next) {
        cout<<cnt<<" Name: "<<it->name<<endl;
        string s(it->name);
        interface_list.push_back(s);
        cnt++;
    }

    // Let user select the device
    int interface_num=0;
    string input;
    cout<<"Insert a number to select interface"<<endl;
    getline(cin, input);
    interface_num = atoi(input.c_str());
    string selected = interface_list[interface_num];

    // Listen on a particular device
    handler = pcap_open_live(selected.c_str(), BUFSIZ, 1, 0, errbuf);
    if(handler == NULL){
        fprintf(stderr, "pcap_open_live(): %s\n", errbuf);
        return(2);
    }
    cout<<"start listening at $"<<selected<<endl;

    // get ip
    if(pcap_lookupnet(selected.c_str(), &ip, &subnet_mask, errbuf) == -1){
        fprintf(stderr, "pcap_lookupnet(): %s\n", errbuf);
        return(2);
    }
    IP="140.113.0.1";
    // Input the filter expression
    cout<<"Insert BPF filter expression:"<<endl;
    getline(cin, filter_exp);
    
    // Compile the filter expression and set the filter
    struct bpf_program fp;
    if (pcap_compile(handler, &fp, filter_exp.c_str(), 0, ip) == -1) {
	    fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp.c_str(), pcap_geterr(handler));
        return(2);
    }
    if (pcap_setfilter(handler, &fp) == -1) {
	    fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp.c_str(), pcap_geterr(handler));
        return(2);
    }else{
        cout<<"applying filter: "<<filter_exp<<endl;
    }
    printf("\n");
    cout<<"waiting for packets..."<<endl;
    printf("\n\n");
    // Process the packets
    if(pcap_loop(handler, 100, call_back_func, NULL) == -1){
        fprintf(stderr, "pcap_loop(): %s\n", pcap_geterr(handler));
    }

    return 0;
}   