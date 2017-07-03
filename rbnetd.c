#include <stdio.h> //XXX
#include <time.h>
#include <iwlib.h>

//getifaddrs
#include <sys/types.h>
#include <ifaddrs.h>

//netdevice
//#include <sys/ioctl.h>
//#include <net/if.h>

//https://stackoverflow.com/questions/5858655/linux-programmatically-up-down-an-interface-kernel
//bring interface up

//TODO next steps
//confirm I can get a list of network names
//confirm I can connect to a network
//connect to a network using WPA2
//either use DHCP or manually acquire an IP address
//write function to connect via ethernet

//add flags for:
//IPv4 vs IPv6
//specify an interface to use


//XXX this only gets networks i'm connected to
//...not really what i want at all
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

int main(int argc, char *argv[])
{
    int sockd;
    int inet_type = AF_INET; //IPv4, not 6
    struct ifaddrs *intfs;
    wireless_scan_head *scan_head = (wireless_scan_head*)malloc(sizeof(wireless_scan_head*));
    wireless_scan *scan_result;

    if (getifaddrs(&intfs) < 0) {
        printf("FAILED TO GET INTERFACE LIST\n");
        return 1;
    }
    while (intfs->ifa_next != NULL)
    {
        if (!(intfs->ifa_flags & IFF_LOOPBACK) &&
           intfs->ifa_addr->sa_family == inet_type) {
            break;
        }
        intfs = intfs->ifa_next;
    }

    //TODO safety checks to ensure current intfs node is what we want to use
    // exit otherwise

    sockd = iw_sockets_open();

    printf("Scanning interface %s\n", intfs->ifa_name);
    int rv = scan_intf(sockd, intfs->ifa_name, scan_head);
    if (rv) {
        printf("Scan of interface %s failed\n", intfs->ifa_name);
        exit(rv);
    }
    scan_result = scan_head->result;

    while(scan_result != NULL)
    {
        printf("%s\n", scan_result->b.essid);
        scan_result = scan_result->next;
    }
    //freeifaddrs(intfs); //segfault
    /*
    if (scan_result->has_ap_addr) {
        if (connect(sockfd, scan_result->ap_addr, sizeof(scan_result->ap_addr)) < 0) {
            printf("Connect to network:%s failed\n", scan_result->b.essid);
            exit(3);
        }
    }
    */
    return 0;
}
