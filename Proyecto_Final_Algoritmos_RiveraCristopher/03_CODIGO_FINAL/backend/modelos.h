#ifndef MODELOS_H
#define MODELOS_H

#include <string>
using namespace std;

const int MAX_RUTAS = 100;
const int ID_MAX = 9999;
const int CAPACIDAD_MAX = 100;
const double PRECIO_MAX = 1000.0;
const int LONGITUD_TEXTO_MAX = 30;

const string ARCHIVO_DATOS = "rutas.txt";
const string ARCHIVO_CIFRADO = "rutas_cifradas.txt";
const string ARCHIVO_CONFIG_MYSQL = "config_mysql.txt";

struct RutaBus {
    int id;
    string origen;
    string destino;
    int capacidad;
    int pasajeros;
    double precio;
    double ingresos;
    bool activa;
};

#endif
