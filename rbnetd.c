#include <stdio.h> //XXX
#include <time.h>
#include <iwlib.h>

//getifaddrs
#include <sys/types.h>
#include <ifaddrs.h>

//netdevice
//#include <sys/ioctl.h>
//#include <net/if.h>


//TODO next steps
//get the right interface
//bring interface up
//  https://stackoverflow.com/questions/5858655/linux-programmatically-up-down-an-interface-kernel
//I can get a list of network names
//detect if the network needs a key
//prompt for a key
//confirm I can connect to a network using key
//either use DHCP or manually acquire an IP address
//config file
//write function to connect via ethernet

//add flags for:
//IPv4 vs IPv6
//specify an interface to use

//commands:
//conn/disc [ESSID]
//list


int scan_intf(int sockd, char *intf, wireless_scan_head *head)
{
    iwrange range;

    if (iw_get_range_info(sockd, intf, &range) < 0) {
        printf("Error during iw_get_range_info\n");
        return 2;
    }

    if (iw_scan(sockd, intf, range.we_version_compiled, head) < 0) {
        printf("Error during iw_scan\n");
        return 2;
    }

    return 0;
}

//FIXME this function sucks
int get_net(int sockd, struct ifaddrs *intfs, wireless_scan_head *scan_head, wireless_scan* result, char *str)
{
    int search = strcmp("",str);
    printf("Scanning interface %s\n", intfs->ifa_name);
    int rv = scan_intf(sockd, intfs->ifa_name, scan_head);
    if (rv) {
        printf("Scan of interface %s failed\n", intfs->ifa_name);
        return rv;
    }
    result = scan_head->result;

    while(result != NULL)
    {
        if(!search)
            printf("%s\n", result->b.essid);
        else if(!strcmp(str,result->b.essid)) //need to set scan_result
            return 0;
        result = result->next;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int sockd;
    //int inet_type = AF_INET; //IPv4, not 6
    struct ifaddrs *intfs;
    wireless_scan_head *scan_head = (wireless_scan_head*)malloc(sizeof(wireless_scan_head*));
    wireless_scan *scan_result;

    //check that interfaces are up
    sockd = iw_sockets_open();

    if (getifaddrs(&intfs) < 0) {
        printf("FAILED TO GET INTERFACE LIST\n");
        return 1;
    }

    while (intfs->ifa_next != NULL) {
        if(!strcmp("wlp1s0",intfs->ifa_name)) //FIXME hardcoded
            break;
        intfs = intfs->ifa_next;
    }
    //TODO safety checks to ensure current intfs node is what we want to use

    if(get_net(sockd,intfs,scan_head,scan_result,"Leps")) //FIXME hardcoded
    {
        printf("FAILED TO LIST NETWORKS\n");
        return 2;
    }

    if (scan_result->has_ap_addr) {
        if (connect(sockd, (const struct sockaddr*)&scan_result->ap_addr, sizeof(scan_result->ap_addr)) < 0) {
            printf("Connect to network:%s failed\n", scan_result->b.essid);
            return 3;
        }
    }

    return 0;
}
