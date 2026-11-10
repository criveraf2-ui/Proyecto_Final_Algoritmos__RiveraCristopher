cmake_minimum_required(VERSION 3.10)
project(RutasBus_ApellidoNombre)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(frontend backend database)

# ============================================================================
# OPCIONAL: Si deseas enlazar libmysql.dll directamente con <mysql.h>,
# descomenta las 3 lineas siguientes (por defecto ya conecta automaticamente
# con MySQL Server / XAMPP mediante el cliente mysql.exe sin errores de DLL):
# include_directories("C:/Program Files/MySQL/MySQL Server 8.0/include")
# link_directories("C:/Program Files/MySQL/MySQL Server 8.0/lib")
# add_compile_definitions(USAR_LIBMYSQL)
# ============================================================================

add_executable(RutasBus
    main.cpp
    frontend/validaciones.cpp
    frontend/interfaz.cpp
    backend/logica_rutas.cpp
    backend/algoritmos.cpp
    database/cifrado_tontos.cpp
    database/conexion_db.cpp
    database/conexion_mysql.cpp
    database/servidor_http.cpp
)

# En Windows enlaza automaticamente ws2_32 para que el Servidor HTTP en C++
# reciba las peticiones CRUD del Frontend HTML en http://localhost:8080
if(WIN32)
    target_link_libraries(RutasBus ws2_32)
endif()

# Si descomentaste USAR_LIBMYSQL arriba, descomenta tambien esta linea:
# target_link_libraries(RutasBus libmysql)
