#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <string_view>
#include <iostream>
#include <set>
#include <functional>
#include <utility>
#include <unordered_set>
#include "geo.h"
using namespace std::literals;

// тип маршрута, для удобного подсчета
enum class RouteType {
	UNKNOWN,
	LINEAR,
	CIRCLE,
};

//информация о маршруте
struct RouteInfo {
	std::string name;
	RouteType route_type;
	int num_of_stops = 0;
	int num_of_unique_stops = 0;
	int route_length = 0;
	double curvature = 0.0;
};

//информация об остановке
struct Stop {
	std::string name;
	Coordinates coordinate;
	friend bool operator==(const Stop& lhs, const Stop& rhs) {
		return (lhs.name == rhs.name && lhs.coordinate == rhs.coordinate);
	}
};

//Маршрут состоит из номера автобуса,типа и списка остановок
struct Route {
	std::string name;
	RouteType route_type = RouteType::UNKNOWN;
	std::vector<const Stop*> stops; // указатели должны указывать на остановки хранящиеся в этом же каталоге
	friend bool operator==(const Route& lhs, const Route& rhs) {
		return (lhs.name == rhs.name);
	}
};
namespace detail_transport_catalogue {
	// считает количество остановок по маршруту
	int CalculateStops(const Route* count_route) noexcept;

	// считает количество уникальных остановок по маршруту
	int CalculateUniqueStops(const Route* route) noexcept;

	// считает расстояние по маршруту по прямой между координатами остановок
	double CalculateRouteLength(const Route* route) noexcept;
}

namespace transport_catalogue {
	class TransportCatalogue {
	public:
		// добавляет остановку в каталог
		void AddStop(const std::string& stop_name, Coordinates coordinate);

		// формирует маршрут из списка остановок и добавляет его в каталог.
		void AddRoute(const std::string& route_name, RouteType route_type, const std::vector<std::string>& stops);

		// возвращает указатель на остановку по её имени
		// если остановки нет в каталоге - выбрасывает исключение
		const Stop* FindStop(const std::string& stop_name) const;

		// возвращает указатель на маршрут по его имени
		// если маршрута нет в каталоге - выбрасывает исключение
		const Route* FindRoute(const std::string& route_name) const;

		// возвращает информацию о маршруе по его имени
		// если маршрута нет в каталоге - выбрасывает исключение std::out_of_range
		RouteInfo GetRouteInfo(const std::string& route_name) const;

		// возвращает список автобусов, проходящих через остановку
		// если остановки нет в каталоге - выбрасывает исключение std::out_of_range
		std::set<std::string_view> GetBusesOnStop(const std::string& stop_name) const;

		// возвращает расстояние от остановки 1 до остановки 2 в прямом направлении
	   // если информации о расстоянии нет в каталоге - выбрасывает исключение
		int GetForwardDistance(const std::string& stop_from, const std::string& stop_to) const;

		// возвращает расстояние между остановками 1 и 2 - в прямом, либо если нет - в обратном направлении
		// если информации о расстоянии нет в каталоге - выбрасывает исключение
		int GetDistance(const std::string& stop_from, const std::string& stop_to) const;

		// добавляет в каталог информацию о расстоянии между двумя остановками
		// если какой-то из остановок нет в каталоге - выбрасывает исключение
		void SetDistance(const std::string& stop_from, const std::string& stop_to, int distance);

		// считает общее расстояние по маршруту
		// если нет информации о расстоянии между какой-либо парой соседних остановок - выбросит исключение
		int CalculateRealRouteLength(const Route* route) const;

		// возвращает ссылку на маршруты в каталоге
		const std::unordered_map<std::string_view, const Route*>& GetRoutes() const;
		// возвращает ссылку на остановки в каталоге
		const std::unordered_map<std::string_view, const Stop*>& GetStops() const;
		const std::unordered_map<std::string_view, std::set<std::string_view>>& GetBusesOnStops() const;

	private:
		// остановки
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Stop*> stops_by_names_;
		// автобусы на каждой остановке
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;
		// маршруты
		std::deque<Route> routes_;
		std::unordered_map<std::string_view, const Route*> routes_by_names_;
		// расстояния между остановками
		std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> distances_;
	};
}//transport_catalogue


