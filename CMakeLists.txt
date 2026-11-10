cmake_minimum_required(VERSION 4.3)
project(Proyecto_Final_Algoritmos__RiveraCristopher)

set(CMAKE_CXX_STANDARD 14)

include_directories(Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/backend)
include_directories(Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database)
include_directories(Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/frontend)
link_libraries(ws2_32)
add_executable(Proyecto_Final_Algoritmos__RiveraCristopher
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/backend/algoritmos.cpp
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/backend/algoritmos.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/backend/logica_rutas.cpp
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/backend/logica_rutas.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/backend/modelos.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database/cifrado_tontos.cpp
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database/cifrado_tontos.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database/conexion_db.cpp
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database/conexion_db.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database/conexion_mysql.cpp
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database/conexion_mysql.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database/servidor_http.cpp
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/database/servidor_http.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/frontend/interfaz.cpp
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/frontend/interfaz.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/frontend/validaciones.cpp
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/frontend/validaciones.h
        Proyecto_Final_Algoritmos_RiveraCristopher/03_CODIGO_FINAL/main.cpp)
