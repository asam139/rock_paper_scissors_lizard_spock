//
// Created by Saul Moreno Abril on 30/06/2018.
//

#include "SocketAddress.h"


static SocketAddressPtr CreateIPv4FromString(const std::string& in) {
    const auto pos = in.find_last_of(':');
    std::string host, service;
    if (pos != std::string::npos) {
        host = in.substr(0, pos);
        service = in.substr(pos + 1);
    } else {
        host = in;
        service = "0"; //use default port...
    }
    addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    addrinfo *list;
    const int error = getaddrinfo(host.c_str(), service.c_str(), &hint, &list);
    addrinfo *list_head = list;
    if (error != 0 && list != nullptr) {
        freeaddrinfo(list_head);
        return nullptr;
    }
    while (!list->ai_addr && list->ai_next) {
        list = list->ai_next;
    }
    if (!list->ai_addr) {
        freeaddrinfo(list_head);
        return nullptr;
    }
    auto result = std::make_shared<SocketAddress>(*list->ai_addr);
    freeaddrinfo(list_head);
    return result;
}