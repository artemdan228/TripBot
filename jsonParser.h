#ifndef ROUTE_PARSER_H
#define ROUTE_PARSER_H

#include <string>
#include <nlohmann/json.hpp>
#include <vector>

struct Transfer {
    std::string title;
};

struct RouteSegment {
    std::string from_city;
    std::string to_city;
    std::string from_station;
    std::string to_station;
    std::string date;
    std::vector<std::string> numbers;
    std::vector<std::string> transport_types;
    std::vector<std::string> vehicle;
    std::string departure_time;
    std::string arrival_time;
    bool hasTransfers;
    std::vector<Transfer> transfers;
};

class RouteParser {
public:
    explicit RouteParser(const nlohmann::json& jsonData);
    void parse();
    void printRoutes() const;
    std::string convertISO(const std::string& iso_date) const;
    std::string formatDateTime(const std::string& isoDateTime) const;

private:
    nlohmann::json data;
    std::vector<RouteSegment> routes;
};

#endif



