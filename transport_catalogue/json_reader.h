#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include "transport_catalogue.h"
//#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"

class JsonReader final {
public:
    //считывает поток данных
    json::Document ReadJsonInformation();

    //отвечает на запросы
    std::vector<json::Node> ReadStatRequests(json::Document& doc_inf);

    //отвечает за скорость автобуса и ожидания на остановке
    std::pair<int, int> ReadRoutingSettings(json::Document& doc_inf);

    //считывает всю инофрмацию о автобусах, маршрутах и остановках
    void ReadBaseRequests(transport_catalogue::TransportCatalogue& catalogue, json::Document& doc_inf);

    void AddStops(transport_catalogue::TransportCatalogue& catalogue, json::Document& doc_inf) const;

    void AddDistanceStops(transport_catalogue::TransportCatalogue& catalogue, json::Document& doc_inf) const;

    void AddBusAndRouts(transport_catalogue::TransportCatalogue& catalogue, json::Document& doc_inf) const;

    //считывает настройки рендера карты
    RenderSettings ReadRenderSettings(json::Document& doc_inf) const;

    //проверяет на цвет
    svg::Color DefineColor(const json::Node& color_type) const;

};




