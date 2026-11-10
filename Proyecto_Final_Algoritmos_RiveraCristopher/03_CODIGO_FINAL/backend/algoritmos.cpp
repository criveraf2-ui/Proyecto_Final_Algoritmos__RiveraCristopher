#include "algoritmos.h"
#include "validaciones.h"
#include "interfaz.h"
#include <iostream>
#include <iomanip>

using namespace std;

// Busqueda secuencial: revisa casilla por casilla hasta hallar el ID
int buscarPorID(const RutaBus rutas[], int cantidad, int idBuscado) {
    for (int i = 0; i < cantidad; i++) {
        if (rutas[i].id == idBuscado) {
            return i;
        }
    }
    return -1;
}

// Busqueda por ciudad de origen
void buscarPorOrigen(const RutaBus rutas[], int cantidad) {
    cout << "\n--- BUSCAR POR ORIGEN ---\n";
    string origen = leerTexto("Ciudad de origen: ");
    bool encontrado = false;

    for (int i = 0; i < cantidad; i++) {
        if (mismoTexto(rutas[i].origen, origen)) {
            if (!encontrado) {
                cout << "\n";
                imprimirEncabezadoRutas();
            }
            imprimirFilaRuta(rutas[i]);
            encontrado = true;
        }
    }

    if (!encontrado) {
        cout << "[!] No hay rutas con ese origen.\n";
    }
}

// Busqueda por ciudad de destino
void buscarPorDestino(const RutaBus rutas[], int cantidad) {
    cout << "\n--- BUSCAR POR DESTINO ---\n";
    string destino = leerTexto("Ciudad de destino: ");
    bool encontrado = false;

    for (int i = 0; i < cantidad; i++) {
        if (mismoTexto(rutas[i].destino, destino)) {
            if (!encontrado) {
                cout << "\n";
                imprimirEncabezadoRutas();
            }
            imprimirFilaRuta(rutas[i]);
            encontrado = true;
        }
    }

    if (!encontrado) {
        cout << "[!] No hay rutas con ese destino.\n";
    }
}

// Ordenamiento Burbuja sobre un arreglo de indices para no desordenar el arreglo original
// Criterios: 1) Mayor ingreso, 2) Mas pasajeros en caso de empate, 3) Menor ID
void ordenarPorIngresos(const RutaBus rutas[], int cantidad, int orden[]) {
    for (int i = 0; i < cantidad; i++) {
        orden[i] = i;
    }

    for (int i = 0; i < cantidad - 1; i++) {
        for (int j = 0; j < cantidad - i - 1; j++) {
            const RutaBus &a = rutas[orden[j]];
            const RutaBus &b = rutas[orden[j + 1]];
            bool intercambiar = false;

            if (a.ingresos < b.ingresos) {
                intercambiar = true;
            } else if (a.ingresos == b.ingresos) {
                if (a.pasajeros < b.pasajeros) {
                    intercambiar = true;
                } else if (a.pasajeros == b.pasajeros && a.id > b.id) {
                    intercambiar = true;
                }
            }

            if (intercambiar) {
                int aux = orden[j];
                orden[j] = orden[j + 1];
                orden[j + 1] = aux;
            }
        }
    }
}

// Reporte 1: Tabla de rutas ordenadas de mayor a menor ingreso
void reporteIngresos(const RutaBus rutas[], int cantidad) {
    cout << "\n--- REPORTE 1: RUTAS POR INGRESOS ---\n";
    if (cantidad == 0) {
        cout << "[!] No hay rutas registradas.\n";
        return;
    }

    int orden[MAX_RUTAS];
    ordenarPorIngresos(rutas, cantidad, orden);

    cout << left << setw(5) << "Pos" << setw(6) << "ID" << setw(34) << "Recorrido"
         << right << setw(8) << "Pasaj." << setw(14) << "Ingresos (Q)" << "\n";
    cout << string(67, '-') << "\n";

    for (int i = 0; i < cantidad; i++) {
        const RutaBus &r = rutas[orden[i]];
        string recorrido = r.origen + " - " + r.destino;
        cout << left << setw(5) << (i + 1) << setw(6) << r.id
             << setw(34) << recorrido
             << right << setw(8) << r.pasajeros
             << fixed << setprecision(2) << setw(14) << r.ingresos << "\n";
    }
}

// Reporte 2: Porcentaje de ocupacion y disponibilidad de asientos por recorrido
void reporteOcupacion(const RutaBus rutas[], int cantidad) {
    cout << "\n--- REPORTE 2: OCUPACION Y DISPONIBILIDAD ---\n";
    if (cantidad == 0) {
        cout << "[!] No hay rutas registradas.\n";
        return;
    }

    cout << left << setw(6) << "ID" << setw(34) << "Recorrido" << right << setw(6) << "Cap."
         << setw(7) << "Ocup." << setw(7) << "Disp." << setw(10) << "% Ocup." << "  "
         << left << "Estado\n";
    cout << string(80, '-') << "\n";

    for (int i = 0; i < cantidad; i++) {
        const RutaBus &r = rutas[i];
        double porcentaje = 100.0 * r.pasajeros / r.capacidad;
        string estado;
        if (!r.activa) {
            estado = "Inactiva";
        } else if (r.pasajeros == r.capacidad) {
            estado = "LLENA";
        } else {
            estado = "Disponible";
        }

        string recorrido = r.origen + " - " + r.destino;
        cout << left << setw(6) << r.id << setw(34) << recorrido
             << right << setw(6) << r.capacidad << setw(7) << r.pasajeros
             << setw(7) << (r.capacidad - r.pasajeros)
             << fixed << setprecision(1) << setw(9) << porcentaje << "%  "
             << left << estado << "\n";
    }
}

// Reporte 3: Estadisticas globales usando contadores y acumuladores
void reporteResumen(const RutaBus rutas[], int cantidad) {
    cout << "\n--- REPORTE 3: RESUMEN GENERAL ---\n";
    if (cantidad == 0) {
        cout << "[!] No hay rutas registradas.\n";
        return;
    }

    int activas = 0;
    int llenas = 0;
    int totalPasajeros = 0;
    int totalCapacidad = 0;
    double totalIngresos = 0.0;
    int idxMayorIngreso = 0;
    int idxMayorOcupacion = 0;
    int idxMasDisponible = -1;

    for (int i = 0; i < cantidad; i++) {
        const RutaBus &r = rutas[i];
        if (r.activa) activas++;
        if (r.pasajeros == r.capacidad) llenas++;

        totalPasajeros += r.pasajeros;
        totalCapacidad += r.capacidad;
        totalIngresos += r.ingresos;

        if (r.ingresos > rutas[idxMayorIngreso].ingresos) {
            idxMayorIngreso = i;
        }

        double ocupActual = (double)r.pasajeros / r.capacidad;
        double ocupMejor = (double)rutas[idxMayorOcupacion].pasajeros / rutas[idxMayorOcupacion].capacidad;
        if (ocupActual > ocupMejor) {
            idxMayorOcupacion = i;
        }

        if (r.activa) {
            int libresActual = r.capacidad - r.pasajeros;
            if (idxMasDisponible == -1 ||
                libresActual > (rutas[idxMasDisponible].capacidad - rutas[idxMasDisponible].pasajeros)) {
                idxMasDisponible = i;
            }
        }
    }

    cout << fixed << setprecision(2);
    cout << "Rutas registradas:        " << cantidad << " (activas: " << activas
         << ", inactivas: " << (cantidad - activas) << ")\n";
    cout << "Rutas llenas:             " << llenas << "\n";
    cout << "Pasajeros transportados:  " << totalPasajeros << " de " << totalCapacidad << " asientos\n";
    cout << "Ocupacion global:         " << (100.0 * totalPasajeros / totalCapacidad) << "%\n";
    cout << "Ingresos totales:         Q" << totalIngresos << "\n";
    cout << "Ingreso promedio / ruta:  Q" << (totalIngresos / cantidad) << "\n";
    cout << "Ruta de mayor ingreso:    " << rutas[idxMayorIngreso].id << " ("
         << rutas[idxMayorIngreso].origen << " - " << rutas[idxMayorIngreso].destino
         << ") Q" << rutas[idxMayorIngreso].ingresos << "\n";
    cout << "Ruta mas ocupada:         " << rutas[idxMayorOcupacion].id << " ("
         << rutas[idxMayorOcupacion].origen << " - " << rutas[idxMayorOcupacion].destino << ")\n";

    if (idxMasDisponible != -1) {
        cout << "Ruta con mas asientos:    " << rutas[idxMasDisponible].id << " ("
             << rutas[idxMasDisponible].origen << " - " << rutas[idxMasDisponible].destino << ") con "
             << (rutas[idxMasDisponible].capacidad - rutas[idxMasDisponible].pasajeros) << " libres\n";
    }
}
