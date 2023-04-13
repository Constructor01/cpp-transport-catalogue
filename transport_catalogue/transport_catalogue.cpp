#include "transport_catalogue.h"
namespace detail_transport_catalogue {
	// считает количество остановок по маршруту
	int CalculateStops(const Route* count_route) noexcept {
		int result = 0;
		if (count_route != nullptr) {
			result = static_cast<int>(count_route->stops.size());
			if (count_route->route_type == RouteType::LINEAR) {
				result = result * 2 - 1;
			}
		}
		return result;
	}

	// считает количество уникальных остановок по маршруту
	int CalculateUniqueStops(const Route* count_route) noexcept {
		int result = 0;
		if (count_route != nullptr) {
			std::unordered_set<std::string_view> uniques;
			for (auto stop : count_route->stops) {
				uniques.insert(stop->name);
			}
			result = static_cast<int>(uniques.size());
		}
		return result;
	}

	// считает расстояние по маршруту по прямой между координатами остановок
	double CalculateRouteLength(const Route* route) noexcept {
		double result = 0.0;
		if (route != nullptr) {
			for (auto iter1 = route->stops.begin(), iter2 = iter1 + 1;
				iter2 < route->stops.end();
				++iter1, ++iter2) {
				result += ComputeDistance((*iter1)->coordinate, (*iter2)->coordinate);
			}
			if (route->route_type == RouteType::LINEAR) {
				result *= 2;
			}
		}
		return result;
	}
}//detail_transport_catalogue

namespace transport_catalogue {
	void TransportCatalogue::AddStop(const std::string& stop_name, Coordinates coordinate) {
		Stop stop;
		stop.name = stop_name;
		stop.coordinate = coordinate;
		stops_.push_back(stop);
		stops_by_names_.insert({ stops_.back().name, &stops_.back() });
	}

	void TransportCatalogue::AddRoute(const std::string& route_name, RouteType route_type, const std::vector<std::string>& stops) {
		// формируем маршрут с указателями на соответсвующие остановки из каталога
		Route route;
		route.name = route_name;
		route.route_type = route_type;
		for (auto& stop_name : stops) {
			route.stops.push_back(FindStop(stop_name));
		}
		routes_.push_back(route);
		std::string_view route_name_add = routes_.back().name;
		routes_by_names_.insert({ route_name_add, &routes_.back() });
		// добавляем информацию об автобусе в остановки по маршруту
		for (auto stop : routes_.back().stops) {
			buses_on_stops_[stop->name].insert(route_name_add);
		}
	}

	const Stop* TransportCatalogue::FindStop(const std::string& stop_name) const {
		if (stops_by_names_.count(stop_name) == 0) {
			throw nullptr;
		}
		return stops_by_names_.at(stop_name);
	}

	const Route* TransportCatalogue::FindRoute(const std::string& route_name) const {
		if (routes_by_names_.count(route_name) == 0) {
			throw nullptr;
		}
		return routes_by_names_.at(route_name);
	}

	RouteInfo TransportCatalogue::GetRouteInfo(const std::string& route_name) const {
		RouteInfo result;
		auto route = FindRoute(route_name);
		result.name = route->name;
		result.route_type = route->route_type;
		result.num_of_stops = detail_transport_catalogue::CalculateStops(route);
		result.num_of_unique_stops = detail_transport_catalogue::CalculateUniqueStops(route);
		result.route_length = CalculateRealRouteLength(route);
		result.curvature = result.route_length / detail_transport_catalogue::CalculateRouteLength(route);
		return result;
	}

	std::set<std::string_view> TransportCatalogue::GetBusesOnStop(const std::string& stop_name) const {
		if (stops_by_names_.count(stop_name) == 0) {
			throw std::out_of_range("Stop "s + stop_name + " does not exist in catalogue"s);
		}
		//std::cout << "Pustoi "<< stop_name << std::endl;
		std::set<std::string_view> result;
		if (buses_on_stops_.count(stop_name) == 1) {
			for (auto& name : buses_on_stops_.at(stop_name)) {
				result.insert(name);
			}
		}
		return result;
	}

	int TransportCatalogue::GetForwardDistance(const std::string& stop_from,
		const std::string& stop_to) const {
		if (distances_.count(stop_from) == 0 || distances_.at(stop_from).count(stop_to) == 0) {
			throw std::out_of_range("No information about distance from "s + stop_from + " to "s + stop_to);
		}
		return distances_.at(stop_from).at(stop_to);
	}

	int TransportCatalogue::GetDistance(const std::string& stop_from, const std::string& stop_to) const {
		int result = 0;
		try {
			result = GetForwardDistance(stop_from, stop_to);
		}
		catch (std::out_of_range&) {
			try {
				result = GetForwardDistance(stop_to, stop_from);
			}
			catch (std::out_of_range&) {
				throw std::out_of_range("No information about distance between stops "s + stop_from + " and "s + stop_to);
			}
		}
		return result;
	}

	void TransportCatalogue::SetDistance(const std::string& stop_from, const std::string& stop_to, int distance) {
		auto Stop_from = FindStop(stop_from);
		auto Stop_to = FindStop(stop_to);
		distances_[Stop_from->name][Stop_to->name] = distance;
	}

	int TransportCatalogue::CalculateRealRouteLength(const Route* route) const {
		int result = 0;
		if (route != nullptr) {
			// проходим по маршруту вперед
			for (auto iter1 = route->stops.begin(), iter2 = iter1 + 1;
				iter2 < route->stops.end();
				++iter1, ++iter2) {
				result += GetDistance((*iter1)->name, (*iter2)->name);
			}
			// проходим по маршруту назад
			if (route->route_type == RouteType::LINEAR) {
				for (auto iter1 = route->stops.rbegin(), iter2 = iter1 + 1;
					iter2 < route->stops.rend();
					++iter1, ++iter2) {
					result += GetDistance((*iter1)->name, (*iter2)->name);
				}
			}
		}
		return result;
	}

	const std::unordered_map<std::string_view, const Route*>
		& TransportCatalogue::GetRoutes() const {
		return routes_by_names_;
	}

	const std::unordered_map<std::string_view, const Stop*>
		& TransportCatalogue::GetStops() const {
		return stops_by_names_;
	}

	const std::unordered_map<std::string_view, std::set<std::string_view>>
		& TransportCatalogue::GetBusesOnStops() const {
		return buses_on_stops_;
	}
}//transport_catalogue