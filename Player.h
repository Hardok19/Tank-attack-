#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <cstdlib>
#include <utility>
#include <queue>
#include <random>
#include <SFML/Graphics/Vertex.hpp>
#include "Matrix.h"


#ifndef PLAYER_H
#define PLAYER_H

#define Width 800
#define Height 800
#define dw 30
#define dh 30

std::vector<std::pair<int, int>> path; // Para almacenar el camino encontrado



struct Node{
    int x, y;
    int dist;

    bool operator>(const Node& other) const {
        return dist > other.dist;
    }

};




// Clase para manejar un "player" con una imagen PNG
class Player {
private:
    sf::Sprite sprite;
    sf::Texture texture;
    std::pair<int, int> position;
    std::random_device rd;
    std::mt19937 gen;
    sf::Clock moveClock;    // Nuevo temporizador
    float updateInterval;   // Intervalo para la actualización del movimiento
    bool Dijkst;
    sf::Color pathColor;








public:
    bool mode;
    int health = 100;
    bool dobleturno = false;
    bool boostdamage = false;
    bool precision = false;
    bool _isMoving; // Nuevo flag para indicar movimiento
    auto getcol(){return pathColor;}





    Player(const std::string& textureFile, bool DK) : _isMoving(false), gen(rd()), updateInterval(0.2f), Dijkst(DK) {
        if (!texture.loadFromFile(textureFile)) {
            std::cerr << "Error loading texture: " << textureFile << std::endl;
        }
        sprite.setTexture(texture);
        setPosition(0, 0);  // Llamar a setPosition para inicializar tanto el sprite como la posición lógica
    }


    // Configurar el intervalo de actualización de movimiento
    void setUpdateInterval(float interval) {
        updateInterval = interval;
    }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);  // Actualiza la posición gráfica
        position = {static_cast<int>(y / dh), static_cast<int>(x / dw)};  // Actualiza la posición en la cuadrícula
    }

    // Add a method to set the path color
    void setPathColor(sf::Color color) {
        pathColor = color;
    }

    sf::Color getColor() const {
        return pathColor;
    }

    void setSize(float width, float height) {
        sf::Vector2u textureSize = texture.getSize();
        float scaleX = width / textureSize.x;
        float scaleY = height / textureSize.y;
        sprite.setScale(scaleX, scaleY);
    }

    // Separate method to draw only the path
    void drawPath(sf::RenderWindow& window) const {
        for (const auto& step : path) {
            sf::RectangleShape pathRect(sf::Vector2f(dw, dh));
            pathRect.setFillColor(sf::Color(pathColor.r, pathColor.g, pathColor.b, 128)); // Semi-transparent
            pathRect.setPosition(step.second * dw, step.first * dh);
            window.draw(pathRect);
        }
    }

    void draw(sf::RenderWindow& window) {
        // Draw the player sprite
        window.draw(sprite);
    }

    void startMoving() {
        if (!path.empty()) {
            _isMoving = true; // Inicia el movimiento
        }
    }

    std::pair<int, int> getPosition() const {
        return position;
    }

    void bfsMove(int targetX, int targetY, Graph matrix, std::deque<Player*> players) {
        if (targetX < 0 || targetX >= matrix.size() || targetY < 0 || targetY >= matrix.size()) {
            return;
        }
        std::queue<std::pair<int, int>> q;
        std::vector<std::vector<bool>> visited(matrix.size(), std::vector<bool>(matrix.size(), false));
        std::vector<std::vector<std::pair<int, int>>> parent(matrix.size(), std::vector<std::pair<int, int>>(matrix.size(), {-1, -1}));

        q.push(position);
        visited[position.first][position.second] = true;

        while (!q.empty()) {
            auto current = q.front();
            q.pop();

            if (current.second == targetX && current.first == targetY) {
                path.clear();
                std::pair<int, int> step = current;
                while (step != position) {
                    path.push_back(step);
                    step = parent[step.first][step.second];
                }
                std::reverse(path.begin(), path.end());
                return;
            }

            for (const auto& direction : std::vector<std::pair<int, int>>{{0, 1}, {1, 0}, {0, -1}, {-1, 0}}) {
                int neighborX = current.second + direction.second;
                int neighborY = current.first + direction.first;

                if (neighborX >= 0 && neighborY >= 0 && neighborX < matrix.size() && neighborY < matrix.size() &&
                    matrix.xy(neighborY, neighborX) != 0 && !visited[neighborY][neighborX]) {

                    // Check if the neighbor position is occupied by another player
                    bool isOccupied = false;
                    for (const Player* player : players) {
                        if (player != this && player->getPosition() == std::make_pair(neighborY, neighborX)) {
                            isOccupied = true;
                            break;
                        }
                    }

                    if (!isOccupied) {
                        visited[neighborY][neighborX] = true;
                        parent[neighborY][neighborX] = current;
                        q.push({neighborY, neighborX});
                    }
                }
            }
        }

        path.clear();
    }

    void randomMove(int targetX, int targetY, Graph matrix, std::deque<Player*> players) {
        path.clear();
        std::pair<int, int> current = position;

        while (current.first != targetY || current.second != targetX) {
            std::vector<std::pair<int, int>> validMoves;

            std::vector<std::pair<int, int>> directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
            for (const auto& dir : directions) {
                int newX = current.second + dir.second;
                int newY = current.first + dir.first;

                if (newX >= 0 && newX < matrix.size() && newY >= 0 && newY < matrix.size() && matrix.xy(newY, newX) != 0) {
                    // Check if the new position is occupied by another player
                    bool isOccupied = false;
                    for (const Player* player : players) {
                        if (player != this && player->getPosition() == std::make_pair(newY, newX)) {
                            isOccupied = true;
                            break;
                        }
                    }

                    if (!isOccupied) {
                        validMoves.push_back({newY, newX});
                    }
                }
            }

            if (validMoves.empty()) {
                break;
            }

            std::uniform_int_distribution<> dis(0, validMoves.size() - 1);
            auto nextMove = validMoves[dis(gen)];

            path.push_back(nextMove);
            current = nextMove;
        }
    }

    void Dijkstra(int targetY, int targetX, Graph matrix, std::deque<Player*> players) {
        std::vector<std::pair<int, int>> pathreal;
        int rows = matrix.size();
        int cols = matrix.size();

        // Distancias iniciales infinitas
        std::vector<std::vector<std::pair<int, int>>> prev(rows, std::vector<std::pair<int, int>>(cols, {-1, -1}));
        std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, std::numeric_limits<int>::max()));
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> q;


        int startX = position.first;
        int startY = position.second;

        dist[startX][startY] = 0;
        q.push({startX, startY, 0});
        std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        while (!q.empty()) {
            Node current = q.top();
            q.pop();


            // Si alcanzamos el objetivo, terminamos
            if (current.x == targetX && current.y == targetY) {
                // Reconstruir la ruta desde el objetivo al inicio usando 'prev'
                int cx = targetX, cy = targetY;
                while (cx != -1 && cy != -1) {
                    path.push_back({cx, cy});
                    std::tie(cx, cy) = prev[cx][cy];
                }



                std::reverse(path.begin(), path.end());
                return;
            }

            for(const auto& direction : directions) {

                int newX = current.x + direction.first;
                int newY = current.y + direction.second;
                //std::cout << newX << " " << newY << std::endl;



                if (newX >= 0 && newX < rows && newY >= 0 && newY < cols && matrix.xy(newX, newY) != 0) {
                    int newDist = current.dist + 1;

                    bool isOccupied = false;
                    for (const Player* player : players) {
                        if (player != this && player->getPosition() == std::make_pair(newX, newY)) {
                            isOccupied = true;
                            break;
                        }
                    }
                    if (newDist < dist[newX][newY] && !isOccupied) {
                        dist[newX][newY] = newDist;
                        prev[newX][newY] = {current.x, current.y}; // Guarda el nodo previo
                        q.push({newX, newY, newDist});
                    }

                }
            }
        }
    }



    // Elegir el algoritmo y configurar la velocidad según el tipo de movimiento
    bool moveTo(int targetX, int targetY, Graph matrix, std::deque<Player*> players) {

        if (targetX < 0 || targetX >= matrix.size() || targetY < 0 || targetY >= matrix.size()) {
            std::cout << "Posición fuera de los límites." << std::endl;
            return false;
        }

        for (Player* player : players) {
            if (player != this && player->getPosition() == std::make_pair(targetY, targetX)) {
                std::cout << "La posición está ocupada por otro jugador." << std::endl;
                return false;
            }
        }
        if(!Dijkst){
            if(!precision) {
                std::uniform_int_distribution<> dis(0, 1);
                if (dis(gen) == 0) {
                    bfsMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.1f);
                } else {
                    randomMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.001f);
                }
            }
            else {
                std::uniform_int_distribution<> dis(0, 9);
                if (dis(gen) <= 8) {
                    bfsMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.1f);
                } else {
                    randomMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.001f);
                }
            }



        }
        else {
            if(!precision) {
                std::uniform_int_distribution<> dis(0, 9);
                int num = dis(gen);
                if(num < 8) {
                    Dijkstra(targetX, targetY, matrix, players);
                    setUpdateInterval(0.1f);
                }
                else {
                    randomMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.001f);
                }
            }
            else {
                std::uniform_int_distribution<> dis(0, 9);
                int num = dis(gen);
                if(num <= 8) {
                    Dijkstra(targetX, targetY, matrix, players);
                    setUpdateInterval(0.1f);
                }
                else {
                    randomMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.001f);
                }
            }

        }
        precision = false;
        _isMoving = !path.empty();
        return _isMoving;
    }


    bool isMoving() const {
        return _isMoving;
    }

    void clearPath() {
        path.clear();
        _isMoving = false;
    }

    void updatePosition() {
        if (_isMoving && !path.empty()) {
            // Si ha pasado suficiente tiempo, actualizar la posición
            if (moveClock.getElapsedTime().asSeconds() >= updateInterval) {
                auto nextStep = path.front();
                path.erase(path.begin());

                // Actualiza la posición del jugador
                setPosition(nextStep.second * dw, nextStep.first * dh);

                // Reiniciar el temporizador después de mover
                moveClock.restart();

                // Verifica si hemos llegado a la última posición
                if (path.empty()) {
                    _isMoving = false;
                    position = nextStep;
                }
            }
        }
    }

    bool hasPath() const {
        return !path.empty();
    }

    void hit(float dama) {
        health -= dama;
    }



};




















#endif //PLAYER_H
