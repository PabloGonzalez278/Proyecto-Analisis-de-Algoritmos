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
Color colorElegidoGlobal = NINGUNO;

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
    int puntos = 0; // Puntos acumulados del jugador
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

    std::vector<Carta> crearMazo() {
    std::vector<Carta> mazo;

    // Usar un generador aleatorio para decidir la cantidad de cada carta
    std::mt19937 gen(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<> distColorClaro(0, 3); // Colores disponibles para lado claro (AZUL, VERDE, ROJO, AMARILLO)
    std::uniform_int_distribution<> distColorOscuro(4, 7); // Colores disponibles para lado oscuro (MORADO, NARANJA, AGUAMARINA, ROSA)
    std::uniform_int_distribution<> distNumero(1, 9); // Números del 1 al 9
    std::uniform_int_distribution<> distTipo(0, 9);  // Tipos disponibles

    // Generar cartas para cada combinación de colores (claro y oscuro)
    auto agregarCartasConLados = [&](Color colorClaro, Color colorOscuro) {
        // Números del 1 al 9, dos copias de cada número
        for (int i = 1; i <= 9; ++i) {
            for (int j = 0; j < 2; ++j) {
                mazo.push_back({
                    {colorClaro, NUMERO, i},   // Lado claro
                    {colorOscuro, NUMERO, i}, // Lado oscuro
                    false
                });
            }
        }
        // Cartas especiales: 2 por tipo
        for (int j = 0; j < 2; ++j) {
            mazo.push_back({
                {colorClaro, ROBA_UNO, -1},     // Lado claro: Toma 1
                {colorOscuro, ROBA_CINCO, -1}, // Lado oscuro: Toma 5
                false
            });
            mazo.push_back({
                {colorClaro, REVERSA, -1},     // Lado claro: Reversa
                {colorOscuro, REVERSA, -1},   // Lado oscuro: Reversa
                false
            });
            mazo.push_back({
                {colorClaro, PIERDE_TURNO, -1},      // Lado claro: Pierde Turno
                {colorOscuro, SALTO_A_TODOS, -1},   // Lado oscuro: Saltar a Todos
                false
            });
            mazo.push_back({
                {colorClaro, FLIP, -1},        // Lado claro: Flip
                {colorOscuro, FLIP, -1},      // Lado oscuro: Flip
                false
            });
        }
    };

    // Agregar cartas por color
    agregarCartasConLados(AZUL, ROSA);
    agregarCartasConLados(VERDE, AGUAMARINA);
    agregarCartasConLados(ROJO, NARANJA);
    agregarCartasConLados(AMARILLO, MORADO);

    // Comodines (4 por tipo)
    for (int i = 0; i < 4; ++i) {
        mazo.push_back({
            {NINGUNO, COMODIN, -1},          // Lado claro: Cambia color
            {NINGUNO, COMODIN, -1},          // Lado oscuro: Cambia color
            false
        });
        mazo.push_back({
            {NINGUNO, COMODIN_ROBA_DOS, -1}, // Lado claro: Comodinn roba 2
            {NINGUNO, ROBO_SALVAJE, -1},    // Lado oscuro: Robo Salvaje
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
void imprimirCarta(const Carta& carta) {
    const Lado& ladoActual = carta.estaVolteada ? carta.ladoOscuro : carta.ladoClaro;

    if ((ladoActual.tipo == COMODIN || ladoActual.tipo == COMODIN_ROBA_DOS) && ladoActual.color != NINGUNO) {
        std::cout << "Color elegido para el comodinn: " << obtenerNombreColor(ladoActual.color) << std::endl;
    } else {
        if (ladoActual.tipo == COMODIN || ladoActual.tipo == COMODIN_ROBA_DOS) {
            std::cout << obtenerNombreTipo(ladoActual.tipo, ladoActual.numero) << std::endl;
        } else {
            std::cout << obtenerNombreColor(ladoActual.color) << " "
                      << obtenerNombreTipo(ladoActual.tipo, ladoActual.numero) << std::endl;
        }
    }
}
void voltearTodasLasCartas(std::vector<Jugador>& jugadores, std::vector<Carta>& pilaDescarte) {
    // Cambia el estado global del modo oscuro
    modoOscuro = !modoOscuro;

    // Voltear todas las cartas de los jugadores
    for (Jugador& jugador : jugadores) {
        for (Carta& carta : jugador.mano) {
            carta.estaVolteada = modoOscuro;
        }
    }

    // Voltear todas las cartas en la pila de descarte
    for (Carta& carta : pilaDescarte) {
        carta.estaVolteada = modoOscuro;
    }

    std::cout << "¡Todas las cartas han sido volteadas! Ahora están en modo "
              << (modoOscuro ? "oscuro" : "claro") << ".\n";
}

void robarCarta(Jugador& jugador, std::vector<Carta>& mazo, int cantidad = 1, bool forzarOscuro = false) {
    for (int i = 0; i < cantidad && !mazo.empty(); ++i) {
        Carta cartaRobada = mazo.back();
        mazo.pop_back();

        // Siempre configura el lado oscuro si está forzado
        if (forzarOscuro) {
            cartaRobada.estaVolteada = true; // Asegurarse de que se use el lado oscuro
        } else {
            cartaRobada.estaVolteada = modoOscuro; // Usa el estado global del modo
        }

        jugador.mano.push_back(cartaRobada);

        // Verificar si el jugador es humano antes de mostrar la carta robada
        if (!jugador.esSintetico) {
            imprimirCarta(cartaRobada);
        }
    }
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
            case SALTO_A_TODOS:
            std::cout << "¡SALTO A TODOS jugado! El jugador actual puede jugar otra carta.\n";
            turnoContinuo = true; // Permite que el jugador juegue otra carta en el mismo turno
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

        case ROBO_SALVAJE:
            std::cout << jugadores[indiceJugadorActual].nombre << " ha jugado Robo Salvaje.\n";
        colorElegidoGlobal = seleccionarColor(false); // Siempre seleccionar en modo oscuro
        std::cout << "Color elegido para el Robo Salvaje: " << obtenerNombreColor(colorElegidoGlobal) << "\n";

        std::cout << jugadores[siguienteJugador].nombre << " debe robar cartas hasta que salga una del color elegido.\n";
        while (true) {
            if (mazo.empty()) {
                std::cout << "El mazo se agotó. El efecto se detiene.\n";
                break;
            }

            Carta cartaRobada = mazo.back();
            mazo.pop_back();
            cartaRobada.estaVolteada = true; // Forzar el lado oscuro
            jugadores[siguienteJugador].mano.push_back(cartaRobada);
            imprimirCarta(cartaRobada);

            const Lado& ladoRobado = cartaRobada.ladoOscuro;
            if (ladoRobado.color == colorElegidoGlobal) {
                std::cout << "¡Carta del color elegido obtenida! Efecto completado.\n";
                break;
            }
        }

        indiceJugadorActual = siguienteJugador;


    // Cambiar al siguiente jugador
    indiceJugadorActual = siguienteJugador;
    break;
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
    std::cout << "\nLe toca al jugador: " << jugador.nombre << "\n";

    // Preguntar si el jugador quiere ver sus cartas
    char respuesta;
    std::cout << "¿Quieres ver tus cartas? (S/N): ";
    std::cin >> respuesta;

    // Mostrar las cartas solo si el jugador dice "S" o "s"
    if (respuesta == 'S' || respuesta == 's') {
        std::cout << "Tu mano: \n";
        for (size_t i = 0; i < jugador.mano.size(); ++i) {
            std::cout << i + 1 << ". ";
            imprimirCarta(jugador.mano[i]);
        }
    } else {
        std::cout << "Decidiste no ver tus cartas.\n";
    }

    // Mostrar la carta actual en el mazo de descarte
    std::cout << "Carta en el mazo de descarte:  ";
    imprimirCarta(cartaSuperior);

    // Verificar si el jugador tiene exactamente dos cartas y darle la opción de decir "UNO"
    bool dijoUno = false;
    if (jugador.mano.size() == 2) {
        std::string decirUno;
        std::cout << jugador.nombre << ", tienes dos cartas. ¿Quieres decir 'UNO'? (s/n): ";
        std::cin >> decirUno;

        if (decirUno == "S" || decirUno == "s") {
            dijoUno = true;
            std::cout << jugador.nombre << " dijo 'UNO'. ¡Puedes jugar tu carta!\n";
        }
    }

    // Pedir al jugador que elija una acción
    int eleccion;
    std::cout << "Elige una carta para jugar (1-" << jugador.mano.size() << ") o 0 para robar: ";
    std::cin >> eleccion;

    if (eleccion == 0) {
        // El jugador roba una carta
        robarCarta(jugador, mazo);
        turnoContinuo = false; // Finalizar el turno después de robar
    } else if (eleccion > 0 && static_cast<size_t>(eleccion) <= jugador.mano.size()) {
        // El jugador elige jugar una carta
        Carta cartaElegida = jugador.mano[eleccion - 1];
        if (esCartaJugable(cartaElegida, cartaSuperior)) {
            pilaDescarte.push_back(cartaSuperior); // Mover la carta actual al descarte
            cartaSuperior = cartaElegida;

            // Manejar cartas especiales
            if (cartaElegida.ladoClaro.tipo == COMODIN || cartaElegida.ladoClaro.tipo == COMODIN_ROBA_DOS || cartaElegida.ladoOscuro.tipo == COMODIN) {
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

            jugador.mano.erase(jugador.mano.begin() + (eleccion - 1)); // Eliminar la carta jugada
            std::cout << "Has jugado una carta.\n";

            // Aplicar efecto de la carta especial
            aplicarEfectoCartaEspecial(cartaElegida, indiceJugadorActual, jugadores, mazo, direccionNormal, turnoContinuo);

            // Penalizar si no dijo "UNO"
            if (jugador.mano.size() == 1 && !dijoUno) {
                std::cout << jugador.nombre << " no dijo 'UNO' y debe robar dos cartas como penalización.\n";
                robarCarta(jugador, mazo, 2);
            }
        } else {
            std::cout << "No puedes jugar esa carta. Debe coincidir en color o número con la carta en el mazo de descarte.\n";
        }
    } else {
        std::cout << "Opción no válida. Inténtalo de nuevo.\n";
    }

    // Cambiar de jugador si el turno no es continuo
    if (!turnoContinuo) {
        indiceJugadorActual = (indiceJugadorActual + (direccionNormal ? 1 : -1) + jugadores.size()) % jugadores.size();
    } else {
        turnoContinuo = false; // Restablecer turno continuo para la próxima iteración
    }
}

   void turnoBot(Jugador& jugador, std::vector<Carta>& mazo, std::vector<Carta>& pilaDescarte, Carta& cartaSuperior, int& indiceJugadorActual, std::vector<Jugador>& jugadores, bool& direccionNormal, bool& turnoContinuo) {
    std::cout << "\nLe toca al jugador (Bot): " << jugador.nombre << "\n";

    // Mostrar la carta actual en el mazo de descarte
    std::cout << "Carta en el mazo de descarte: ";
    imprimirCarta(cartaSuperior);

    // Buscar una carta jugable en la mano del bot
    bool cartaJugableEncontrada = false;
    for (size_t i = 0; i < jugador.mano.size(); ++i) {
        if (esCartaJugable(jugador.mano[i], cartaSuperior)) {
            // Si encuentra una carta jugable, la juega
            Carta cartaElegida = jugador.mano[i];
            pilaDescarte.push_back(cartaSuperior);
            cartaSuperior = cartaElegida;

            // Aplicar efectos de cartas especiales
            if (cartaElegida.ladoClaro.tipo == COMODIN || cartaElegida.ladoClaro.tipo == COMODIN_ROBA_DOS || cartaElegida.ladoOscuro.tipo == ROBO_SALVAJE) {
                // Seleccionar un color aleatorio para el comodinn
                bool ladoClaro = !cartaElegida.estaVolteada;
                Color colorElegido = static_cast<Color>(std::rand() % 4 + (ladoClaro ? ROJO : MORADO));
                if (modoOscuro) {
                    cartaSuperior.ladoOscuro.color = colorElegido;
                } else {
                    cartaSuperior.ladoClaro.color = colorElegido;
                }
                std::cout << jugador.nombre << " ha elegido el color " << obtenerNombreColor(colorElegido) << " para el comodinn.\n";
            }

            if (cartaElegida.ladoClaro.tipo == FLIP || cartaElegida.ladoOscuro.tipo == FLIP) {
                std::cout << "¡Carta FLIP jugada por " << jugador.nombre << "! Todas las cartas han sido volteadas.\n";
                voltearTodasLasCartas(jugadores, pilaDescarte);
                voltearCarta(cartaSuperior);
            }

            // Eliminar la carta jugada de la mano del bot
            jugador.mano.erase(jugador.mano.begin() + i);
            std::cout << jugador.nombre << " ha jugado una carta.\n";

            // Aplicar el efecto de la carta especial si corresponde
            aplicarEfectoCartaEspecial(cartaElegida, indiceJugadorActual, jugadores, mazo, direccionNormal, turnoContinuo);

            cartaJugableEncontrada = true;
            break;
        }
    }

    // Si no encuentra una carta jugable, roba una carta
    if (!cartaJugableEncontrada) {
        robarCarta(jugador, mazo);
        std::cout << jugador.nombre << " no tenía cartas jugables y ha robado una carta.\n";
    }

    // Comprobación adicional para que el bot diga "UNO" si le queda una carta
    if (jugador.mano.size() == 1) {
        std::cout << jugador.nombre << " dice ¡UNO!\n";
    }

    // Cambiar de jugador si el turno no es continuo
    if (!turnoContinuo) {
        indiceJugadorActual = (indiceJugadorActual + (direccionNormal ? 1 : -1) + jugadores.size()) % jugadores.size();
    } else {
        turnoContinuo = false; // Restablecer turno continuo para la próxima iteración
    }
}


int calcularPuntos(const Carta& carta) {
    const Lado& ladoActual = carta.estaVolteada ? carta.ladoOscuro : carta.ladoClaro;

    switch (ladoActual.tipo) {
        case NUMERO: return ladoActual.numero;
        case ROBA_UNO: return 10;
        case ROBA_CINCO: return 20;
        case REVERSA: return 20;
        case PIERDE_TURNO: return 20;
        case SALTO_A_TODOS: return 30;
        case FLIP: return 20;
        case COMODIN: return 40;
        case COMODIN_ROBA_DOS: return 50;
        case ROBO_SALVAJE: return 60;
        default: return 0;
    }
}
int calcularPuntosJugador(const Jugador& jugador) {
    int puntos = 0;
    for (const Carta& carta : jugador.mano) {
        puntos += calcularPuntos(carta);
    }
    return puntos;
}
void aplicarEfectoInicial(Carta& cartaInicial, std::vector<Jugador>& jugadores, std::vector<Carta>& mazo, int& indiceJugadorActual, bool& direccionNormal) {
    const Lado& ladoActual = cartaInicial.estaVolteada ? cartaInicial.ladoOscuro : cartaInicial.ladoClaro;
    int siguienteJugador = (indiceJugadorActual + (direccionNormal ? 1 : -1) + jugadores.size()) % jugadores.size();

    switch (ladoActual.tipo) {


        case COMODIN_ROBA_DOS:
            std::cout << jugadores[indiceJugadorActual].nombre << " ha recibido un comodin roba dos al inicio. Debe elegir un color.\n";
            colorElegidoGlobal = seleccionarColor(!modoOscuro);
            std::cout << "El color elegido es: " << obtenerNombreColor(colorElegidoGlobal) << "\n";
            robarCarta(jugadores[siguienteJugador], mazo, 2); // El siguiente jugador roba dos cartas
            indiceJugadorActual = siguienteJugador;
            break;

        case ROBA_UNO:
            robarCarta(jugadores[siguienteJugador], mazo);
            indiceJugadorActual = siguienteJugador;
            break;

        case PIERDE_TURNO:
            indiceJugadorActual = siguienteJugador; // Salta al siguiente jugador
            break;

        case REVERSA:
            direccionNormal = !direccionNormal; // Cambia la dirección del juego
            std::cout << "¡Dirección cambiada!\n";

            // Si se invierte la dirección, el siguiente jugador es el último
            if (!direccionNormal) {
                indiceJugadorActual = jugadores.size() - 1;
            } else {
                indiceJugadorActual = siguienteJugador;
            }
            break;

        case ROBA_CINCO:
            robarCarta(jugadores[siguienteJugador], mazo, 5); // El siguiente jugador roba 5 cartas
            indiceJugadorActual = siguienteJugador;
            break;


        default:
            std::cout << "No hay efecto inicial por la carta: ";
            imprimirCarta(cartaInicial);
            break;
    }
}




void cicloJuego(std::vector<Jugador>& jugadores, std::vector<Carta>& mazo) {
    std::vector<Carta> pilaDescarte;

    // Asegurarse de que la carta inicial no sea FLIP
    while (!mazo.empty() && (mazo.back().ladoClaro.tipo == FLIP || mazo.back().ladoOscuro.tipo == FLIP)  || !mazo.empty() && (mazo.back().ladoClaro.tipo == COMODIN || mazo.back().ladoOscuro.tipo == COMODIN)) {
        std::cout << "La carta inicial era FLIP o COMODIN. Reembarajando...\n";
        barajarMazo(mazo);
    }

    pilaDescarte.push_back(mazo.back());
    mazo.pop_back();
    Carta cartaSuperior = pilaDescarte.back();

    int indiceJugadorActual = 0;
    bool direccionNormal = true;
    bool turnoContinuo = false;

    // Mostrar la carta de descarte al inicio del juego
    std::cout << "\nLa carta inicial en el mazo de descarte es: ";
    imprimirCarta(cartaSuperior);

    // Aplicar efecto inicial de la carta
    aplicarEfectoInicial(cartaSuperior, jugadores, mazo, indiceJugadorActual, direccionNormal);

    while (true) {
        Jugador& jugadorActual = jugadores[indiceJugadorActual];
        std::cout << "\nTurno de: " << jugadorActual.nombre << "\n";

        // Mostrar la carta de descarte en cada turno
        std::cout << "Carta actual en el mazo de descarte: ";
        imprimirCarta(cartaSuperior);

        mostrarCartasJugadores(jugadores);

        if (jugadorActual.esSintetico) {
            turnoBot(jugadorActual, mazo, pilaDescarte, cartaSuperior, indiceJugadorActual, jugadores, direccionNormal, turnoContinuo);
        } else {
            turnoJugador(jugadorActual, mazo, pilaDescarte, cartaSuperior, jugadores, indiceJugadorActual, direccionNormal, turnoContinuo);
        }

        if (jugadorActual.mano.empty()) {
            // El jugador ha ganado la ronda
            std::cout << jugadorActual.nombre << " ha ganado la ronda!\n";

            // Calcular puntos de la ronda
            int puntosRonda = 0;
            for (const Jugador& jugador : jugadores) {
                if (jugador.nombre != jugadorActual.nombre) {
                    puntosRonda += calcularPuntosJugador(jugador);
                }
            }

            // Sumar puntos al jugador ganador
            jugadorActual.puntos += puntosRonda;

            std::cout << jugadorActual.nombre << " tiene ahora " << jugadorActual.puntos << " puntos.\n";

            if (jugadorActual.puntos >= 500) {
                std::cout << jugadorActual.nombre << " ha alcanzado 500 puntos y gana el juego!\n";
                return; // Terminar el juego
            }

            // Reiniciar el juego
            std::cout << "Iniciando una nueva ronda...\n";
            for (Jugador& jugador : jugadores) {
                jugador.mano.clear();
            }

            mazo = crearMazo();
            barajarMazo(mazo);
            repartirCartas(jugadores, mazo, 7);

            pilaDescarte.clear();
            pilaDescarte.push_back(mazo.back());
            mazo.pop_back();
            cartaSuperior = pilaDescarte.back();

            indiceJugadorActual = 0;
            direccionNormal = true;
            turnoContinuo = false;

            std::cout << "\nLa nueva carta inicial en el mazo de descarte es: ";
            imprimirCarta(cartaSuperior);

            aplicarEfectoInicial(cartaSuperior, jugadores, mazo, indiceJugadorActual, direccionNormal);
        }

        if (!turnoContinuo) {
            indiceJugadorActual = (indiceJugadorActual + (direccionNormal ? 1 : -1) + jugadores.size()) % jugadores.size();
        } else {
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
                std::vector<Carta> mazo = crearMazo();
                barajarMazo(mazo);
std::cout << "Mazo creado con " << mazo.size() << " cartas.\n";
            } else if (opcion == 3) {
                return 0;
            } else {
                std::cout << "Opción no válida. Inténtalo de nuevo.\n";
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
            std::cout << "Gracias por jugar. ¡Hasta la próxima!\n";
        }
    }

    return 0;
}
