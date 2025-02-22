#include <iostream>
#include "cpr/cpr.h"
#include <sstream>
#include "jsonParser.h"
#include <nlohmann/json.hpp>


struct ApiRequest {
    std::string from;
    std::string to;
    std::string date_from;
    std::string date_to;
    std::string apikey;
};

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

int main() {

    std::cout << "Введите дату поездки туда:" << "\n";
    std::string inputDate_from;
    std::cin >> inputDate_from;

    std::cout << "Введите дату поездки обратно:" << "\n";
    std::string inputDate_to;
    std::cin >> inputDate_to;

    ApiRequest request;
    request.from = "c2";
    request.to = "c213";
    request.date_from = DateParser(inputDate_from);
    request.date_to = DateParser(inputDate_to);
    request.apikey = "90dfe575-3c10-4440-a481-73a4650f08a1";

    if (request.date_from.empty() || request.date_to.empty()) {
        std::cerr << "Неправильный формат даты!" << std::endl;
        return 1;
    }

    std::string url_from = "https://api.rasp.yandex.net/v3.0/search/"
                      "?from=" + request.from +
                      "&to=" + request.to +
                      "&format=json"
                      "&apikey=" + request.apikey +
                      "&date=" + request.date_from +
                      "&transfers=true";

    std::string url_to = "https://api.rasp.yandex.net/v3.0/search/"
                      "?from=" + request.to +
                      "&to=" + request.from +
                      "&format=json"
                      "&apikey=" + request.apikey +
                      "&date=" + request.date_to +
                      "&transfers=true";

    cpr::Response r_from = cpr::Get(cpr::Url{url_from});

    cpr::Response r_to = cpr::Get(cpr::Url{url_to});

    std::cout << "" << "\n";
    std::cout << "====================================" << "\n";
    std::cout << "САНКТ-ПЕТЕРБУРГ" << " ⇄ " << "БЛАГОВЕЩЕНСК" << "\n";
    std::cout << "====================================" << "\n";
    std::cout << "" << "\n";

    if (r_from.status_code == 200) {
        //std::cout << "Ответ сервера:\n" << r.text << std::endl;

        std::cout << "Маршруты туда:" << '\n';
        nlohmann::json jsonData_from = nlohmann::json::parse(r_from.text);

        RouteParser routeParser_from(jsonData_from);
        routeParser_from.parse();
        routeParser_from.printRoutes();

    } else {
        std::cout << "Ошибка запроса! Код: " << r_from.status_code << "\n";
        std::cout << "Ответ:\n" << r_from.text << "\n";
    }

    if (r_to.status_code == 200) {
        std::cout << "Маршруты обратно:" << '\n';
        nlohmann::json jsonData_to = nlohmann::json::parse(r_to.text);

        RouteParser routeParser_to(jsonData_to);
        routeParser_to.parse();
        routeParser_to.printRoutes();
    } else{
        std::cout << "Ошибка запроса! Код: " << r_to.status_code << "\n";
        std::cout << "Ответ:\n" << r_to.text << "\n";
    }

    return 0;
}
