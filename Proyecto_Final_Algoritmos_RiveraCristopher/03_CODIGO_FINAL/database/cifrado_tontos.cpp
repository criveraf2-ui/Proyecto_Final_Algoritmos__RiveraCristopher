#include "cifrado_tontos.h"
#include "validaciones.h"
#include <iostream>

using namespace std;

// Cifrado para Tontos: recorre cada letra y la desplaza +3 posiciones en la tabla ASCII
// Ejemplo: 'A' pasa a 'D', '1' pasa a '4', ';' pasa a '>'
string cifrarParaTontos(const string &textoOriginal) {
    string resultado = textoOriginal;
    for (int i = 0; i < (int)resultado.length(); i++) {
        if (resultado[i] >= 32 && resultado[i] <= 122) {
            resultado[i] = resultado[i] + CLAVE_TONTOS;
        }
    }
    return resultado;
}

// Descifrado para Tontos: hace la operacion inversa restando 3 en la tabla ASCII
string descifrarParaTontos(const string &textoCifrado) {
    string resultado = textoCifrado;
    for (int i = 0; i < (int)resultado.length(); i++) {
        if (resultado[i] >= (32 + CLAVE_TONTOS) && resultado[i] <= (122 + CLAVE_TONTOS)) {
            resultado[i] = resultado[i] - CLAVE_TONTOS;
        }
    }
    return resultado;
}

// Revisa si una linea del archivo viene cifrada (el ';' se convierte en '>' al sumar 3)
bool lineaEstaCifrada(const string &linea) {
    bool tienePuntoYComa = false;
    bool tieneMayorQue = false;
    for (int i = 0; i < (int)linea.length(); i++) {
        if (linea[i] == ';') tienePuntoYComa = true;
        if (linea[i] == '>') tieneMayorQue = true;
    }
    return (!tienePuntoYComa && tieneMayorQue);
}

// Permite probar en vivo el Cifrado para Tontos frente al ingeniero en la defensa
void mostrarDemostracionCifrado() {
    cout << "\n--- DEMOSTRACION DE CIFRADO PARA TONTOS ---\n";
    cout << "Explicacion: desplaza cada caracter +" << CLAVE_TONTOS
         << " posiciones en la tabla ASCII al guardar y -" << CLAVE_TONTOS << " al leer.\n";
    string prueba = leerTexto("Escriba un texto de prueba para cifrar: ");
    string cifrado = cifrarParaTontos(prueba);
    string recuperado = descifrarParaTontos(cifrado);

    cout << "Texto original   : " << prueba << "\n";
    cout << "Cifrado (+3)     : " << cifrado << "\n";
    cout << "Descifrado (-3)  : " << recuperado << "\n";
}
