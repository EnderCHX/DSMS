//
// Created by c on 2025/4/13.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace CHX {

using json = nlohmann::json;

class Config {
public:
    Config(const std::string &config_path) {
        std::ifstream config_file;
		try {
            config_file.open(config_path);
            std::string content;
            config_file >> content;
            config = json::parse(content);
            config_file.close();
		}catch (...) {
            std::cout << "open config file error" << std::endl;
		}
    }
    json GetConfig() {
        return config;
    }
private:
    json config;
};
}



#endif //CONFIG_H
