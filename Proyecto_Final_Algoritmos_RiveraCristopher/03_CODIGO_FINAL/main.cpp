// Universidad Mariano Galvez de Guatemala - Ingenieria en Sistemas
// Curso: Algoritmos (2026) - Proyecto 12: Transporte y Rutas (Variante A)
// Arquitectura modular en C++17 con CRUD completo, MySQL, Servidor HTTP y Cifrado para Tontos

#include <iostream>
#include <string>
#include "modelos.h"
#include "validaciones.h"
#include "interfaz.h"
#include "conexion_db.h"
#include "conexion_mysql.h"
#include "servidor_http.h"

using namespace std;

int main(int argc, char *argv[]) {
    RutaBus rutas[MAX_RUTAS];
    int cantidad = 0;
    bool cambios = false;
    int opcion;

    cargarDatos(rutas, cantidad);

    // Si se inicia con --servidor, levanta de inmediato el servidor HTTP para el Frontend HTML
    if (argc > 1 && string(argv[1]) == "--servidor") {
        iniciarServidorWebCRUD(rutas, cantidad, 8080);
        return 0;
    }

    do {
        cout << "\\n========================================\\n";
        cout << "  SISTEMA DE TRANSPORTE Y RUTAS (VAR. A)\\n";
        cout << "========================================\\n";
        cout << "1. Gestion CRUD de rutas (Crear, Leer, Actualizar, Eliminar)\\n";
        cout << "2. Ventas y asignacion de boletos\\n";
        cout << "3. Busquedas\\n";
        cout << "4. Reportes\\n";
        cout << "5. Guardar datos en Archivos y MySQL\\n";
        cout << "6. Base de Datos MySQL y Cifrado para Tontos\\n";
        cout << "7. Iniciar Servidor Web para Frontend HTML (Puerto 8080)\\n";
        cout << "0. Guardar y salir\\n";
        opcion = leerEntero("Seleccione una opcion: ", 0, 7);

        switch (opcion) {
            case 1:
                menuRutas(rutas, cantidad, cambios);
                break;
            case 2:
                menuVentas(rutas, cantidad, cambios);
                break;
            case 3:
                menuBusquedas(rutas, cantidad);
                break;
            case 4:
                menuReportes(rutas, cantidad);
                break;
            case 5:
                if (guardarDatos(rutas, cantidad)) {
                    guardarRutasEnMySQL(rutas, cantidad);
                    cambios = false;
                    cout << "\\n[*] Datos guardados en " << ARCHIVO_DATOS
                         << ", respaldados con Cifrado para Tontos en " << ARCHIVO_CIFRADO
                         << " y sincronizados con MySQL.\\n";
                }
                break;
            case 6:
                menuBaseDatosYCifrado(rutas, cantidad);
                break;
            case 7:
                iniciarServidorWebCRUD(rutas, cantidad, 8080);
                break;
            case 0:
                if (guardarDatos(rutas, cantidad)) {
                    guardarRutasEnMySQL(rutas, cantidad);
                    cout << "\\n[*] Datos guardados. Hasta pronto.\\n";
                }
                break;
        }
    } while (opcion != 0);

    return 0;
}
