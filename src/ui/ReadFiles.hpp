#ifndef VULKAN_CUBE_READFILES_HPP
#define VULKAN_CUBE_READFILES_HPP

#include "Common.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class ReadFiles {
public:
    void readFiles(Instances *_instances) {
        instances = _instances;
        readVertexFile();
        readIndexFile();
    }

private:
    Instances *instances;
    std::vector<std::string> sensors;
    std::stack<int> sensorsHistory;

    void readVertexFile() {
        std::ifstream file("shaders/swing0701.trc");
        if (file.fail()) {
            throw std::runtime_error("failed to open vertex file");
        }

        std::string buffer;

        // get sensors names
        for (int i = 0; i < 4; i++) {
            std::getline(file, buffer);
        }
        std::istringstream iss(buffer);
        std::string tmp;
        for (int i = 0; i < 2; i++) {
            std::getline(iss, tmp, '\t');
        }
        while (std::getline(iss, tmp, '\t')) {
            if (tmp != "") {
                sensors.push_back(tmp);
            }
        }

        // skip line5~6
        for (int i = 0; i < 2; ++i) {
            std::getline(file, buffer);
        }

        // get vertex data
        while (std::getline(file, buffer)) {
            std::istringstream iss(buffer);
            std::vector<float> data;
            while (std::getline(iss, tmp, '\t')) {
                data.push_back(std::stof(tmp));
            }
            data.erase(data.begin(), data.begin() + 2);

            std::vector<Vertex> atTimeVertices(data.size()/3);
            for (int i = 0; i < data.size()/3; i++) {
                Vertex vert = {{data[i*3], data[i*3+1], data[i*3+2]}, {1.0f, 1.0f, 1.0f}};
                atTimeVertices[i] = vert;
            }
            instances->vertices.push_back(atTimeVertices);
        }
    }

    void readIndexFile() {
        std::ifstream file("shaders/swing0701.bvh");
        if (file.fail()) {
            throw std::runtime_error("failed to open bvh file");
        }

        std::string buffer;
        std::string tmp;

        // get root sensor
        while (std::getline(file, buffer)) {
            if (buffer.find("ROOT") != std::string::npos) {
                int pos = static_cast<int>(buffer.find("ROOT"));
                buffer.erase(0, pos+5);
                buffer.erase(buffer.size() - 1);
                auto pointer = std::find(sensors.begin(), sensors.end(), buffer);
                sensorsHistory.push(std::distance(sensors.begin(), pointer));
                std::getline(file, buffer);
                break;
            }
        }

        // get all sensor index
        while (std::getline(file, buffer)) {
            if (buffer.find("End Site") != std::string::npos) {
                instances->gIndices.push_back(sensorsHistory.top());
                instances->gIndices.push_back(sensorsHistory.top() + 1);
                sensorsHistory.pop();
                for (int i = 0; i < 4; ++i) {
                    std::getline(file, buffer);
                }
            } else if (buffer.find("}") != std::string::npos) {
                sensorsHistory.pop();
                if (sensorsHistory.empty()) {
                    break;
                }
            } else if (buffer.find("JOINT") != std::string::npos) {
                int pos = static_cast<int>(buffer.find("JOINT"));
                buffer.erase(0, pos+6);
                buffer.erase(buffer.size() - 1);
                auto sensorPoint = std::find(sensors.begin(), sensors.end(), buffer);
                int sensorIndex = std::distance(sensors.begin(), sensorPoint);
                instances->gIndices.push_back(sensorsHistory.top());
                instances->gIndices.push_back(sensorIndex);
                sensorsHistory.push(sensorIndex);
            }
        }
    }
};

#endif //VULKAN_CUBE_READFILES_HPP
