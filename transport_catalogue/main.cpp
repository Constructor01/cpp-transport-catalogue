#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include "request_handler.h"
#include "map_renderer.h"



void TestAll2() {
    JsonReader json_inf;
    json::Document a = json_inf.ReadJsonInformation();
    transport_catalogue::TransportCatalogue catalogue;
    json_inf.ReadBaseRequests(catalogue, a);
    OutputStatRequests(catalogue, json_inf.ReadStatRequests(a), json_inf.ReadRenderSettings(a), json_inf.ReadRoutingSettings(a));
}

int main() {

    //TestSVG2();
    TestAll2();
    return 0;
}
