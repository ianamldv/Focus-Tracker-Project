#pragma once

#include <WebServer.h>

extern WebServer server;

void initWeb();       // Connect Wi-Fi, NTP, register routes, start server
void printIP();       // Show IP on Serial (called after display is ready)