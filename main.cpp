#include <iostream>
#include "cpr/cpr.h"
#include <sstream>
#include "jsonParser.h"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>

std::unordered_map<std::string, nlohmann::json> routeCache;

struct ApiRequest {
    std::string from;
    std::string to;
    std::string date_from;
    std::string date_to;
    std::string apikey;
};

std::string generateCacheKey(const std::string& from, const std::string& to, const std::string& date_from, const std::string& date_to) {
    return from + "_" + to + "_" + date_from + "_" + date_to;
}

std::string DateParser(std::string& inputDate) {
    std::istringstream iss(inputDate);
    int day, month, year;
    char point1, point2;

    if(iss >> day >> point1 >> month >> point2 >> year && point1 == '.' && point2 == '.') {
        std::ostringstream oss;
        oss << year << "-"
            << (month < 10 ? "0" : "") << month << "-"
            << (day < 10 ? "0" : "") << day;
        return oss.str();
    }

    return "";
}

void loadCacheFromFile(std::unordered_map<std::string, nlohmann::json>& cache, const std::string& filename) {
    std::ifstream inputFile(filename);
    if (inputFile.is_open()) {
        nlohmann::json cacheJson;
        inputFile >> cacheJson;
        cache = cacheJson.get<std::unordered_map<std::string, nlohmann::json>>();
        inputFile.close();
    } else {
        std::cerr << "Не удалось открыть файл для чтения кэша!" << std::endl;
    }
}

void saveCacheToFile(const std::unordered_map<std::string, nlohmann::json>& cache, const std::string& filename) {
    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        nlohmann::json cacheJson = cache;
        outputFile << cacheJson.dump(4);
        outputFile.close();
    } else {
        std::cerr << "Не удалось открыть файл для записи кэша!" << std::endl;
    }
}

int main() {

    loadCacheFromFile(routeCache, "cache.json");

    std::cout << "Введите дату поездки туда:" << "\n";
    std::string inputDate_from;
    std::cin >> inputDate_from;

    std::cout << "Введите дату поездки обратно:" << "\n";
    std::string inputDate_to;
    std::cin >> inputDate_to;

    ApiRequest request;
    request.from = "c2";
    request.to = "c77";
    request.date_from = DateParser(inputDate_from);
    request.date_to = DateParser(inputDate_to);
    request.apikey = "90dfe575-3c10-4440-a481-73a4650f08a1";

    if (request.date_from.empty() || request.date_to.empty()) {
        std::cerr << "Неправильный формат даты!" << std::endl;
        return 1;
    }

    std::string cacheKey_from = generateCacheKey(request.from, request.to, request.date_from, request.date_to);
    std::string cacheKey_to = generateCacheKey(request.to, request.from, request.date_to, request.date_from);

    nlohmann::json jsonData_from;
    if (routeCache.find(cacheKey_from) != routeCache.end()) {
        jsonData_from = routeCache[cacheKey_from];
        std::cout << "Данные маршрута туда найдены в кэше." << std::endl;
    } else {
        std::string url_from = "https://api.rasp.yandex.net/v3.0/search/"
                               "?from=" + request.from +
                               "&to=" + request.to +
                               "&format=json"
                               "&apikey=" + request.apikey +
                               "&date=" + request.date_from +
                               "&transfers=true";

        cpr::Response r_from = cpr::Get(cpr::Url{url_from});

        if (r_from.status_code == 200) {
            jsonData_from = nlohmann::json::parse(r_from.text);
            routeCache[cacheKey_from] = jsonData_from;
        } else {
            std::cout << "Ошибка запроса! Код: " << r_from.status_code << "\n";
            std::cout << "Ответ:\n" << r_from.text << "\n";
            return 1;
        }
    }

    nlohmann::json jsonData_to;
    if (routeCache.find(cacheKey_to) != routeCache.end()) {
        jsonData_to = routeCache[cacheKey_to];
        std::cout << "Данные маршрута обратно найдены в кэше." << std::endl;
    } else {
        std::string url_to = "https://api.rasp.yandex.net/v3.0/search/"
                             "?from=" + request.to +
                             "&to=" + request.from +
                             "&format=json"
                             "&apikey=" + request.apikey +
                             "&date=" + request.date_to +
                             "&transfers=true";

        cpr::Response r_to = cpr::Get(cpr::Url{url_to});

        if (r_to.status_code == 200) {
            jsonData_to = nlohmann::json::parse(r_to.text);
            routeCache[cacheKey_to] = jsonData_to;
        } else {
            std::cout << "Ошибка запроса! Код: " << r_to.status_code << "\n";
            std::cout << "Ответ:\n" << r_to.text << "\n";
            return 1;
        }
    }

    std::cout << "" << "\n";
    std::cout << "====================================" << "\n";
    std::cout << "САНКТ-ПЕТЕРБУРГ" << " ⇄ " << "БЛАГОВЕЩЕНСК" << "\n";
    std::cout << "====================================" << "\n";
    std::cout << "" << "\n";

    std::cout << "Маршруты туда:" << "\n";
    RouteParser routeParser_from(jsonData_from);
    routeParser_from.parse();
    routeParser_from.printRoutes();

    std::cout << "Маршруты обратно:" << "\n";
    RouteParser routeParser_to(jsonData_to);
    routeParser_to.parse();
    routeParser_to.printRoutes();

    saveCacheToFile(routeCache, "cache.json");

    return 0;
}
