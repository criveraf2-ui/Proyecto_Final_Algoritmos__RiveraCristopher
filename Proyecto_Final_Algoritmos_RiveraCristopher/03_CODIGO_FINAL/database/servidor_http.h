#ifndef SERVIDOR_HTTP_H
#define SERVIDOR_HTTP_H

#include "modelos.h"

// Inicia el servidor HTTP local en C++ (http://localhost:8080) para que el
// Frontend HTML ejecute el CRUD (Create, Read, Update, Delete) directamente
// sobre la memoria de C++, rutas.txt y la base de datos MySQL.
void iniciarServidorWebCRUD(RutaBus rutas[], int &cantidad, int puerto = 8080);

#endif
