#include "request_handler.h"


void OutputStatRequests(transport_catalogue::TransportCatalogue& catalogue, std::vector<json::Node> doc_inf, RenderSettings settings_, std::pair<int, double> bus_setting) {
    std::vector<json::Node> correct_requests;

    transport_router::TransportRouter::RoutingSettings setting_bus_;
    setting_bus_.wait_time = bus_setting.first;
    setting_bus_.velocity = bus_setting.second* transport_router::KMH_TO_MMIN;
    transport_router::TransportRouter router_graph(catalogue, setting_bus_);
    //std::cout << "Settings bus "<<"\nSpeed " << router_graph.GetSettings().velocity << "\nTime " <<router_graph.GetSettings().wait_time << std::endl;
    for (auto& node_inf : doc_inf) {

        //если запрос это остановка
        if (*(&(&node_inf.AsMap())->at("type").AsString()) == "Stop") {
            std::string name = *(&(&node_inf.AsMap())->at("name").AsString());
            try {
                catalogue.GetBusesOnStop(name);
                std::vector<json::Node> all_buses;
                for (auto& bus : catalogue.GetBusesOnStop(name)) {
                    all_buses.push_back(json::Builder{}.Value(static_cast<std::string>(bus)).Build());
                }
                correct_requests.push_back(json::Builder{}.StartDict().
                    Key("buses").Value(std::move(all_buses)).
                    Key("request_id").Value((&node_inf.AsMap())->at("id").AsInt()).
                    EndDict().Build());
            }
            catch (...) {
                correct_requests.push_back(json::Builder{}.StartDict().
                    Key("error_message").Value("not found").
                    Key("request_id").Value((&node_inf.AsMap())->at("id").AsInt()).
                    EndDict().Build());
            }
        }

        //если запрос это автобус
        if (*(&(&node_inf.AsMap())->at("type").AsString()) == "Bus") {
            std::string name = *(&(&node_inf.AsMap())->at("name").AsString());
            try {
                catalogue.GetRouteInfo(name);
                correct_requests.push_back(json::Builder{}.StartDict().
                    Key("request_id").Value((&node_inf.AsMap())->at("id").AsInt()).
                    Key("curvature").Value(catalogue.GetRouteInfo(name).curvature).
                    Key("route_length").Value(catalogue.GetRouteInfo(name).route_length).
                    Key("stop_count").Value(catalogue.GetRouteInfo(name).num_of_stops).
                    Key("unique_stop_count").Value(catalogue.GetRouteInfo(name).num_of_unique_stops).
                    EndDict().Build());
            }
            catch (...) {
                correct_requests.push_back(json::Builder{}.StartDict().
                    Key("error_message").Value("not found").
                    Key("request_id").Value((&node_inf.AsMap())->at("id").AsInt()).
                    EndDict().Build());
            }
        }

        //если запрос это параметры карты
        if (*(&(&node_inf.AsMap())->at("type").AsString()) == "Map") {
            RenderSettings rend_set = settings_;
            std::ostringstream ss;
            MapRenderer map_rend;
            map_rend.SetSettings(rend_set);
            svg::Document svg_doc = map_rend.RenderMap(catalogue);
            svg_doc.Render(ss);
            correct_requests.push_back(json::Builder{}.StartDict().
                Key("request_id").Value((&node_inf.AsMap())->at("id").AsInt()).
                Key("map").Value(ss.str()).
                EndDict().Build());
        }
        if (*(&(&node_inf.AsMap())->at("type").AsString()) == "Route") {
            std::string from_stop = *(&(&node_inf.AsMap())->at("from").AsString());
            std::string to_stop= *(&(&node_inf.AsMap())->at("to").AsString());
            std::optional<std::vector<transport_router::TransportRouter::RouterEdge>> marshrut=router_graph.BuildRoute(from_stop, to_stop);
            //std::cout << "Otvet marshruta "<< (&node_inf.AsMap())->at("id").AsInt() << std::endl;
            std::vector<json::Node> all_rout;

            if (marshrut.has_value()) {
                double total_time = 0;
                for (auto znak : marshrut.value()) {
                    //std::cout << "{" << std::endl;
                    //std::cout << znak.stop_from << "\n" << setting_bus_.wait_time<<"\n" << znak.bus_name << "\n" << znak.span_count << "\n" << znak.total_time << "\n";
                    //std::cout << znak.bus_name << "\n" << znak.span_count << "\n" << znak.total_time << "\n";
                    //std::cout << "}" << std::endl;
                    all_rout.push_back(json::Builder{}.StartDict().
                        Key("stop_name").Value(static_cast<std::string>(znak.stop_from)).
                        Key("time").Value(setting_bus_.wait_time).
                        Key("type").Value("Wait").EndDict().Build());
                    all_rout.push_back(json::Builder{}.StartDict().
                        Key("bus").Value(static_cast<std::string>(znak.bus_name)).
                        Key("span_count").Value(znak.span_count).
                        Key("time").Value(znak.total_time - setting_bus_.wait_time).
                        Key("type").Value("Bus").EndDict().Build());
                    total_time += znak.total_time;
                }
                
                correct_requests.push_back(json::Builder{}.StartDict().
                    Key("items").Value(all_rout).
                    Key("request_id").Value((&node_inf.AsMap())->at("id").AsInt()).
                    Key("total_time").Value(total_time).
                    EndDict().Build());
            }
            else {
                correct_requests.push_back(json::Builder{}.StartDict().
                    Key("error_message").Value("not found").
                    Key("request_id").Value((&node_inf.AsMap())->at("id").AsInt()).
                    EndDict().Build());
            }

        }

    }

    json::Print(json::Document{ json::Builder{}.Value(correct_requests).Build() }, std::cout);
}
