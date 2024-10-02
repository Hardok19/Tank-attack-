#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <random>
#include <list>
#include "Player.h"
#include "Bullet.h"


#include "Matrix.h"

#define Width 800
#define Height 800
#define dw 30
#define dh 30


std::mutex matrixMutex;
std::condition_variable cv;
bool matrixReady = false;
int currentTurn = 0; // Indice del turno actual

int instanttx;
int instantty;


// Función para dibujar un cuadrado de color en una posición específica
void Draw(sf::RenderWindow& window, int A, int B, sf::Color color) {
    // Eliminar esta línea, ya que invierte las coordenadas Y
    // B = Height / dh - 1 - B;
    if (A < 0 || B < 0 || A >= Width / dw || B >= Height / dh) return;

    sf::RectangleShape square(sf::Vector2f(dw, dh));
    square.setFillColor(color);
    square.setPosition(A * dw, B * dh);
    window.draw(square);
}



Graph crearmatriz(int rows, int cols, int obstacleCount, const std::vector<std::pair<int, int>>& playerPositions, Graph jola) {
    // Crear lista de todas las posiciones disponibles en la cuadrícula
    std::vector<std::pair<int, int>> availablePositions;
    jola.alltrue();

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            availablePositions.push_back({i, j});
        }
    }

    // Eliminar las posiciones ocupadas por los jugadores de la lista de posiciones disponibles
    for (const auto& playerPos : playerPositions) {
        availablePositions.erase(std::remove(availablePositions.begin(), availablePositions.end(), playerPos), availablePositions.end());
    }

    // Colocar obstáculos en posiciones aleatorias de la lista disponible
    for (int i = 0; i < obstacleCount; i++) {
        if (availablePositions.empty()) {
            std::cout << "No quedan posiciones disponibles para colocar más obstáculos." << std::endl;
            break;
        }

        // Selecciona una posición aleatoria de la lista de posiciones disponibles
        int randIndex = rand() % availablePositions.size();
        pair<int, int> obstaclePos = availablePositions[randIndex];

        // Marcar la posición como obstáculo en la matriz
        jola.removeEdge(obstaclePos.first, obstaclePos.second);

        // Eliminar la posición seleccionada de la lista
        availablePositions.erase(availablePositions.begin() + randIndex);
    }

    {
        std::lock_guard<std::mutex> lock(matrixMutex);
        matrixReady = true;

    }
    cv.notify_all();
    return jola;
}




int main() {
    try {
        srand(static_cast<unsigned int>(time(0)));

        sf::RenderWindow window(sf::VideoMode(Width, Height), "Tank Game");

        Player player1blue("../ProyectoII/Assets/TanqueAzul.png", false);
        Player player2sky("../ProyectoII/Assets/TanqueCeleste.png", false);
        Player player1red("../ProyectoII/Assets/TanqueRojo.png", true);
        Player player2yell("../ProyectoII/Assets/TanqueAmarillo.png", true);



        player1blue.setSize(dw - 2, dh - 2);
        player1blue.setPosition(dw * 1, dh * 1);

        player2sky.setSize(dw - 2, dh - 2);
        player2sky.setPosition(dw * 1, dh * 2);

        player1red.setSize(dw - 2, dh - 2);
        player1red.setPosition(dw * 1, dh * 3);

        player2yell.setSize(dw - 2, dh - 2);
        player2yell.setPosition(dw * 1, dh * 4);

        std::vector<std::pair<int, int>> playerPositions = {
            player1blue.getPosition(), player2sky.getPosition(),
            player1red.getPosition(), player2yell.getPosition()
        };

        std::vector<Bullet> bullets;
        bool canShoot = true;
        TrajectoryLine trajectoryLine;

        player1blue.setPathColor(sf::Color::Blue);
        player2sky.setPathColor(sf::Color::Cyan);
        player1red.setPathColor(sf::Color::Red);
        player2yell.setPathColor(sf::Color::Yellow);

        int rows = Height / dh;
        int cols = Width / dw;
        int obstacleCount = 20;

        Graph matriz(Height / dh);

        matriz.alltrue();


        std::deque<Player*> players = {&player1blue, &player2sky, &player1red, &player2yell};



        bool moveInitiated = false;

        matriz = crearmatriz(rows, cols, obstacleCount, playerPositions, matriz);

        matriz.toString();

        //std::thread matrixThread(generateAdjacencyMatrix, rows, cols, obstacleCount, playerPositions);

        sf::Clock clock;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left && !moveInitiated && canShoot) {
                        int targetX = event.mouseButton.x / dw;
                        int targetY = event.mouseButton.y / dh;


                        std::cout << "Left click - Move attempt" << std::endl;
                        moveInitiated = players[currentTurn]->moveTo(targetX, targetY, matriz, players);
                        if (moveInitiated) canShoot = false;
                    }

                    if (event.mouseButton.button == sf::Mouse::Middle && !moveInitiated && canShoot) {
                        int targetX = event.mouseButton.x / dw;
                        int targetY = event.mouseButton.y / dh;
                        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                        sf::Vector2f startPos = sf::Vector2f((players[currentTurn]->getPosition().second * dw) + 13.5,
                                                             (players[currentTurn]->getPosition().first * dh) + 13.5);
                        sf::Color bulletColor = players[currentTurn]->getColor();
                        if(players[currentTurn]->boostdamage) {
                            Bullet newBullet(bulletColor);
                            newBullet.damage = 45;
                            newBullet.fire(startPos, mousePos, players[currentTurn]); // Pasar el jugador actual como propietario
                            bullets.push_back(newBullet);
                            moveInitiated = true;
                            canShoot = false;
                            trajectoryLine.hide();
                            std::cout << "Middle click - Bullet fired from (" << startPos.x << ", " << startPos.y
                                      << ") to (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
                            players[currentTurn]->boostdamage = false;
                        }
                        else {
                            Bullet newBullet(bulletColor);
                            newBullet.damage = 45*0.5;
                            newBullet.fire(startPos, mousePos, players[currentTurn]); // Pasar el jugador actual como propietario
                            bullets.push_back(newBullet);
                            moveInitiated = true;
                            canShoot = false;
                            trajectoryLine.hide();
                            std::cout << "Middle click - Bullet fired from (" << startPos.x << ", " << startPos.y
                                      << ") to (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
                        }


                    }
                }

                if (event.type == sf::Event::MouseMoved && canShoot) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    sf::Vector2f startPos = sf::Vector2f(players[currentTurn]->getPosition().second * dw,
                                                          players[currentTurn]->getPosition().first * dh);
                    trajectoryLine.update(startPos, mousePos);
                }
            }

            {
                std::unique_lock<std::mutex> lock(matrixMutex);
                cv.wait(lock, [] { return matrixReady; });
            }



            // Verifica si se presionó una tecla
            if (event.type == sf::Event::KeyPressed) {
                // Si se presiona la tecla 'Space', activa el doble turno
                if (event.key.code == sf::Keyboard::Space) {
                    players[currentTurn]->dobleturno = true;
                    std::cout << "Doble turno activado para el jugador " << currentTurn << std::endl;
                }
                // Si se presiona la tecla 'D', activa el boost de daño
                if (event.key.code == sf::Keyboard::D) {
                    players[currentTurn]->boostdamage = true;
                    std::cout << "Boost de daño activado para el jugador " << players[currentTurn]->getcol().toInteger() << std::endl;
                    currentTurn = (currentTurn + 1) % players.size();
                }
                if(event.key.code == sf::Keyboard::P) {
                    players[currentTurn]->precision = true;
                    std::cout << "Precisión de movimiento activada " << players[currentTurn]->getcol().toInteger() << std::endl;
                    this_thread::sleep_for(std::chrono::milliseconds(50));
                    currentTurn = (currentTurn + 1) % players.size();
                }
            }


            window.clear(sf::Color::White);

            // Draw grid and obstacles
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    if (matriz.xy(i,j) == 0) {
                        Draw(window, j, i, sf::Color::Red); // Obstacle
                    } else {
                        Draw(window, j, i, sf::Color(240, 240, 240)); // Normal cell
                    }

                    // Draw cell border
                    sf::RectangleShape border(sf::Vector2f(dw, dh));
                    border.setFillColor(sf::Color::Transparent);
                    border.setOutlineThickness(1);
                    border.setOutlineColor(sf::Color(200, 200, 200));
                    border.setPosition(j * dw, i * dh);
                    window.draw(border);
                }
            }

            // Update and draw bullets
            for (auto& bullet : bullets) {
                bullet.update(Width, Height);
                bullet.draw(window);

                sf::Vector2f bulletPos = bullet.getPosition();
                int bulletGridX = static_cast<int>(bulletPos.x / dw);
                int bulletGridY = static_cast<int>(bulletPos.y / dh);

                if (bulletGridX >= 0 && bulletGridX < cols && bulletGridY >= 0 && bulletGridY < rows) {
                    if (matriz.xy(bulletGridY, bulletGridX) == 0) {
                        bullet.deactivate();
                        std::cout << "Bullet deactivated due to collision with obstacle" << std::endl;
                    }

                    for (Player* player : players) {
                        if (player->getPosition() == std::make_pair(bulletGridY, bulletGridX) && player != bullet.getOwner()) {
                            bullet.deactivate();
                            player->hit(bullet.damage);
                            std::cout << "Bullet hit a player " << bullet.damage << " de daño"  << std::endl;

                        }
                    }
                }
            }

            // Remove inactive bullets
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) { return !b.isActive(); }),
                          bullets.end());

            // Update player positions
            for (auto it = players.begin(); it != players.end();) {
                Player* player = *it;
                if (player->health > 0) {
                    player->updatePosition();
                    ++it;
                } else {
                    it = players.erase(it); // Elimina el jugador y actualiza el iterador
                }
            }

            // Check if the current player's move is complete
            if (moveInitiated) {
                bool movementComplete = players[currentTurn]->isMoving() == false;

                if (movementComplete) {
                    if(players[currentTurn]->dobleturno == false) {
                        currentTurn = (currentTurn + 1) % players.size(); // Cambia al siguiente jugador
                        moveInitiated = false;
                        canShoot = true;
                        std::cout << "Turn switched. Next player's turn" << std::endl;
                    }
                    else {
                        players[currentTurn]->dobleturno = false;
                        moveInitiated = false;
                        canShoot = true;
                        std::cout << "Doble turno" << std::endl;
                    }

                }
            }

            // Draw players and trajectory line
            for (Player* player : players) {
                player->drawPath(window);
                player->draw(window);
            }


            trajectoryLine.draw(window);
            window.display();
        }

        //matrixThread.join();
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}

