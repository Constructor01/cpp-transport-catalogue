#include "json_reader.h"



json::Document JsonReader::ReadJsonInformation() {
    //имя документа
    std::istream& input(std::cin);
    json::Document json_doc = json::Load(input);
    return json_doc;
}


std::vector<json::Node> JsonReader::ReadStatRequests(json::Document& doc_inf) {
    std::vector<json::Node> stat_requests;
    for (auto& node_inf : *(&(&(doc_inf.GetRoot().AsMap()))->at("stat_requests").AsArray())) {
        stat_requests.push_back(node_inf);
    }
    return stat_requests;
}

//отвечает за скорость автобуса и ожидания на остановке
std::pair<int, int> JsonReader::ReadRoutingSettings(json::Document& doc_inf) {
    std::map<std::string, json::Node> routing_settings = (&(doc_inf.GetRoot().AsMap()))->at("routing_settings").AsMap();
    int time = routing_settings.at("bus_wait_time").AsInt();
    int speed= routing_settings.at("bus_velocity").AsInt();
    std::pair<int, int> a = std::make_pair( time, speed );
    return a;
}


void JsonReader::ReadBaseRequests(transport_catalogue::TransportCatalogue& catalogue, json::Document& doc_inf) {
    //добовляем остановки
    this->AddStops(catalogue, doc_inf);

    //добавляет расстояние между остановками
    this->AddDistanceStops(catalogue, doc_inf);

    //добовляем автобусы и мрашруты к ним
    this->AddBusAndRouts(catalogue, doc_inf);
}

void JsonReader::AddStops(transport_catalogue::TransportCatalogue& catalogue, json::Document& doc_inf) const {
    //добовляем остановки
    for (auto& node_inf : *(&(&(doc_inf.GetRoot().AsMap()))->at("base_requests").AsArray())) {
        if (*(&(&node_inf.AsMap())->at("type").AsString()) == "Stop") {
            Coordinates cord;
            cord.lat = (&node_inf.AsMap())->at("latitude").AsDouble();
            cord.lng = (&node_inf.AsMap())->at("longitude").AsDouble();
            catalogue.AddStop(*(&(&node_inf.AsMap())->at("name").AsString()), cord);

        }
    }
}

void JsonReader::AddDistanceStops(transport_catalogue::TransportCatalogue& catalogue, json::Document& doc_inf) const {
    //добавляет расстояние между остановками
    for (auto& node_inf : *(&(&(doc_inf.GetRoot().AsMap()))->at("base_requests").AsArray())) {
        if (*(&(&node_inf.AsMap())->at("type").AsString()) == "Stop") {
            //обращаемся к контейнеру с расстояниями до остановок
            auto& road_to_road_inf = *(&(&node_inf.AsMap())->at("road_distances").AsMap());
            for (auto& one_road : road_to_road_inf) {
                catalogue.SetDistance(*(&(&node_inf.AsMap())->at("name").AsString()), one_road.first, one_road.second.AsInt());
            }
        }
    }
}

void JsonReader::AddBusAndRouts(transport_catalogue::TransportCatalogue& catalogue, json::Document& doc_inf) const {
    //добовляем автобусы и мрашруты к ним
    for (auto& node_inf : *(&(&(doc_inf.GetRoot().AsMap()))->at("base_requests").AsArray())) {
        if (*(&(&node_inf.AsMap())->at("type").AsString()) == "Bus") {
            //имя автобуса
            std::string name = *(&(&node_inf.AsMap())->at("name").AsString());
            //создаем вектор остановок на маршруте
            std::vector<std::string> stop_names;
            json::Array all_bus_stops = (&node_inf.AsMap())->at("stops").AsArray();
            for (auto& stop : all_bus_stops) {
                stop_names.push_back(stop.AsString());
            }
            //проверяем тип маршрута(круговой или линейный)
            if ((&node_inf.AsMap())->at("is_roundtrip").AsBool()) {
                catalogue.AddRoute(name, RouteType::CIRCLE, stop_names);
            }
            else {
                catalogue.AddRoute(name, RouteType::LINEAR, stop_names);
            }
        }
    }
}

RenderSettings JsonReader::ReadRenderSettings(json::Document& doc_inf) const {
    RenderSettings render_settings;
    std::map<std::string, json::Node> json_settings = (&(doc_inf.GetRoot().AsMap()))->at("render_settings").AsMap();

    double width = json_settings.at("width").AsDouble();
    double height = json_settings.at("height").AsDouble();
    render_settings.size = svg::Point(width, height);

    render_settings.padding = json_settings.at("padding").AsDouble();

    render_settings.line_width = json_settings.at("line_width").AsDouble();
    render_settings.stop_radius = json_settings.at("stop_radius").AsDouble();

    render_settings.bus_label_font_size = json_settings.at("bus_label_font_size").AsDouble();
    double bus_label_offset_x = json_settings.at("bus_label_offset").AsArray()[0].AsDouble();
    double bus_label_offset_y = json_settings.at("bus_label_offset").AsArray()[1].AsDouble();
    render_settings.bus_label_offset = svg::Point(bus_label_offset_x, bus_label_offset_y);

    render_settings.stop_label_font_size = json_settings.at("stop_label_font_size").AsDouble();
    double stop_label_offset_x = json_settings.at("stop_label_offset").AsArray()[0].AsDouble();
    double stop_label_offset_y = json_settings.at("stop_label_offset").AsArray()[1].AsDouble();
    render_settings.stop_label_offset = svg::Point(stop_label_offset_x, stop_label_offset_y);

    render_settings.underlayer_color = DefineColor(json_settings.at("underlayer_color"));
    render_settings.underlayer_width = json_settings.at("underlayer_width").AsDouble();

    for (auto& color_type : json_settings.at("color_palette").AsArray()) {
        render_settings.color_palette.push_back(DefineColor(color_type));
    }
    return render_settings;
}

//проверяет на цвет
svg::Color JsonReader::DefineColor(const json::Node& color_type) const {
    svg::Color color;
    //если есть вектор, то это Rgb или Rgba
    if (color_type.IsArray()) {
        if (color_type.AsArray().size() == 4) {
            svg::Color rgba = svg::Rgba{
                static_cast<uint8_t>(color_type.AsArray()[0].AsInt()),
                static_cast<uint8_t>(color_type.AsArray()[1].AsInt()),
                static_cast<uint8_t>(color_type.AsArray()[2].AsInt()),
                color_type.AsArray()[3].AsDouble() };
            color = rgba;
        }
        if (color_type.AsArray().size() == 3) {
            svg::Color rgb = svg::Rgb{
                static_cast<uint8_t>(color_type.AsArray()[0].AsInt()),
                static_cast<uint8_t>(color_type.AsArray()[1].AsInt()),
                static_cast<uint8_t>(color_type.AsArray()[2].AsInt()) };
            color = rgb;
        }
    }
    //если есть строка, то это просто цвет
    if (color_type.IsString()) {
        color = color_type.AsString();
    }
    return color;
}

