#pragma once
#include "json_reader.h"
#include "transport_router.h"
void OutputStatRequests(transport_catalogue::TransportCatalogue& catalogue, std::vector<json::Node> doc_inf, RenderSettings settings_,std::pair<int,double> bus_setting);
