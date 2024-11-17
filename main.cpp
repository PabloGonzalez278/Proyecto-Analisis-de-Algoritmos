#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <thread>
#include <chrono>
#if defined(_WIN32)
#include <windows.h>
#elif defined(_APPLE_)
#include <cstdlib>
#elif defined(_linux_)
#include <cstdlib>
#endif

enum Color { ROJO, AZUL, VERDE, AMARILLO, MORADO, NARANJA, AGUAMARINA, ROSA, NINGUNO };
enum Tipo { NUMERO, PIERDE_TURNO, REVERSA, ROBA_UNO, FLIP, COMODIN, COMODIN_ROBA_DOS, ROBA_CINCO, SALTO_A_TODOS, ROBO_SALVAJE };

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
#elif defined(_APPLE_)
    std::string command = "open " + url;
    system(command.c_str());
#elif defined(_linux_)
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
        case ROBA_UNO: return "Roba Uno";
        case FLIP: return "Flip";
        case COMODIN: return "Comodin";
        case COMODIN_ROBA_DOS: return "Comodin Roba Dos";
        case ROBA_CINCO: return "Roba Cinco";
        case SALTO_A_TODOS: return "Salto a Todos";
        case ROBO_SALVAJE: return "Robo Salvaje";
        default: return "Desconocido";
    }
}

void setColor(Color color) {
    switch (color) {
    
    }
}

void resetColor() {
    
}

std::vector<Carta> crearMazo() {
    std::vector<Carta> mazo;
    for (int c = ROJO; c <= AMARILLO; ++c) {
        for (int i = 0; i <= 9; ++i) {
            mazo.push_back({
                {static_cast<Color>(c), NUMERO, i},
                {static_cast<Color>(c + 4), NUMERO, i},
                false
            });
        }
        mazo.push_back({
            {static_cast<Color>(c), PIERDE_TURNO, -1},
            {static_cast<Color>(c + 4), SALTO_A_TODOS, -1},
            false
        });
        mazo.push_back({
            {static_cast<Color>(c), REVERSA, -1},
            {static_cast<Color>(c + 4), REVERSA, -1},
            false
        });
        mazo.push_back({
            {static_cast<Color>(c), ROBA_UNO, -1},
            {static_cast<Color>(c + 4), ROBA_CINCO, -1},
            false
        });
        mazo.push_back({
            {static_cast<Color>(c), FLIP, -1},
            {static_cast<Color>(c + 4), FLIP, -1},
            false
        });
    }
    for (int i = 0; i < 4; ++i) {
        mazo.push_back({
            {NINGUNO, COMODIN, -1},
            {NINGUNO, ROBO_SALVAJE, -1},
            false
        });
        mazo.push_back({
            {NINGUNO, COMODIN_ROBA_DOS, -1},
            {NINGUNO, COMODIN, -1},
            false
        });
    }
    return mazo;
}

void barajarMazo(std::vector<Carta>& mazo) {
    unsigned int seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::mt19937 g(seed);
    std::shuffle(mazo.begin(), mazo.end(), g);
}

bool esCartaJugable(const Carta& cartaJugada, const Carta& cartaSuperior) {
    const Lado& ladoActualJugada = cartaJugada.estaVolteada ? cartaJugada.ladoOscuro : cartaJugada.ladoClaro;
    const Lado& ladoActualSuperior = cartaSuperior.estaVolteada ? cartaSuperior.ladoOscuro : cartaSuperior.ladoClaro;

    if (ladoActualJugada.tipo == ROBO_SALVAJE || ladoActualJugada.tipo == COMODIN || ladoActualJugada.tipo == COMODIN_ROBA_DOS) {
        return true;
    }

    if (ladoActualJugada.tipo != NUMERO && ladoActualJugada.tipo == ladoActualSuperior.tipo) {
        return true;
    }

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
    modoOscuro = !modoOscuro;
    for (Jugador& jugador : jugadores) {
        for (Carta& carta : jugador.mano) {
            carta.estaVolteada = modoOscuro;
        }
    }
    for (Carta& carta : pilaDescarte) {
        carta.estaVolteada = modoOscuro;
    }
}

void robarCarta(Jugador& jugador, std::vector<Carta>& mazo, int cantidad = 1) {
    for (int i = 0; i < cantidad && !mazo.empty(); ++i) {
        Carta cartaRobada = mazo.back();
        cartaRobada.estaVolteada = modoOscuro; // Asegura que la carta robada esté en el mismo estado que el modoOscuro
        jugador.mano.push_back(cartaRobada);
        mazo.pop_back();
    }
    std::cout << jugador.nombre << " ha robado " << cantidad << " carta(s).\n";
}

void aplicarEfectoCartaEspecial(const Carta& cartaEspecial, int& indiceJugadorActual, std::vector<Jugador>& jugadores, std::vector<Carta>& mazo, bool& direccionNormal, bool& turnoContinuo) {
    int siguienteJugador = (indiceJugadorActual + (direccionNormal ? 1 : -1) + jugadores.size()) % jugadores.size();

    // Determina si la carta está en el lado claro o en el lado oscuro
    const Lado& ladoActual = cartaEspecial.estaVolteada ? cartaEspecial.ladoOscuro : cartaEspecial.ladoClaro;

    // Aplica el efecto de la carta en el lado correspondiente
    switch (ladoActual.tipo) {
        case ROBA_UNO:
            robarCarta(jugadores[siguienteJugador], mazo);
            indiceJugadorActual = siguienteJugador;
            break;
        case PIERDE_TURNO:
            indiceJugadorActual = siguienteJugador; // Salta al siguiente jugador
            break;
        case REVERSA:
            direccionNormal = !direccionNormal; // Cambia la dirección del juego
            std::cout << "¡Direccion cambiada!\n";
            break;
        case COMODIN_ROBA_DOS:
            // Efecto principal de "roba dos"
            std::cout << jugadores[siguienteJugador].nombre << " tiene que robar 2 cartas... ";

            // Preguntar al siguiente jugador si desea desafiar
            std::cout << jugadores[siguienteJugador].nombre << ", ¿quieres desafiar a " << jugadores[indiceJugadorActual].nombre << "? (s/n): ";
            char respuesta;
            std::cin >> respuesta;

            if (respuesta == 's' || respuesta == 'S') {
                // Verificar si el jugador actual tenía cartas del mismo color que la del mazo de descarte
                bool tieneColorIgual = false;
                Color colorDescarte = cartaEspecial.estaVolteada ? cartaEspecial.ladoOscuro.color : cartaEspecial.ladoClaro.color;

                for (const Carta& carta : jugadores[indiceJugadorActual].mano) {
                    const Lado& ladoMano = carta.estaVolteada ? carta.ladoOscuro : carta.ladoClaro;
                    if (ladoMano.color == colorDescarte) {
                        tieneColorIgual = true;
                        break;
                    }
                }

                if (tieneColorIgual) {
                    // El jugador actual jugó la carta ilegalmente
                    std::cout << jugadores[indiceJugadorActual].nombre << " tenia cartas del color " << obtenerNombreColor(colorDescarte) << " y jugo ilegalmente.\n";
                    std::cout << jugadores[indiceJugadorActual].nombre << " roba 2 cartas como penalizacion.\n";
                    robarCarta(jugadores[indiceJugadorActual], mazo, 2);
                } else {
                    // El jugador actual no tenía cartas del color, el retador pierde el desafío
                    std::cout << jugadores[siguienteJugador].nombre << " desafio incorrectamente. ¡Debe robar 4 cartas!\n";
                    robarCarta(jugadores[siguienteJugador], mazo, 4);
                }
            } else {
                // Si no hay desafío, aplica el efecto normal
                robarCarta(jugadores[siguienteJugador], mazo, 2);
            }

            indiceJugadorActual = siguienteJugador;
            break;
        case ROBA_CINCO:
            robarCarta(jugadores[siguienteJugador], mazo, 5);
            indiceJugadorActual = siguienteJugador;
            break;
        case SALTO_A_TODOS:
            std::cout << "¡SALTO A TODOS jugado! El jugador actual puede jugar otra carta.\n";
            turnoContinuo = true; // Permite que el jugador juegue otra carta en el mismo turno
            break;
        case ROBO_SALVAJE: {
            // Selección de color solo una vez
            Color colorElegido = seleccionarColor(!cartaEspecial.estaVolteada);
            std::cout << "Color elegido para el ROBO SALVAJE: " << obtenerNombreColor(colorElegido) << std::endl;

            // El jugador debe robar cartas hasta obtener una del color elegido
            bool cartaDelColorEncontrada = false;
            while (!cartaDelColorEncontrada && !mazo.empty()) {
                robarCarta(jugadores[siguienteJugador], mazo);
                const Carta& cartaRobada = jugadores[siguienteJugador].mano.back();
                Color colorCartaRobada = cartaRobada.estaVolteada ? cartaRobada.ladoOscuro.color : cartaRobada.ladoClaro.color;

                if (colorCartaRobada == colorElegido) {
                    std::cout << jugadores[siguienteJugador].nombre << " ha robado una carta del color " << obtenerNombreColor(colorElegido) << ".\n";
                    cartaDelColorEncontrada = true;
                }
            }
            break;
        }
        default:
            break;
    }
}

void imprimirCarta(const Carta& carta) {
    const Lado& ladoActual = carta.estaVolteada ? carta.ladoOscuro : carta.ladoClaro;

    if ((ladoActual.tipo == COMODIN || ladoActual.tipo == COMODIN_ROBA_DOS) && ladoActual.color != NINGUNO) {
        std::cout << "Color elegido para el comodin: " << obtenerNombreColor(ladoActual.color) << std::endl;
    } else {
        if (ladoActual.tipo == COMODIN || ladoActual.tipo == COMODIN_ROBA_DOS) {
            std::cout << obtenerNombreTipo(ladoActual.tipo, ladoActual.numero) << std::endl;
        } else {
            setColor(ladoActual.color);
            std::cout << obtenerNombreColor(ladoActual.color) << " "
                      << obtenerNombreTipo(ladoActual.tipo, ladoActual.numero) << std::endl;
            resetColor();
        }
    }
}

void mostrarCartasJugadores(const std::vector<Jugador>& jugadores) {
    for (const Jugador& jugador : jugadores) {
        std::cout << jugador.nombre << " tiene " << jugador.mano.size() << " cartas." << std::endl;
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

void turnoJugador(Jugador& jugador, std::vector<Carta>& mazo, std::vector<Carta>& pilaDescarte, Carta& cartaSuperior, std::vector<Jugador>& jugadores, int& indiceJugadorActual, bool& direccionNormal, bool& turnoContinuo) {
    std::cout << "Carta en el mazo de descarte: ";
    imprimirCarta(cartaSuperior);

    std::cout << "Tu mano:\n";
    for (size_t i = 0; i < jugador.mano.size(); ++i) {
        std::cout << i + 1 << ". ";
        imprimirCarta(jugador.mano[i]);
    }

    // Verificar si el jugador tiene exactamente dos cartas y darle la opción de decir "UNO"
    bool dijoUno = false;
    if (jugador.mano.size() == 2) {
        std::string decirUno;
        std::cout << jugador.nombre << ", tienes dos cartas. ¿Quieres decir 'UNO'? (s/n): ";
        std::cin >> decirUno;

        // Marca si el jugador dijo "UNO"
        if (decirUno == "s" || decirUno == "S") {
            dijoUno = true;
            std::cout << jugador.nombre << " dijo 'UNO'. ¡Puedes jugar tu carta!\n";
        }
    }

    int eleccion;
    std::cout << "Elige una carta para jugar (1-" << jugador.mano.size() << ") o 0 para robar: ";
    std::cin >> eleccion;

    if (eleccion == 0) {
        robarCarta(jugador, mazo);
    } else if (eleccion > 0 && static_cast<size_t>(eleccion) <= jugador.mano.size()) {
        Carta cartaElegida = jugador.mano[eleccion - 1];
        if (esCartaJugable(cartaElegida, cartaSuperior)) {
            pilaDescarte.push_back(cartaSuperior);
            cartaSuperior = cartaElegida;

            if ((cartaElegida.ladoClaro.tipo == COMODIN || cartaElegida.ladoClaro.tipo == COMODIN_ROBA_DOS ||
                 cartaElegida.ladoOscuro.tipo == COMODIN) &&
                !turnoContinuo) {

                bool ladoClaro = !cartaElegida.estaVolteada;
                Color nuevoColor = seleccionarColor(ladoClaro);
                if (modoOscuro) {
                    cartaSuperior.ladoOscuro.color = nuevoColor;
                } else {
                    cartaSuperior.ladoClaro.color = nuevoColor;
                }
                std::cout << "Color elegido para el comodin: ";
                imprimirCarta(cartaSuperior);
            }

            if (cartaElegida.ladoClaro.tipo == FLIP || cartaElegida.ladoOscuro.tipo == FLIP) {
                std::cout << "¡Carta FLIP jugada! Todas las cartas han sido volteadas.\n";
                voltearTodasLasCartas(jugadores, pilaDescarte);
                voltearCarta(cartaSuperior);
            }

            jugador.mano.erase(jugador.mano.begin() + (eleccion - 1));
            std::cout << "Has jugado una carta.\n";
            aplicarEfectoCartaEspecial(cartaElegida, indiceJugadorActual, jugadores, mazo, direccionNormal, turnoContinuo);

            // Si al final del turno el jugador tiene solo una carta y no dijo "UNO", se le penaliza
            if (jugador.mano.size() == 1 && !dijoUno) {
                std::cout << jugador.nombre << " no dijo 'UNO' y debe robar dos cartas como penalizacion.\n";
                robarCarta(jugador, mazo, 2);
            }
        } else {
            std::cout << "\nNo puedes jugar esa carta. Debe coincidir en color o numero con la carta en el mazo de descarte.\n\n";
            turnoJugador(jugador, mazo, pilaDescarte, cartaSuperior, jugadores, indiceJugadorActual, direccionNormal, turnoContinuo);
        }
    } else {
        std::cout << "Opcion no valida. Intentalo de nuevo.\n";
        turnoJugador(jugador, mazo, pilaDescarte, cartaSuperior, jugadores, indiceJugadorActual, direccionNormal, turnoContinuo);
    }
}


   void turnoBot(Jugador& jugador, std::vector<Carta>& mazo, std::vector<Carta>& pilaDescarte, Carta& cartaSuperior, int& indiceJugadorActual, std::vector<Jugador>& jugadores, bool& direccionNormal, bool& turnoContinuo) {
    std::cout << jugador.nombre << " esta jugando...\n";

    // Buscar una carta jugable en la mano del bot
    bool cartaJugableEncontrada = false;
    for (size_t i = 0; i < jugador.mano.size(); ++i) {
        if (esCartaJugable(jugador.mano[i], cartaSuperior)) {
            Carta cartaElegida = jugador.mano[i];
            pilaDescarte.push_back(cartaSuperior);
            cartaSuperior = cartaElegida;

            // Si la carta es un comodín o cambio de color, el bot elige un color aleatorio
            if (cartaElegida.ladoClaro.tipo == COMODIN || cartaElegida.ladoClaro.tipo == COMODIN_ROBA_DOS || cartaElegida.ladoOscuro.tipo == ROBO_SALVAJE) {
                bool ladoClaro = !cartaElegida.estaVolteada;
                Color colorElegido = static_cast<Color>(std::rand() % 4 + (ladoClaro ? ROJO : MORADO)); // Selección aleatoria de color
                if (modoOscuro) {
                    cartaSuperior.ladoOscuro.color = colorElegido;
                } else {
                    cartaSuperior.ladoClaro.color = colorElegido;
                }
                std::cout << jugador.nombre << " ha elegido el color " << obtenerNombreColor(colorElegido) << " para el comodin.\n";
            }

            if (cartaElegida.ladoClaro.tipo == FLIP || cartaElegida.ladoOscuro.tipo == FLIP) {
                std::cout << "¡Carta FLIP jugada por " << jugador.nombre << "! Todas las cartas han sido volteadas.\n";
                voltearTodasLasCartas(jugadores, pilaDescarte);
                voltearCarta(cartaSuperior);
            }

            jugador.mano.erase(jugador.mano.begin() + i);
            std::cout << jugador.nombre << " ha jugado una carta.\n";

            // Aplicar efecto de la carta especial si corresponde
            aplicarEfectoCartaEspecial(cartaElegida, indiceJugadorActual, jugadores, mazo, direccionNormal, turnoContinuo);

            cartaJugableEncontrada = true;
            break;
        }
    }

    // Si no encuentra una carta jugable, roba una carta
    if (!cartaJugableEncontrada) {
        robarCarta(jugador, mazo);
        std::cout << jugador.nombre << " no tenia cartas jugables y ha robado una carta.\n";
    }

    // Comprobación adicional para el bot diciendo "UNO" si solo le queda una carta
    if (jugador.mano.size() == 1) {
        std::cout << jugador.nombre << " dice ¡UNO!\n";
    }
}

void cicloJuego(std::vector<Jugador>& jugadores, std::vector<Carta>& mazo) {
    std::vector<Carta> pilaDescarte;
    pilaDescarte.push_back(mazo.back());
    mazo.pop_back();
    Carta cartaSuperior = pilaDescarte.back();

    int indiceJugadorActual = 0;
    bool direccionNormal = true;
    bool turnoContinuo = false; // Se mantiene fuera del bucle para recordar el estado

    while (true) {
        Jugador& jugadorActual = jugadores[indiceJugadorActual];
        std::cout << "\nTurno de: " << jugadorActual.nombre << "\n";
        mostrarCartasJugadores(jugadores);

        if (jugadorActual.esSintetico) {
            // Llamada corregida
            turnoBot(jugadorActual, mazo, pilaDescarte, cartaSuperior, indiceJugadorActual, jugadores, direccionNormal, turnoContinuo);
        } else {
            turnoJugador(jugadorActual, mazo, pilaDescarte, cartaSuperior, jugadores, indiceJugadorActual, direccionNormal, turnoContinuo);
        }

        if (jugadorActual.mano.empty()) {
            std::cout << jugadorActual.nombre << " ha ganado el juego!\n";
            break;
        }

        // Si turnoContinuo es falso, cambia de jugador
        if (!turnoContinuo) {
            indiceJugadorActual = (indiceJugadorActual + (direccionNormal ? 1 : -1) + jugadores.size()) % jugadores.size();
        } else {
            // Resetea turnoContinuo para que no se repita automáticamente en el próximo turno
            turnoContinuo = false;
        }
    }
}

int main() {
    unsigned int seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::srand(seed);
    int opcion;
    bool iniciarJuego = false;
    bool jugarDeNuevo = true;

    while (jugarDeNuevo) {
        // Menú principal para ver reglas, iniciar juego o salir
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
                std::cout << "Opcion no valida. Intentalo de nuevo.\n";
            }
        }

        // Configuración de jugadores
        std::vector<Jugador> jugadores;
        int numJugadores;

        std::cout << "Introduce el numero de jugadores (2-10): ";
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

        // Crear y repartir el mazo
        std::vector<Carta> mazo = crearMazo();
        barajarMazo(mazo);
        repartirCartas(jugadores, mazo, 7);

        // Iniciar ciclo de juego
        cicloJuego(jugadores, mazo);

        // Preguntar si desean jugar de nuevo
        char respuesta;
        std::cout << "¿Quiere jugar de nuevo? (s/n): ";
        std::cin >> respuesta;

        if (respuesta == 's' || respuesta == 'S') {
            iniciarJuego = false; // Reiniciar el juego
        } else {
            jugarDeNuevo = false; // Salir del bucle y finalizar el programa
            std::cout << "Gracias por jugar. ¡Hasta la proxima!\n";
        }
    }

    return 0;
}
