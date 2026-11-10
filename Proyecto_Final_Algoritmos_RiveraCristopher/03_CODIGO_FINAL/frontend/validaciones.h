#ifndef VALIDACIONES_H
#define VALIDACIONES_H

#include <string>
using namespace std;

// Funciones de lectura validada para que el programa no se bloquee con letras
int leerEntero(const string &mensaje, int minimo, int maximo);
double leerDecimal(const string &mensaje, double minimo, double maximo);
string leerTexto(const string &mensaje);

// Funciones auxiliares de texto hechas manualmente
string aMinusculas(string texto);
string recortar(const string &texto);
bool mismoTexto(const string &a, const string &b);

#endif
