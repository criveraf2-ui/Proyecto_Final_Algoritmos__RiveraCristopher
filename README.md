GUIA RAPIDA DE EJECUCION Y ESTRUCTURA DEL PROYECTO
Universidad Mariano Galvez de Guatemala - Algoritmos 2026
Proyecto 12: Transporte y Rutas - Variante A (Rutas de Buses)

1. ARQUITECTURA MODULAR EN C++17 (Carpeta 03_CODIGO_FINAL):
   - main.cpp                 : Coordinador principal del sistema.
   - CMakeLists.txt           : Configuracion lista para abrir y compilar en CLion (C++17).
   - frontend/validaciones.*  : Lectura segura (getline + istringstream) y manejo de texto ASCII.
   - frontend/interfaz.*      : Menus de consola, submenus y tablas formateadas.
   - backend/modelos.h        : Estructura RutaBus y constantes del sistema.
   - backend/logica_rutas.*   : Registro, listado, cambio de estado, ventas, cancelaciones y Reto Algoritmico.
   - backend/algoritmos.*     : Busqueda secuencial por ID, origen, destino, Burbuja manual y 3 reportes.
   - database/conexion_db.*   : Conexion a Base de Datos en archivos de texto mediante fstream.
   - database/cifrado_tontos.*: Modulo "Cifrado para Tontos" (desplazamiento ASCII +3 al guardar y -3 al leer).

2. COMO ABRIR Y EJECUTAR EN CLION:
   - En CLion: File > Open > seleccionar la carpeta 03_CODIGO_FINAL.
   - En Run > Edit Configurations > Working directory: colocar $ProjectFileDir$
   - Ejecutar el target 'RutasBus'.

3. COMO COMPILAR DESDE TERMINAL (OPCIONAL):
   g++ -std=c++17 -Wall -Wextra -Ifrontend -Ibackend -Idatabase main.cpp frontend/*.cpp backend/*.cpp database/*.cpp -o RutasBus
