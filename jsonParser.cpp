#include <iostream>
#include "jsonParser.h"
#include <ctime>
#include <sstream>

RouteParser::RouteParser(const nlohmann::json& jsonData) {
    data = jsonData;
}

void RouteParser::parse() {
    if (!data.contains("segments") || data["segments"].empty()) {
        std::cerr << "В ответе отсутствуют сегменты!" << std::endl;
        return;
    }

    for (const auto& segment : data["segments"]) {

        RouteSegment route;

        route.from_city = data["search"]["from"]["title"];
        route.to_city = data["search"]["to"]["title"];
        route.date = data["search"]["date"];

        if (segment.contains("departure_from") && segment["departure_from"].contains("title")) {
            route.from_station = segment["departure_from"]["title"];
        } else if (segment.contains("from") && segment["from"].contains("popular_title") && !segment["from"]["popular_title"].is_null()) {
            route.from_station = segment["from"]["popular_title"];
        }

        if (segment.contains("arrival_to") && segment["arrival_to"].contains("title")) {
            route.to_station = segment["arrival_to"]["title"];
        } else if (segment.contains("to") && segment["to"].contains("popular_title") && !segment["to"]["popular_title"].is_null()) {
            route.to_station = segment["to"]["popular_title"]; // Исправлено здесь
        }

        route.departure = segment["departure"];
        route.arrival = segment["arrival"];
        route.hasTransfers = segment["has_transfers"];

        // if (segment.contains("transport_types")) {
        //     for (const auto& transport_type : segment["transport_types"]) {
        //         route.transport_types.push_back(transport_type.get<std::string>());
        //     }
        // } else {
        //     if (segment.contains("thread") && segment["thread"].contains("transport_type") && !segment["thread"]["transport_type"].is_null()) {
        //         route.transport_types.push_back(segment["thread"]["transport_type"].get<std::string>());
        //     }
        // }

        // if (segment.contains("details") && segment["details"].is_array()) {
        //     for (const auto& detail : segment["details"]) {
        //         if (detail.contains("thread") && detail["thread"].contains("number") && detail["thread"]["number"].is_string()) {
        //             route.numbers.push_back(detail["thread"]["number"].get<std::string>());
        //         }
        //     }
        // } else if (!segment.contains("details") && segment.contains("thread") && segment["thread"].contains("number") && !segment["thread"]["number"].is_null()) {
        //     route.numbers.push_back(segment["thread"]["number"].get<std::string>());
        // }

        // Проверяем, есть ли поле "has_transfers"
        if (segment.contains("has_transfers")) {
            bool hasTransfers = segment["has_transfers"].get<bool>();
            std::cout << "has_transfers: " << hasTransfers << std::endl; // Отладочная печать

            if (hasTransfers) {
                if (segment.contains("details")) {
                    for (const auto& detail : segment["details"]) {
                        if (detail.contains("thread") && detail["thread"].contains("number")) {
                            std::string flightNumber = detail["thread"]["number"].get<std::string>();
                            std::cout << "Flight number (with transfer): " << flightNumber << std::endl; // Отладочная печать
                            route.numbers.push_back(flightNumber);
                        }
                    }
                }
            } else {
                // Если нет пересадок, просто получаем номер рейса из "thread"
                if (segment.contains("thread") && segment["thread"].contains("number")) {
                    std::string flightNumber = segment["thread"]["number"].get<std::string>();
                    std::cout << "Flight number (no transfer): " << flightNumber << std::endl; // Отладочная печать
                    route.numbers.push_back(flightNumber);
                }
            }
        } else {
            std::cout << "has_transfers field is missing!" << std::endl; // Отладочная печать, если поля нет
            // Если нет поля "has_transfers", можно считать, что пересадок нет
            if (segment.contains("thread") && segment["thread"].contains("number")) {
                std::string flightNumber = segment["thread"]["number"].get<std::string>();
                std::cout << "Flight number (no transfers field): " << flightNumber << std::endl; // Отладочная печать
                route.numbers.push_back(flightNumber);
            }
        }



        // if (segment.contains("details")) {
        //     for (const auto& detail : segment["details"]) {
        //         if (detail.contains("thread") && detail["thread"].contains("number")) {
        //             route.numbers.push_back(detail["thread"]["number"].get<std::string>());  // Добавляем номер рейса
        //         }
        //     }
        // }

        if (route.hasTransfers && segment.contains("transfers")) {
            if (segment["transfers"].size() > 1) {
                continue;
            }

            for (const auto& transfer : segment["transfers"]) {
                if (transfer.contains("title")) {
                    route.transfers.push_back({transfer["title"]});
                }
            }
        }

        routes.push_back(route);
    }
}

std::string RouteParser::convertISO(const std::string& iso_date) const {

    std::unordered_map<std::string, std::string> months = {
        {"01", "января"}, {"02", "февраля"}, {"03", "марта"}, {"04", "апреля"},
        {"05", "мая"}, {"06", "июня"}, {"07", "июля"}, {"08", "августа"},
        {"09", "сентября"}, {"10", "октября"}, {"11", "ноября"}, {"12", "декабря"}
    };

    std::string year = iso_date.substr(0, 4);
    std::string month = iso_date.substr(5, 2);
    std::string day = std::to_string(std::stoi(iso_date.substr(8, 2))); // Убираем ведущий ноль

    return day + " " + months[month] + ", " + year;
}

std::string RouteParser::formatDateTime(const std::string& isoDateTime) const {
    std::tm tm = {};
    std::istringstream ss(isoDateTime.substr(0, 19));
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

    if (ss.fail()) {
        return "Ошибка форматирования";
    }

    std::string timezoneOffset = isoDateTime.substr(19);

    char buffer[100];
    strftime(buffer, sizeof(buffer), "%d %B %Y, %H:%M", &tm);

    std::string formattedDate(buffer);
    std::string months_en[] = { "January", "February", "March", "April", "May", "June",
                                "July", "August", "September", "October", "November", "December" };
    std::string months_ru[] = { "января", "февраля", "марта", "апреля", "мая", "июня",
                                "июля", "августа", "сентября", "октября", "ноября", "декабря" };

    for (size_t i = 0; i < 12; i++) {
        size_t pos = formattedDate.find(months_en[i]);
        if (pos != std::string::npos) {
            formattedDate.replace(pos, months_en[i].length(), months_ru[i]);
            break;
        }
    }

    return formattedDate + " (GMT" + timezoneOffset + ")";
}


void RouteParser::printRoutes() const {
    if (routes.empty()) {
        std::cout << "Маршруты не найдены!" << std::endl;
        return;
    }

    int count = 1;

    for (const auto& route : routes) {
        std::cout << count << ". " << route.from_city << " → " << route.to_city << " (" <<
            route.from_station << " → " << route.to_station << ")" << "\n";

        std::cout << "   📅 Дата: " << convertISO(route.date) << "\n";

        std::cout << "   ⏳ Время отправления: " << formatDateTime(route.departure) << " | Время прибытия: " << formatDateTime(route.arrival) << std::endl;

        if (route.hasTransfers) {
            std::ostringstream transferInfo;
            transferInfo << "   🔁 Пересадка: ";
            for (size_t i = 0; i < route.transfers.size(); ++i) {
                transferInfo << route.transfers[i].title;
                if (i != route.transfers.size() - 1) {
                    transferInfo << ", ";
                }
            }
            std::cout << transferInfo.str() << std::endl;
        } else {
            std::cout << "   🔁 Пересадка: Прямой маршрут, пересадок нет!"<< std::endl;
        }

        std::cout << "   🚀 Рейсы: " << "\n";

        for (size_t i = 0; i < route.numbers.size(); ++i) {


            std::cout << "      🛑 " << (route.transport_types[i] == "plane" ? "Самолёт" :
            route.transport_types[i] == "train" ? "Поезд" :
            route.transport_types[i] == "suburban" ? "Электричка" :
            route.transport_types[i] == "bus" ? "Автобус" :
            route.transport_types[i] == "water" ? "Водный транспорт" :
            route.transport_types[i] == "helicopter" ? "Вертолёт" :
            route.transport_types[i])
            << " | " << route.numbers[i] << "\n";
        }

        // for (const auto& flight_number : route.numbers) {
        //     std::cout << "      🛑 " << flight_number << "\n";
            // for(const auto& transport_type : route.transport_types) {
            //     std::cout << transport_type << flight_number << " ";
            // }
        std::cout << std::endl;

        std::cout << "--------------------------" << std::endl;
        count += 1;
    }
}
