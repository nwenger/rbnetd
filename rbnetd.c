#include <stdio.h> //XXX
#include <time.h>
#include <iwlib.h>

//getifaddrs
#include <sys/types.h>
#include <ifaddrs.h>

//netdevice
//#include <sys/ioctl.h>
//#include <net/if.h>

int scan_intf(char *intf, wireless_scan *result)
{
    wireless_scan_head head;
    iwrange range;
    int sockd;

    sockd = iw_sockets_open();

    if (iw_get_range_info(sockd, intf, &range) < 0) {
        printf("Error during iw_get_range_info\n");
        return 2;
    }

    if (iw_scan(sockd, intf, range.we_version_compiled, &head) < 0) {
        printf("Error during iw_scan\n");
        return 2;
    }

    result = head.result;
    return 0;
}

int main(int argc, char *argv[])
{

    int inet_type = AF_INET; //IPv4, not 6
    struct ifaddrs *intfs;
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

    int rv = scan_intf(intfs->ifa_name, scan_result);
    if (!rv) {
        printf("Scan of interface %s failed\n", intfs->ifa_name);
        exit(rv);
    }
    while(scan_result != NULL)
    {
        printf("%s\n", scan_result->b.essid);
        scan_result = scan_result->next;
    }
    freeifaddrs(intfs);
    return 0;
}
