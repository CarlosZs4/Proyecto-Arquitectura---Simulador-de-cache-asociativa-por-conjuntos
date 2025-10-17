#include <iostream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <random>
#include <ctime>
#include <sstream>
#include <iomanip>

using namespace std;

const int nVias = 4;

class LineaCache {
private:
    int tag;    
    int datos; 
    bool valida;

public:
    LineaCache(int t = -1, int d = 0) : tag(t), datos(d), valida(t != -1) {}
    
    int getTag() const { return tag; }
    int getDatos() const { return datos; }
    bool getValida() const { return valida; }
    
    void setTag(int t) { 
        tag = t; 
        valida = (t != -1);
    }
    void setDatos(int d) { datos = d; }
    void setValida(bool v) { valida = v; }
};

using ConjuntoCache = list<LineaCache>;
using Cache = unordered_map<int, ConjuntoCache>;

// Variables globales
int numConjuntos; 
int tamanoBloqueBits = 4; // Tamaño de bloque: 2^4 = 16 (simulado, solo afecta el cálculo de bits)
int numBitsConjunto; 
int bitsTotales = 16; 

bool mostrarBinario = false; 

Cache miCache;
int accesos = 0;
int aciertos = 0;
int fallos = 0;

//   FUNCIONES

void menuOperaciones();
void menuTamano();
void operarManualmente();
void simularMatrizMips();
void simularCargaMipsReducida();
void cargarPrefetch(int direccionPrefetch);

// --- Funciones de Utilidad  ---

// Convierte un número decimal a una cadena binaria 
string decimalABinarioSimple(int n, int longitud) {
    if (n < 0 || longitud <= 0) return string(longitud, 'X');
    
    string bin = "";
    int temp = n;
    
    for (int i = 0; i < longitud; ++i) {
        bin = ((temp % 2 == 0) ? "0" : "1") + bin;
        temp /= 2;
    }
    return bin;
}

//devuelve el valor en decimal o binario
string obtenerValorFormato(int valor, int numBits) {
    if (mostrarBinario) {
        return decimalABinarioSimple(valor, numBits);
    }
    return to_string(valor);
}

// Formatea la dirección completa en binario resaltando Tag, Index y Offset.
string decimalABinario(int direccion, int tag, int index, int numBitsTag) {
    if (!mostrarBinario) {
        return to_string(direccion); 
    }
    
    // campos
    int numBitsOffset = tamanoBloqueBits;
    int numBitsIndex = numBitsConjunto;
    
    string dirBinCompleta = decimalABinarioSimple(direccion, bitsTotales);

    // Indices de corte
    int inicioTag = 0;
    int finTag = numBitsTag;
    int finIndex = finTag + numBitsIndex;

    stringstream ss;
    ss << "Bin: [";
    
    ss << "\033[1;31m" << dirBinCompleta.substr(inicioTag, numBitsTag) << "\033[0m";
    ss << "|";
    
    ss << "\033[1;34m" << dirBinCompleta.substr(finTag, numBitsIndex) << "\033[0m"; 
    ss << "|";

    ss << "\033[1;32m" << dirBinCompleta.substr(finIndex, numBitsOffset) << "\033[0m"; 
    ss << "]";

    return ss.str();
}

// --- Funciones de la Caché ---

// Calcula el índice, la etiqueta y el offset a partir de la dirección simulada
void calcularDireccion(int direccion, int& tag, int& index, int& numBitsTag, int& offsetBloque) {
    
    // Calcular el Offset del bloque (los últimos tamanoBloqueBits)
    int tamanoBloque = pow(2, tamanoBloqueBits);
    offsetBloque = direccion % tamanoBloque; 

    // Eliminar offset para obtener bits relevantes: | Tag | Index |
    int bitsRelevantes = direccion >> tamanoBloqueBits; 

    index = bitsRelevantes % numConjuntos;

    tag = bitsRelevantes / numConjuntos;

    // Calculo de el número de bits del tag
    int bitsRelevantesTotal = bitsTotales - tamanoBloqueBits;
    numBitsTag = bitsRelevantesTotal - numBitsConjunto;
}

// Simula la operación de lectura/escritura en la caché
void accederCache(int direccion, int datoNuevo = -1) {
    int tag, index, numBitsTag, offsetBloque;
    calcularDireccion(direccion, tag, index, numBitsTag, offsetBloque);
    accesos++;

    string dirFormato = decimalABinario(direccion, tag, index, numBitsTag);
    string tagFormato = obtenerValorFormato(tag, numBitsTag);
    string indexFormato = obtenerValorFormato(index, numBitsConjunto);
    string offsetFormato = obtenerValorFormato(offsetBloque, tamanoBloqueBits);

    // Buscar en el conjunto
    if (miCache.count(index)) {
        // El conjunto existe, buscar la etiqueta en sus vías
        ConjuntoCache& conjunto = miCache[index];
        auto it = find_if(conjunto.begin(), conjunto.end(),
                          [&tag](const LineaCache& linea) {
                              return linea.getValida() && linea.getTag() == tag;
                          });

        if (it != conjunto.end()) {
            aciertos++;
            if (datoNuevo != -1) {
                it->setDatos(datoNuevo);
            }
            cout << " ACIERTO: Dir " << dirFormato 
                 << " (Tag: " << tagFormato 
                 << ", Index: " << indexFormato 
                 << ", Offset: " << offsetFormato << "). Dato: " << it->getDatos() << endl;
            return;
        }
    }

    fallos++;
    cout << " FALLO: Dir " << dirFormato 
         << " (Tag: " << tagFormato 
         << ", Index: " << indexFormato 
         << ", Offset: " << offsetFormato << "). ";

    ConjuntoCache& conjunto = miCache[index];

    // 3. Política FIFO
    if (conjunto.size() >= nVias) {
       LineaCache victima = conjunto.front();
        conjunto.pop_front();
        
        string tagVictimaFormato = obtenerValorFormato(victima.getTag(), numBitsTag);

        cout << "Reemplaza (FIFO) la línea con Tag: " << tagVictimaFormato << ". ";
    } else {
         cout << "Hay espacio. ";
    }

    // 4. Cargar nuevo bloque
    int datoCargado = rand() % 100 + 100; // Simular dato traído de memoria principal
    if (datoNuevo != -1) {
        datoCargado = datoNuevo;
    }

    LineaCache nuevaLinea(tag, datoCargado);
    conjunto.push_back(nuevaLinea); // Insertar al final
    cout << "Carga nueva línea con Dato: " << datoCargado << "." << endl;

    int tamanoBloque = pow(2, tamanoBloqueBits);
    int direccionPrefetch = direccion + tamanoBloque; 
    cargarPrefetch(direccionPrefetch); 
}

// Simula la carga anticipada de un bloque 
void cargarPrefetch(int direccionPrefetch) {
    int tagP, indexP, numBitsTag, offsetBloqueP;
    calcularDireccion(direccionPrefetch, tagP, indexP, numBitsTag, offsetBloqueP);

    ConjuntoCache& conjuntoP = miCache[indexP];

    auto it = find_if(conjuntoP.begin(), conjuntoP.end(),
                      [&tagP](const LineaCache& linea) {
                          return linea.getValida() && linea.getTag() == tagP;
                      });

    if (it != conjuntoP.end()) {
        // Ya está en caché 
        return; 
    }
    
    if (conjuntoP.size() < nVias) {
        // Simular dato traído de memoria (usamos un rango diferente para distinguirlos visualmente)
        int datoCargadoP = rand() % 100 + 200; 
        LineaCache nuevaLineaP(tagP, datoCargadoP);
        
        conjuntoP.push_back(nuevaLineaP);
        
        string dirFormato = decimalABinario(direccionPrefetch, tagP, indexP, numBitsTag);
        string tagFormato = obtenerValorFormato(tagP, numBitsTag);
        string indexFormato = obtenerValorFormato(indexP, numBitsConjunto);
        string offsetFormato = obtenerValorFormato(offsetBloqueP, tamanoBloqueBits);

        cout << "[Prefetch] -> Trae Dir " << dirFormato 
             << " (Tag: " << tagFormato 
             << ", Index: " << indexFormato 
             << ", Offset: " << offsetFormato << "). Dato: " 
             << datoCargadoP << "." << endl;
    } 
}

// --- Funciones de Utilidad y Menú ---

void inicializarCache(int tamanoTotalBytes) {
    // Simulación de numero de conjuntos: Tamano total (en bytes) / (Tamano bloque * N_VIAS) = numConjuntos
    int tamanoBloqueBytes = pow(2, tamanoBloqueBits);
    int numLineasTotales = tamanoTotalBytes / tamanoBloqueBytes;

    if (numLineasTotales % nVias != 0) {
        cout << "Error: El número de líneas no es divisible por " << nVias << " vías." << endl;
        numConjuntos = 0;
        return;
    }

    numConjuntos = numLineasTotales / nVias;

    miCache.clear();
    accesos = 0;
    aciertos = 0;
    fallos = 0;

    if (numConjuntos > 0) {
        numBitsConjunto = log2(numConjuntos);
        
        int numBitsTag;
        // Calcular el número de bits del tag
        int bitsRelevantesTotal = bitsTotales - tamanoBloqueBits;
        numBitsTag = bitsRelevantesTotal - numBitsConjunto;

        cout << "\n--- CACHÉ INICIALIZADA ---" << endl;
        cout << "Configuración: " << nVias << "-Vías, FIFO." << endl;
        cout << "Tamaño Total: " << tamanoTotalBytes << " Bytes" << endl;
        cout << "Número de Conjuntos: " << numConjuntos << endl;
        cout << "Desglose de Dirección (" << bitsTotales << " bits): ";
        cout << "\033[1;31mTag (" << numBitsTag << " bits)\033[0m | ";
        cout << "\033[1;34mIndex (" << numBitsConjunto << " bits)\033[0m | ";
        cout << "\033[1;32mOffset (" << tamanoBloqueBits << " bits)\033[0m" << endl;
        cout << "--------------------------\n";
    } else {
        cout << "Error al configurar la caché. Revisa los tamaños." << endl;
    }
}

void mostrarEstadisticas() {
    if (accesos == 0) {
        cout << "\nNo se han realizado accesos a la caché." << endl;
        return;
    }
    cout << "\n--- Estadísticas de Rendimiento ---" << endl;
    cout << "Accesos Totales: " << accesos << endl;
    cout << "ACIERTO: " << aciertos << endl;
    cout << "FALLO: " << fallos << endl;
    double tasaAcierto = (double)aciertos / accesos * 100.0;
    cout << "Tasa de aciertos: " << tasaAcierto << " %" << endl;
    cout << "------------------------------------" << endl;
}

void simularMatrizMips() {
    int inicioDatos = 2000;   
    int tamanoElemento = 4;   
    int filas = 8;
    int columnas = 8;
    int numElementos = filas * columnas;
    int repeticiones = 4; // Repetir el recorrido 4, (para reforzar reemplazos)

    cout << "\n--- Simulación MIPS: Acceso a Matriz (" << filas << "x" << columnas << ") ---" << endl;
    cout << "Total de " << numElementos * repeticiones << " accesos a datos (excl. instr.)." << endl;

    for (int r = 0; r < repeticiones; ++r) {
        cout << "\n--- Repetición " << r + 1 << " ---" << endl;

        for (int i = 0; i < filas; ++i) {
            for (int j = 0; j < columnas; ++j) {
                //  CÁLCULO DE LA DIRECCIÓN DEL DATO
                int offset = (i * columnas + j) * tamanoElemento;
                int direccionDato = inicioDatos + offset;

                //  SIMULACIÓN DE INSTRUCCIONES DE ACCESO (Fetch)
                int direccionInstruccionBase = 1000 + (r % 2) * 100; 

                // Instrucción 1: Calculo de dirección (ej. addi, sll)
                accederCache(direccionInstruccionBase); 

                // Instrucción 2: Carga de Dato (lw)
                accederCache(direccionDato); 

                // Instrucción 3: Operación o Almacenamiento (add, sw, etc.)
                accederCache(direccionInstruccionBase + 4); 

            }
        }
    }
    cout << "\nSimulación de Matriz completada." << endl;
    mostrarEstadisticas();
}

void simularCargaMipsReducida() {
    int inicioDatos = 2000;   
    int tamanoElemento = 4;   
    int numElementos = 10;     
    int repeticiones = 3;       // para repetir 3 veces
    
    int direccionInstruccion = 1000; 

    cout << "\n--- Simulación MIPS Reducida (Array de 10 elementos) ---" << endl;
    cout << "Total de accesos a datos: " << numElementos * repeticiones << "." << endl;

    for (int r = 0; r < repeticiones; ++r) {
        cout << "\n--- Repetición " << r + 1 << " ---" << endl;

        for (int i = 0; i < numElementos; ++i) {
            
            // CÁLCULO DE LA DIRECCIÓN DEL DATO
            int offset = i * tamanoElemento;
            int direccionDato = inicioDatos + offset;
            
            // SIMULACIÓN DE INSTRUCCIONES DE ACCESO (Fetch)
            accederCache(direccionInstruccion); 

            // ACCESO A DATOS (lw)
            accederCache(direccionDato); 

            // Simular las otras instrucciones del bucle (addi, bne)
            accederCache(direccionInstruccion + 4); 
            accederCache(direccionInstruccion + 8); 
        }
    }
    cout << "\nSimulación MIPS Reducida completada." << endl;
    mostrarEstadisticas();
}

void operarManualmente() {
    int direccion, dato;
    int opcionOp;

    while (true) {
        cout << "\n--- Operación Manual ---" << endl;
        cout << "1. Leer Dirección" << endl;
        cout << "2. Escribir Dirección (simular 'dato' nuevo)" << endl;
        cout << "3. Mostrar Estadísticas" << endl;
        cout << "4. Volver al menú de operaciones" << endl;
        cout << "Seleccione una operación: ";
        cin >> opcionOp;

        if (opcionOp == 4) break;
        if (opcionOp == 3) {
            mostrarEstadisticas();
            continue;
        }

        cout << "Ingrese la Dirección de Memoria (entero): ";
        cin >> direccion;

        if (opcionOp == 1) {
            accederCache(direccion); // Lectura
        } else if (opcionOp == 2) {
            cout << "Ingrese el Dato a Escribir (entero): ";
            cin >> dato;
            accederCache(direccion, dato); // Escritura (datoNuevo = dato)
        } else {
            cout << "Opción inválida." << endl;
        }
    }
}

void menuOperaciones() {
    int opcion;

    while (true) {
        cout << "\n--- Menú de Operaciones ---" << endl;
        string formato = mostrarBinario ? "\033[1;33mBinario\033[0m" : "\033[1;33mDecimal\033[0m";
        cout << "Configuración actual: " << nVias << "-Vías, " << numConjuntos << " Conjuntos. Dirección en " << formato << "." << endl;
        cout << "1. Llenar la Caché Manualmente (Operar manualmente)" << endl;
        cout << "2. Simular codigo en MIPS (complejo - Matriz)" << endl;
        cout << "3. Simular codigo en MIPS (sencillo - Array)" << endl;
        cout << "4. Volver al Menú de Tamaño de Caché" << endl;
        cout << "5. Salir" << endl;
        cout << "Seleccione una opción: ";
        cin >> opcion;

        switch (opcion) {
            case 1: operarManualmente(); break;
            case 2: simularMatrizMips(); break;
            case 3: simularCargaMipsReducida(); break;
            case 4: return;
            case 5: exit(0);
            default: cout << "Opción inválida." << endl;
        }
    }
}

void menuTamano() {
    int opcionTamano;
    cout << "\n--- Menú de Configuración de Caché ---" << endl;
    cout << "1. Cache PEQUEÑA (512 Bytes)" << endl;
    cout << "2. Cache MEDIANA (1024 Bytes)" << endl;
    cout << "3. Cache GRANDE (2048 Bytes)" << endl;
    cout << "4. Salir" << endl;
    cout << "Seleccione el tamaño: ";
    cin >> opcionTamano;

    int tamanoElegido = 0;
    switch (opcionTamano) {
        case 1: tamanoElegido = 512; break;
        case 2: tamanoElegido = 1024; break;
        case 3: tamanoElegido = 2048; break;
        case 4: exit(0);
        default: cout << "Opción de tamaño inválida." << endl; return;
    }

    int opcionFormato;
    cout << "\n--- Menú de Formato de Dirección ---" << endl;
    cout << "1. Mostrar direcciones en Decimal (más limpio)" << endl;
    cout << "2. Mostrar direcciones en Binario (más explicativo)" << endl;
    cout << "Seleccione el formato: ";
    cin >> opcionFormato;
    
    if (opcionFormato == 2) {
        mostrarBinario = true;
    } else if (opcionFormato == 1) {
        mostrarBinario = false;
    } else {
        cout << "Opción de formato inválida. Usando Decimal por defecto." << endl;
        mostrarBinario = false;
    }

    inicializarCache(tamanoElegido);
    if (numConjuntos > 0) {
        menuOperaciones(); 
    }
}

int main() {
    srand(time(0));
    while(true){
        menuTamano();
    }

    return 0;
}