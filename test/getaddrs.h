
#pragma once

#include <sys/types.h>
#include <ifaddrs.h>

void setaddrs(char const * file);
int getaddrs(struct ifaddrs **ifap);
void freeaddrs(struct ifaddrs *ifa);
