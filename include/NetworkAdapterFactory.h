#ifndef NETWORKADAPTERFACTORY_H
#define NETWORKADAPTERFACTORY_H

#include <vector>
#include "NetworkAdapter.h"
#include "bstr.h"

void QueryNetworkAdapters(std::string query, std::vector<NetworkAdapter*> &vAdapters);

#endif