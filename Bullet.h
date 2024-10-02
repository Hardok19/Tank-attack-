#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#ifndef BULLET_H
#define BULLET_H



class Bullet {
private:
    sf::CircleShape shape;
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    bool active;
    sf::Color color;
    int bounceCount;
    static const int MAX_BOUNCES = 3; // Número máximo de rebotes permitidos
    Player* owner; // Nuevo: puntero al jugador que disparó la bala

public:
    float damage;
    Bullet(sf::Color bulletColor = sf::Color::Black)
        : speed(0.40f), active(false), color(bulletColor), bounceCount(0), owner(nullptr) {
        shape.setRadius(3.0f);
        shape.setFillColor(color);
    }

    void fire(const sf::Vector2f& start, const sf::Vector2f& target, Player* shooter) {
        position = start;
        direction = target - start;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        direction /= length;
        active = true;
        bounceCount = 0;
        owner = shooter; // Establecer el propietario de la bala
    }

    void update(int windowWidth, int windowHeight) {
        if (active) {
            position += direction * speed;

            // Comprobar colisiones con los bordes de la ventana
            if (position.x <= 0 || position.x >= windowWidth) {
                direction.x = -direction.x; // Invertir dirección horizontal
                bounceCount++;
            }
            if (position.y <= 0 || position.y >= windowHeight) {
                direction.y = -direction.y; // Invertir dirección vertical
                bounceCount++;
            }

            // Desactivar la bala si ha alcanzado el número máximo de rebotes
            if (bounceCount >= MAX_BOUNCES) {
                active = false;
            }


            shape.setPosition(position);
        }
    }


    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(shape);
        }
    }


    bool isActive() const { return active; }
    void deactivate() { active = false; }
    sf::Vector2f getPosition() const { return position; }
    Player* getOwner() const { return owner; }
};

class TrajectoryLine {
    private:
        sf::VertexArray line;
        bool visible;

    public:
        TrajectoryLine() : line(sf::Lines, 2), visible(false) {
            line[0].color = sf::Color(255, 0, 0, 128); // Semi-transparent red
            line[1].color = sf::Color(255, 0, 0, 128);
        }

        void update(sf::Vector2f& start, const sf::Vector2f& end) {
            start.x = start.x + 15;
            start.y = start.y + 15;
            line[0].position = start;
            line[1].position = end;
            visible = true;
        }

        void draw(sf::RenderWindow& window) {
            if (visible) {
                window.draw(line);
            }
        }

        void hide() {
            visible = false;
        }
    };

#endif //BULLET_H
