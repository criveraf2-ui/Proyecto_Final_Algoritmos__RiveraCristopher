#include "validaciones.h"
#include "modelos.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;

// Convierte mayusculas a minusculas sumando 32 en la tabla ASCII
string aMinusculas(string texto) {
    for (int i = 0; i < (int)texto.length(); i++) {
        if (texto[i] >= 'A' && texto[i] <= 'Z') {
            texto[i] = texto[i] + 32;
        }
    }
    return texto;
}

// Quita los espacios en blanco al inicio y al final de un texto
string recortar(const string &texto) {
    int inicio = 0;
    int fin = (int)texto.length();

    while (inicio < fin && (texto[inicio] == ' ' || texto[inicio] == '\t' || texto[inicio] == '\r')) {
        inicio++;
    }
    while (fin > inicio && (texto[fin - 1] == ' ' || texto[fin - 1] == '\t' || texto[fin - 1] == '\r')) {
        fin--;
    }
    return texto.substr(inicio, fin - inicio);
}

// Compara dos cadenas sin importar si estan en mayusculas o minusculas
bool mismoTexto(const string &a, const string &b) {
    return aMinusculas(a) == aMinusculas(b);
}

// Lee un numero entero validando que no metan letras y que este en el rango
int leerEntero(const string &mensaje, int minimo, int maximo) {
    while (true) {
        cout << mensaje;
        string linea;
        if (!getline(cin, linea)) {
            cout << "\n[!] Entrada cerrada. Fin del programa.\n";
            exit(1);
        }

        istringstream lectura(linea);
        int valor;
        char extra;

        if ((lectura >> valor) && !(lectura >> extra)) {
            if (valor >= minimo && valor <= maximo) {
                return valor;
            }
            cout << "[!] Valor fuera de rango (" << minimo << " a " << maximo << ").\n";
        } else {
            cout << "[!] Entrada invalida. Escriba un numero entero.\n";
        }
    }
}

// Lee un numero con decimales validando formato y rango
double leerDecimal(const string &mensaje, double minimo, double maximo) {
    while (true) {
        cout << mensaje;
        string linea;
        if (!getline(cin, linea)) {
            cout << "\n[!] Entrada cerrada. Fin del programa.\n";
            exit(1);
        }

        istringstream lectura(linea);
        double valor;
        char extra;

        if ((lectura >> valor) && !(lectura >> extra)) {
            if (valor >= minimo && valor <= maximo) {
                return valor;
            }
            cout << "[!] Valor fuera de rango (" << minimo << " a " << maximo << ").\n";
        } else {
            cout << "[!] Entrada invalida. Escriba un numero (ejemplo: 12.50).\n";
        }
    }
}

// Lee un texto cuidando que no este vacio ni tenga punto y coma (separador de la BD)
string leerTexto(const string &mensaje) {
    while (true) {
        cout << mensaje;
        string linea;
        if (!getline(cin, linea)) {
            cout << "\n[!] Entrada cerrada. Fin del programa.\n";
            exit(1);
        }

        linea = recortar(linea);
        bool tienePuntoYComa = false;
        for (int i = 0; i < (int)linea.length(); i++) {
            if (linea[i] == ';') {
                tienePuntoYComa = true;
            }
        }

        if (linea.empty()) {
            cout << "[!] El texto no puede estar vacio.\n";
        } else if (tienePuntoYComa) {
            cout << "[!] No se permite el caracter ';'.\n";
        } else if ((int)linea.length() > LONGITUD_TEXTO_MAX) {
            cout << "[!] Maximo " << LONGITUD_TEXTO_MAX << " caracteres.\n";
        } else {
            return linea;
        }
    }
}
