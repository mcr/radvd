
#include "test/getaddrs.h"

#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

static struct ifaddrs * reverse_list(struct ifaddrs * list)
{
	struct ifaddrs * prev = 0;
	struct ifaddrs * cur = list;

	while (cur) {
		struct ifaddrs * next = cur->ifa_next;
		cur->ifa_next = prev;
		prev = cur;
		cur = next;
	}
	return prev;
}

static char const * addrs = 0;

void setaddrs(char const * file)
{
	addrs = file;
}

int getaddrs(struct ifaddrs **ifap)
{
	struct ifaddrs * retval = 0;

	FILE * in = fopen(addrs, "r");
	if (in) {
		do {
			char name[100];
			int type;
			char addr_str[INET6_ADDRSTRLEN];
			int int_mask;

			char line[1024];
			char * s = fgets(line, sizeof(line), in);
			if (s) {
				int count = sscanf(s, " %s %d %s %d", name, &type, addr_str, &int_mask);
				if (count >= 3) {
					struct ifaddrs * ifa = malloc(sizeof(struct ifaddrs));

					memset(ifa, 0, sizeof(struct ifaddrs));

					ifa->ifa_name = strdup(name);
					switch (type) {
					case 4: {
						struct sockaddr_in * in;

						in = malloc(sizeof(struct sockaddr_in));

						memset(in, 0, sizeof(struct sockaddr_in));

						in->sin_family = AF_INET;

						assert(inet_pton(AF_INET, addr_str, &in->sin_addr));

						ifa->ifa_addr = (struct sockaddr *)in;
					}break;

					case 6: {
						struct sockaddr_in6 * in6;
						struct sockaddr_in6 * mask;

						in6 = malloc(sizeof(struct sockaddr_in6));
						mask = malloc(sizeof(struct sockaddr_in6));

						memset(in6, 0, sizeof(struct sockaddr_in6));
						memset(mask, 0, sizeof(struct sockaddr_in6));

						in6->sin6_family = AF_INET6;
						mask->sin6_family = AF_INET6;

						assert(inet_pton(AF_INET6, addr_str, &in6->sin6_addr));
						if (int_mask == 64) {
							assert(inet_pton(AF_INET6, "ffff:ffff:ffff:ffff::", &mask->sin6_addr));
						} else {
							assert(0);
						}

						ifa->ifa_addr = (struct sockaddr *)in6;
						ifa->ifa_netmask = (struct sockaddr *)mask;
					}break;

					default:
						abort();
					}

					ifa->ifa_next = retval;
					retval = ifa;
				}
			}
		} while (!feof(in));
		fclose(in);
	}

	retval = reverse_list(retval);
#if 0
	for (struct ifaddrs * ifa2 = retval; ifa2; ifa2 = ifa2->ifa_next) {
		char dst[INET6_ADDRSTRLEN];
		if (ifa2->ifa_addr->sa_family == AF_INET6) {
			char mask[INET6_ADDRSTRLEN];
			struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa2->ifa_addr;
			inet_ntop(AF_INET6, &s6->sin6_addr, dst, sizeof(dst));
			printf("%s 6 %s 64\n", ifa2->ifa_name, dst);
		}
		else if (ifa2->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *s = (struct sockaddr_in *)ifa2->ifa_addr;
			inet_ntop(AF_INET, &s->sin_addr, dst, sizeof(dst));
			printf("%s 4 %s\n", ifa2->ifa_name, dst);
		}
	}
#endif

	*ifap = retval;

	return 0;
}

void freeaddrs(struct ifaddrs *ifa)
{
	struct ifaddrs * ifa2 = ifa;
	while (ifa2) {
		struct ifaddrs * next = ifa2->ifa_next;
		if (ifa2->ifa_addr->sa_family == AF_INET6) {
			free(ifa2->ifa_netmask);
		}
		free(ifa2->ifa_addr);
		free(ifa2->ifa_name);
		free(ifa2);
		ifa2 = next;
	}
}


