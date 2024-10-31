#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <thread>
#include <chrono>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <cstdlib>
#elif defined(__linux__)
#include <cstdlib>
#endif

enum Color { ROJO, AZUL, VERDE, AMARILLO, MORADO, NARANJA, AGUAMARINA, ROSA, NINGUNO };
enum Tipo { NUMERO, PIERDE_TURNO, REVERSA, ROBA_DOS, FLIP, ROBA_CUATRO, COMODIN, COMODIN_ROBA_CUATRO };

bool modoOscuro = false;

struct Lado {
    Color color;
    Tipo tipo;
    int numero;
};

struct Carta {
    Lado ladoClaro;
    Lado ladoOscuro;
    bool estaVolteada;
};

struct Jugador {
    std::string nombre;
    std::vector<Carta> mano;
    bool esSintetico;
};

void abrirEnlace(const std::string& url) {
#if defined(_WIN32)
    ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::string command = "open " + url;
    system(command.c_str());
#elif defined(__linux__)
    std::string command = "xdg-open " + url;
    system(command.c_str());
#endif
}

void mostrarReglas() {
    std::string url = "https://www.unorules.com/uno-flip-rules/";
    std::cout << "Abriendo las reglas en tu navegador web...\n";
    abrirEnlace(url);
}

std::string obtenerNombreColor(Color color) {
    switch (color) {
        case ROJO: return "Rojo";
        case AZUL: return "Azul";
        case VERDE: return "Verde";
        case AMARILLO: return "Amarillo";
        case MORADO: return "Morado";
        case NARANJA: return "Naranja";
        case AGUAMARINA: return "Aguamarina";
        case ROSA: return "Rosa";
        default: return "Ninguno";
    }
}

std::string obtenerNombreTipo(Tipo tipo, int numero) {
    switch (tipo) {
        case NUMERO: return "Numero " + std::to_string(numero);
        case PIERDE_TURNO: return "Pierde Turno";
        case REVERSA: return "Reversa";
        case ROBA_DOS: return "Roba Dos";
        case FLIP: return "Flip";
        case ROBA_CUATRO: return "Roba Cuatro";
        case COMODIN: return "Comodin";
        case COMODIN_ROBA_CUATRO: return "Comodin Roba Cuatro";
        default: return "Desconocido";
    }
}

void setColor(Color color) {
    switch (color) {
        case ROJO: std::cout << "\033[31m"; break;
        case VERDE: std::cout << "\033[32m"; break;
        case AZUL: std::cout << "\033[34m"; break;
        case AMARILLO: std::cout << "\033[33m"; break;
        case MORADO: std::cout << "\033[35m"; break;
        case NARANJA: std::cout << "\033[33m"; break;
        case AGUAMARINA: std::cout << "\033[36m"; break;
        case ROSA: std::cout << "\033[91m"; break;
        default: std::cout << "\033[0m";
    }
}

void resetColor() {
    std::cout << "\033[0m";
}

std::vector<Carta> crearMazo() {
    std::vector<Carta> mazo;
    for (int c = ROJO; c <= AMARILLO; ++c) {
        for (int i = 0; i <= 9; ++i) {
            mazo.push_back({{static_cast<Color>(c), NUMERO, i}, {static_cast<Color>(c + 4), NUMERO, i}, false});
        }
        mazo.push_back({{static_cast<Color>(c), PIERDE_TURNO, -1}, {static_cast<Color>(c + 4), PIERDE_TURNO, -1}, false});
        mazo.push_back({{static_cast<Color>(c), REVERSA, -1}, {static_cast<Color>(c + 4), REVERSA, -1}, false});
        mazo.push_back({{static_cast<Color>(c), ROBA_DOS, -1}, {static_cast<Color>(c + 4), ROBA_DOS, -1}, false});
        mazo.push_back({{static_cast<Color>(c), FLIP, -1}, {static_cast<Color>(c + 4), FLIP, -1}, false});
    }
    for (int i = 0; i < 4; ++i) {
        mazo.push_back({{NINGUNO, COMODIN, -1}, {NINGUNO, COMODIN_ROBA_CUATRO, -1}, false});
        mazo.push_back({{NINGUNO, COMODIN_ROBA_CUATRO, -1}, {NINGUNO, COMODIN, -1}, false});
    }
    return mazo;
}

void barajarMazo(std::vector<Carta>& mazo) {
    unsigned int seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::mt19937 g(seed);
    std::shuffle(mazo.begin(), mazo.end(), g);
}

// Función para verificar si una carta es jugable sobre otra, solo considerando color o número
bool esCartaJugable(const Carta& cartaJugada, const Carta& cartaSuperior) {
    const Lado& ladoActualJugada = cartaJugada.estaVolteada ? cartaJugada.ladoOscuro : cartaJugada.ladoClaro;
    const Lado& ladoActualSuperior = cartaSuperior.estaVolteada ? cartaSuperior.ladoOscuro : cartaSuperior.ladoClaro;

    // Permitir jugar si es un comodín
    if (ladoActualJugada.tipo == COMODIN || ladoActualJugada.tipo == COMODIN_ROBA_CUATRO) {
        return true;
    }

    // Permitir jugar si coincide en color o número
    return ladoActualJugada.color == ladoActualSuperior.color ||
           (ladoActualJugada.tipo == NUMERO && ladoActualJugada.numero == ladoActualSuperior.numero);
}


Color seleccionarColor(bool ladoClaro) {
    int colorSeleccionado;
    if (ladoClaro) {
        std::cout << "Selecciona el color: 1. Rojo  2. Azul  3. Verde  4. Amarillo\n";
    } else {
        std::cout << "Selecciona el color: 1. Morado  2. Rosa  3. Aguamarina  4. Naranja\n";
    }
    std::cin >> colorSeleccionado;
    if (ladoClaro) {
        switch (colorSeleccionado) {
            case 1: return ROJO;
            case 2: return AZUL;
            case 3: return VERDE;
            case 4: return AMARILLO;
        }
    } else {
        switch (colorSeleccionado) {
            case 1: return MORADO;
            case 2: return ROSA;
            case 3: return AGUAMARINA;
            case 4: return NARANJA;
        }
    }
    return NINGUNO;
}

void voltearCarta(Carta& carta) {
    carta.estaVolteada = !carta.estaVolteada;
}

void voltearTodasLasCartas(std::vector<Jugador>& jugadores, std::vector<Carta>& pilaDescarte) {
    modoOscuro = !modoOscuro; // Toggle the global mode
    for (Jugador& jugador : jugadores) {
        for (Carta& carta : jugador.mano) {
            carta.estaVolteada = modoOscuro; // Set all cards based on the new mode
        }
    }
    for (Carta& carta : pilaDescarte) {
        carta.estaVolteada = modoOscuro;
    }
}

void robarCarta(Jugador& jugador, std::vector<Carta>& mazo) {
    Carta cartaRobada = mazo.back();
    cartaRobada.estaVolteada = modoOscuro; // Set the flipped status according to the current mode
    jugador.mano.push_back(cartaRobada);
    mazo.pop_back();
    std::cout << jugador.nombre << " ha robado una carta.\n";
}

void imprimirCarta(const Carta& carta) {
    const Lado& ladoActual = carta.estaVolteada ? carta.ladoOscuro : carta.ladoClaro;

    // Verificar si la carta es un comodín y tiene un color asignado
    if ((ladoActual.tipo == COMODIN || ladoActual.tipo == COMODIN_ROBA_CUATRO) && ladoActual.color != NINGUNO) {
        std::cout << "Color elegido para el comodín: " << obtenerNombreColor(ladoActual.color) << std::endl;
    } else {
        if (ladoActual.tipo == COMODIN || ladoActual.tipo == COMODIN_ROBA_CUATRO) {
            std::cout << obtenerNombreTipo(ladoActual.tipo, ladoActual.numero) << std::endl;
        } else {
            setColor(ladoActual.color);
            std::cout << obtenerNombreColor(ladoActual.color) << " "
                      << obtenerNombreTipo(ladoActual.tipo, ladoActual.numero) << std::endl;
            resetColor();
        }
    }
}

void repartirCartas(std::vector<Jugador>& jugadores, std::vector<Carta>& mazo, int numCartas) {
    for (int i = 0; i < numCartas; ++i) {
        for (Jugador& jugador : jugadores) {
            jugador.mano.push_back(mazo.back());
            mazo.pop_back();
        }
    }
}

void turnoJugador(Jugador& jugador, std::vector<Carta>& mazo, std::vector<Carta>& pilaDescarte, Carta& cartaSuperior, std::vector<Jugador>& jugadores) {
    std::cout << "Carta en el mazo de descarte: ";
    imprimirCarta(cartaSuperior);

    std::cout << "Tu mano:\n";
    for (size_t i = 0; i < jugador.mano.size(); ++i) {
        std::cout << i + 1 << ". ";
        imprimirCarta(jugador.mano[i]);
    }

    int eleccion;
    std::cout << "Elige una carta para jugar (1-" << jugador.mano.size() << ") o 0 para robar: ";
    std::cin >> eleccion;

    if (eleccion == 0) {
        robarCarta(jugador, mazo); // Usa la función modificada para robar
    } else if (eleccion > 0 && eleccion <= jugador.mano.size()) {
        Carta cartaElegida = jugador.mano[eleccion - 1];
        if (esCartaJugable(cartaElegida, cartaSuperior)) {
            pilaDescarte.push_back(cartaSuperior); // Coloca la carta actual en la pila de descarte
            cartaSuperior = cartaElegida; // Actualiza la carta superior con la carta jugada

            // Permitir selección de color si es un comodín y mostrar el color elegido en la carta de descarte
            if (cartaElegida.ladoClaro.tipo == COMODIN || cartaElegida.ladoClaro.tipo == COMODIN_ROBA_CUATRO ||
                cartaElegida.ladoOscuro.tipo == COMODIN || cartaElegida.ladoOscuro.tipo == COMODIN_ROBA_CUATRO) {
                bool ladoClaro = !cartaElegida.estaVolteada;
                Color nuevoColor = seleccionarColor(ladoClaro);

                // Asigna el color seleccionado al lado actual (oscuro o claro) y muestra el comodín con el color elegido
                if (modoOscuro) {
                    cartaSuperior.ladoOscuro.color = nuevoColor;
                } else {
                    cartaSuperior.ladoClaro.color = nuevoColor;
                }

                // Mostrar la carta de descarte con el nuevo color
                std::cout << "Color elegido para el comodín: ";
                imprimirCarta(cartaSuperior);
            }

            // Si la carta es de tipo FLIP, voltear todas las cartas y la carta superior
            if (cartaElegida.ladoClaro.tipo == FLIP || cartaElegida.ladoOscuro.tipo == FLIP) {
                std::cout << "¡Carta FLIP jugada! Todas las cartas han sido volteadas.\n";
                voltearTodasLasCartas(jugadores, pilaDescarte);
                voltearCarta(cartaSuperior); // Voltear también la carta superior
            }

            jugador.mano.erase(jugador.mano.begin() + (eleccion - 1));
            std::cout << "Has jugado una carta.\n";
        } else {
            std::cout << "\nNo puedes jugar esa carta. Debe coincidir en color o número con la carta en el mazo de descarte.\n\n";
            turnoJugador(jugador, mazo, pilaDescarte, cartaSuperior, jugadores); // Llamada recursiva para intentar de nuevo
        }
    } else {
        std::cout << "Opción no válida. Inténtalo de nuevo.\n";
        turnoJugador(jugador, mazo, pilaDescarte, cartaSuperior, jugadores); // Llamada recursiva para opción no válida
    }
}


void turnoBot(Jugador& jugador, std::vector<Carta>& mazo, std::vector<Carta>& pilaDescarte, Carta& cartaSuperior) {
    std::cout << jugador.nombre << " está jugando...\n";
    for (size_t i = 0; i < jugador.mano.size(); ++i) {
        if (esCartaJugable(jugador.mano[i], cartaSuperior)) {
            Carta cartaElegida = jugador.mano[i];
            pilaDescarte.push_back(cartaSuperior);
            cartaSuperior = cartaElegida;
            jugador.mano.erase(jugador.mano.begin() + i);
            std::cout << jugador.nombre << " ha jugado una carta.\n";
            return;
        }
    }
    jugador.mano.push_back(mazo.back());
    mazo.pop_back();
    std::cout << jugador.nombre << " ha robado una carta.\n";
}

void mostrarCartasJugadores(const std::vector<Jugador>& jugadores) {
    for (const Jugador& jugador : jugadores) {
        std::cout << jugador.nombre << " tiene " << jugador.mano.size() << " cartas." << std::endl;
    }
}

void cicloJuego(std::vector<Jugador>& jugadores, std::vector<Carta>& mazo) {
    std::vector<Carta> pilaDescarte;
    pilaDescarte.push_back(mazo.back());
    mazo.pop_back();
    Carta cartaSuperior = pilaDescarte.back();

    int indiceJugadorActual = 0;
    while (true) {
        Jugador& jugadorActual = jugadores[indiceJugadorActual];
        std::cout << "\nTurno de: " << jugadorActual.nombre << "\n";
        mostrarCartasJugadores(jugadores);

        if (jugadorActual.esSintetico) {
            turnoBot(jugadorActual, mazo, pilaDescarte, cartaSuperior);
        } else {
            turnoJugador(jugadorActual, mazo, pilaDescarte, cartaSuperior, jugadores);
        }

        if (jugadorActual.mano.empty()) {
            std::cout << jugadorActual.nombre << " ha ganado el juego!\n";
            break;
        }
        indiceJugadorActual = (indiceJugadorActual + 1) % jugadores.size();
    }
}

int main() {
    unsigned int seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::srand(seed);
    int opcion;
    bool iniciarJuego = false;

    while (!iniciarJuego) {
        std::cout << "Bienvenido a UNO Flip\n";
        std::cout << "1. Ver reglas del juego\n";
        std::cout << "2. Iniciar juego\n";
        std::cout << "3. Salir\n";
        std::cout << "Selecciona una opcion: ";
        std::cin >> opcion;

        if (opcion == 1) {
            mostrarReglas();
        } else if (opcion == 2) {
            iniciarJuego = true;
        } else if (opcion == 3) {
            return 0;
        } else {
            std::cout << "Opción no válida. Inténtalo de nuevo.\n";
        }
    }

    std::vector<Jugador> jugadores;
    int numJugadores;

    std::cout << "Introduce el número de jugadores (2-10): ";
    std::cin >> numJugadores;

    for (int i = 0; i < numJugadores; ++i) {
        Jugador jugador;
        std::cout << "Jugador " << (i + 1) << " es humano (0) o bot (1): ";
        int esBot;
        std::cin >> esBot;
        jugador.esSintetico = (esBot == 1);
        jugador.nombre = "Jugador " + std::to_string(i + 1);
        jugadores.push_back(jugador);
    }

    std::vector<Carta> mazo = crearMazo();
    barajarMazo(mazo);
    repartirCartas(jugadores, mazo, 7);
    cicloJuego(jugadores, mazo);

    return 0;
}
