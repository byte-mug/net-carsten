#include <stdio.h>
#include <netpkt/pkt.h>
#include <netipv4/ipv4.h>
#include <netipv4/defs.h>
#include <netipv6/ipv6.h>
#include <netipv6/defs.h>

void main(){
	ipv6_addr_t loop = IP6_ADDR_LOOPBACK_INIT;
	ipv6_addr_t test1 = IP6_ADDR_NODELOCAL_ALLNODES_INIT;
	ipv4_addr_t lp = IP4_ADDR_LINK_LOCAL_PREFIX;
	printf("Hello World %d\n",(int)(IP6ADDR_EQ(loop,test1)));
	printf("Hello World %d\n",(int)(IP6ADDR_EQ(loop,loop)));
}