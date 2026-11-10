#ifndef CIFRADO_TONTOS_H
#define CIFRADO_TONTOS_H

#include <string>
using namespace std;

// Clave de desplazamiento para el Cifrado para Tontos
const int CLAVE_TONTOS = 3;

// Funciones del modulo "Cifrado para Tontos"
string cifrarParaTontos(const string &textoOriginal);
string descifrarParaTontos(const string &textoCifrado);
bool lineaEstaCifrada(const string &linea);
void mostrarDemostracionCifrado();

#endif
