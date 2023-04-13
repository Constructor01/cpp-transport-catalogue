#pragma once
#include <map>
#include "svg.h"
#include "geo.h"
#include "transport_catalogue.h"



struct RenderSettings {
    svg::Point size;

    double padding = 0.0;

    double line_width = 0.0;
    double stop_radius = 0;

    int bus_label_font_size = 0;
    svg::Point bus_label_offset;

    int stop_label_font_size = 0;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width = 0.0;

    std::vector<svg::Color> color_palette;
};

class MapRenderer {
public:
    //заносим параметры карты
    void SetSettings(const RenderSettings& settings) {
        settings_ = settings;
    }

    //формирование всей карты
    svg::Document RenderMap(const transport_catalogue::TransportCatalogue& catalogue);
private:
    //функции отрисовки всех даннх маршрута
    void RenderLines(svg::Document& doc, const std::map<std::string_view, const Route*>& routes) const;
    void RenderRouteNames(svg::Document& doc, const std::map<std::string_view, const Route*>& routes) const;
    void RenderStops(svg::Document& doc, const std::map<std::string_view, const Stop*>& stops, const std::unordered_map<std::string_view, std::set<std::string_view>>& buses_on_stops) const;
    void RenderStopNames(svg::Document& doc, const std::map<std::string_view, const Stop*>& stops, const std::unordered_map<std::string_view, std::set<std::string_view>>& buses_on_stops) const;

    // возвращает пару - минимальная и максимальная координаты прямоугольника, в который вписаны все остановки на маршрутах
    std::pair<Coordinates, Coordinates> ComputeFieldSize(const transport_catalogue::TransportCatalogue& catalogue) const;

    // пересчитывает широту и долготу в координаты для рисования на карте
    svg::Point GetRelativePoint(Coordinates coordinate) const;

    RenderSettings settings_;
    std::pair<Coordinates, Coordinates> field_size_;
};

