#ifndef VULKAN_CUBE_VERTEXDATA_HPP
#define VULKAN_CUBE_VERTEXDATA_HPP

#include "common/Common.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

// TODO vectorのpush back
class VertexData {
private:
    Instances *instances;
    std::vector<std::string> sensors;
    std::stack<int> sensorsHistory;
    std::vector<std::vector<glm::vec3>> joints;
    std::vector<uint16_t> pairs;
    const int DIVIDE = 4;
    
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
            std::vector<float> data;    // 数字の一時補完配列
            std::vector<glm::vec3> atTimeJoint;
            while (std::getline(iss, tmp, '\t')) {
                data.push_back(std::stof(tmp));
            }
            data.erase(data.begin(), data.begin() + 2);

            for (int i = 0; i < data.size()/3; i++) {
                glm::vec3 vec = {data[i*3], data[i*3+1], data[i*3+2]};
                atTimeJoint.push_back(vec);
            }
            joints.push_back(atTimeJoint);
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
                pairs.push_back(sensorsHistory.top());
                pairs.push_back(sensorsHistory.top() + 1);
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
                pairs.push_back(sensorsHistory.top());
                pairs.push_back(sensorIndex);
                sensorsHistory.push(sensorIndex);
            }
        }
    }

    void createVerticesIndices() {
        for (std::vector<glm::vec3> atTimeJoint : joints) {
            std::vector<Vertex> atTimeVertices;
            for (int i = 0; i < pairs.size()/2; i++) {
                Vertex v1 = {{atTimeJoint[pairs[i*2]]}, {1.0f, 1.0f, 1.0f}};
                Vertex v2 = {{atTimeJoint[pairs[i*2+1]]}, {1.0f, 1.0f, 1.0f}};
                createCylinder(v1, v2, &atTimeVertices);
            }
            instances->vertices.push_back(atTimeVertices);
        }
    }

    // TODO 共通処理を整理する
    void createCylinder(Vertex vert1, Vertex vert2, std::vector<Vertex> *vertices) {
        float radius = 10;
        float length = glm::length(vert1.pos - vert2.pos);
        Vertex vert;

        // ratate
        glm::vec3 direction = vert2.pos - vert1.pos;
        glm::vec3 rotateShaft = glm::vec3(direction.x, 0, direction.z);
        rotateShaft = glm::rotateY(rotateShaft, glm::radians(90.0f));
        float angle = direction.y / length; // cos
        angle = glm::acos(angle);

        // vert1
        vertices->push_back(vert1);
        for (int i = 0; i < DIVIDE; i++) {
            vert = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
            vert.pos.x = radius * glm::sin(2.0 * glm::pi<float>() / DIVIDE * i);
            vert.pos.z = radius * glm::cos(2.0 * glm::pi<float>() / DIVIDE * i);
            if (rotateShaft.x != 0 || rotateShaft.z!= 0) {
                vert.pos = glm::rotate(vert.pos, angle, rotateShaft);
            }
            vert.pos += vert1.pos;
            vertices->push_back(vert);
        }

        // vert2
        vertices->push_back(vert2);
        for (int i = 0; i < DIVIDE; i++) {
            vert = {{0.0f, length, 0.0f}, {0.0f, 0.0f, 1.0f}};
            vert.pos.x = radius * glm::sin(2.0 * glm::pi<float>() / DIVIDE * i);
            vert.pos.z = radius * glm::cos(2.0 * glm::pi<float>() / DIVIDE * i);
            if (rotateShaft.x != 0 || rotateShaft.z != 0) {
                vert.pos = glm::rotate(vert.pos, angle, rotateShaft);
            }
            vert.pos += vert1.pos;
            vertices->push_back(vert);
        }
    }

    void createIndices() {
        int adj = 0;
        for (int i = 0; i < pairs.size()/2; i++) {
            // vert1 indices
            for (int i = 1; i < DIVIDE; i++) {
                instances->gIndices.push_back(0 + adj);
                instances->gIndices.push_back(i + 1 + adj);
                instances->gIndices.push_back(i + adj);
            }
            instances->gIndices.push_back(0 + adj);
            instances->gIndices.push_back(1 + adj);
            instances->gIndices.push_back(DIVIDE + adj);

            adj += DIVIDE + 1;
            // vert2 indices
            for (int i = 1; i < DIVIDE; i++) {
                instances->gIndices.push_back(0 + adj);
                instances->gIndices.push_back(i + adj);
                instances->gIndices.push_back(i + 1 + adj);

                // draw side
                instances->gIndices.push_back(i + adj - DIVIDE - 1);
                instances->gIndices.push_back(i + 1 + adj - DIVIDE - 1);
                instances->gIndices.push_back(i + adj);

                instances->gIndices.push_back(i + 1 + adj - DIVIDE - 1);
                instances->gIndices.push_back(i + 1 + adj);
                instances->gIndices.push_back(i + adj);
            }
            instances->gIndices.push_back(0 + adj);
            instances->gIndices.push_back(DIVIDE + adj);
            instances->gIndices.push_back(1 + adj);

            // draw side
            instances->gIndices.push_back(DIVIDE + adj);
            instances->gIndices.push_back(1 + adj - DIVIDE - 1);
            instances->gIndices.push_back(1 + adj);

            instances->gIndices.push_back(DIVIDE + adj);
            instances->gIndices.push_back(adj - 1);
            instances->gIndices.push_back(1 + adj - DIVIDE - 1);

            adj += DIVIDE + 1;
        }
    }
    
public:
    void readFiles(Instances *_instances) {
        instances = _instances;

        readVertexFile();
        readIndexFile();
        createVerticesIndices();
        createIndices();
    }
};


#endif //VULKAN_CUBE_VERTEXDATA_HPP
