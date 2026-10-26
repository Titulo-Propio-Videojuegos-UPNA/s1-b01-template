// Bucle de paso fijo (1/120 s) con acumulador; dibujo con interpolación. SFML 3.1
#include <SFML/Graphics.hpp>
struct Ball { float x, y, vx, vy; };
static void step(Ball& b, float dt) {
    b.x += b.vx * dt; b.y += b.vy * dt;
    if (b.x < 0 || b.x > 800) b.vx = -b.vx;
    if (b.y < 0 || b.y > 450) b.vy = -b.vy;
}
int main() {
    sf::RenderWindow win(sf::VideoMode({800, 450}), "J1 - paso fijo");
    const float dt = 1.f / 120.f; float acc = 0; sf::Clock clock;
    Ball b{400, 225, 300, 200}, last = b;
    sf::CircleShape c(10.f); c.setFillColor(sf::Color(190, 33, 55)); c.setOrigin({10.f, 10.f});
    while (win.isOpen()) {
        while (const std::optional ev = win.pollEvent())
            if (ev->is<sf::Event::Closed>()) win.close();
        acc += clock.restart().asSeconds();
        while (acc >= dt) { last = b; step(b, dt); acc -= dt; }
        float a = acc / dt;
        c.setPosition({last.x + (b.x - last.x) * a, last.y + (b.y - last.y) * a});
        win.clear(sf::Color::White); win.draw(c); win.display();
    }
}
