/**
* @file     main.cpp
* @date     3/9/2015
* @author:  Kevin Delgado Sandí B22214 (kefdelgado@gmail.com)
* @brief Programa que encuentra los números relativos entre 2**27 y 2**32 de un número dado
*
* La clase utliza la biblioteca MPI para realizar esta tarea en parelelo.
* Cada proceso escribirá el resultado en un archivo de texto
*
* @version : 2.5
*/

#define NUMERO_DATOS_ENVIAR 4
#define NUMERO 0
#define INICIO_RANGO 1
#define FIN_RANGO 2
#define NUMERO_MINIMO 1
#define NUMEROS_POR_PROCESO 2
#define RESIDUO 3

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>

#include "mpi.h"

// Funciones auxiliares

int maximo_comun_divisor(unsigned long a, unsigned long b) {
  unsigned long tmp;
  while (b != 0) {
    tmp = b;
    b = a % tmp;
    a = tmp;
  }
  return a;
}

int obtener_primos_relativos(const unsigned long* datos, const char* processor_name, int proceso_id) {
  unsigned long cantidad_coprimos = 0;

  char filename[100];
  snprintf ( filename, sizeof(filename), "Coprimos_%d_%s_proc_%d%s", datos[NUMERO], processor_name, proceso_id, ".part" );
  //std::ofstream salida(filename, std::ios::app);
  for (unsigned long i = datos[INICIO_RANGO]; i < datos[FIN_RANGO]; ++i) {
    if ( maximo_comun_divisor( i, datos[NUMERO] ) == 1 ) {
      //salida << i << std::endl;
      ++cantidad_coprimos;
    }
  }
  //salida.close();
  return cantidad_coprimos;
}

// Programa Principal

int main(int argc, char* argv[]) {
  // Variables de interes para MPI
  int proceso_id;
  int numero_procesos;
  int namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Status status;

  // Variables que se usan durante la ejecución del programa
  double tiempo_inicio;
  unsigned long cantidad_total_coprimos;

  // Se inicia el trabajo con MPI
  MPI_Init(&argc, &argv);
  // MPI almacena en numero_procesos el numero total de procesos que se pusieron a correr
  MPI_Comm_size(MPI_COMM_WORLD, &numero_procesos);
  // MPI almacena en proceso_id la identificacion del proceso actual
  MPI_Comm_rank(MPI_COMM_WORLD, &proceso_id);
  // MPI almacena en processor_name el nombre de la computadora en la que corre el proceso actual, y en namelen la longitud de este
  MPI_Get_processor_name(processor_name, &namelen);

  /*
  * informacion[NUMERO] tendrá el numero que digitó el usuario
  * informacion[NUMERO_MINIMO] tendrá el número menor a ser evaluado
  * informacion[NUMEROS_POR_PROCESO] tendrá la cantidad de procesos a ser evaluados por cada proceso
  * informacion[RESIDUO] tendrá el RESIDUO para el último proceso
  */
  unsigned long informacion[NUMERO_DATOS_ENVIAR];

  /*
  *  datos[NUMERO] tendrá el numero que digitó el usuario
  *  datos[INICIO_RANGO] tendrá el principio del rango que le toca a ese proceso
  *  datos[FIN_RANGO] tendrá el fin del rango que le toca a ese proceso
  */
  unsigned long datos[NUMERO_DATOS_ENVIAR];

  if (proceso_id == 0) {
    // El número se envia como argumento del programa
    informacion[NUMERO] = strtoul (argv[1], NULL, 0);

    // Hora inicio
    tiempo_inicio = MPI_Wtime();

    // _información_ tendrá la información necesaria para que cada proceso pueda obtener el rango que le corresponde
    informacion[NUMERO_MINIMO] = pow(2, 27);
    const unsigned long NUMERO_MAXIMO = pow(2, 32);
    const unsigned long NUMEROS_TOTALES = NUMERO_MAXIMO - informacion[NUMERO_MINIMO] + 1;
    informacion[NUMEROS_POR_PROCESO] = NUMEROS_TOTALES / numero_procesos;
    informacion[RESIDUO] = NUMEROS_TOTALES % numero_procesos;
  }

  MPI_Bcast( informacion, NUMERO_DATOS_ENVIAR, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD );

  datos[NUMERO] = informacion[NUMERO];
  datos[INICIO_RANGO] = informacion[NUMERO_MINIMO] + (proceso_id * informacion[NUMEROS_POR_PROCESO]);
  // Asigna el final del rango. Si es el ultimo proceso, se le asigna el residuo
  datos[FIN_RANGO] = ( proceso_id == (numero_procesos-1) ) ? datos[INICIO_RANGO] + informacion[NUMEROS_POR_PROCESO] + informacion[RESIDUO] : datos[INICIO_RANGO] + informacion[NUMEROS_POR_PROCESO];

  // Obtiene los primos relativos
  unsigned long cantidad_coprimos = obtener_primos_relativos(datos, processor_name, proceso_id);
  MPI_Reduce(&cantidad_coprimos, &cantidad_total_coprimos, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  if (proceso_id == 0) {
    double tiempo_fin = MPI_Wtime();
    std::cout << "Busqueda de coprimos del numero " << datos[NUMERO] << " finalizado." << std::endl;
    std::cout << "La cantidad de coprimos encontrados fue: " << cantidad_total_coprimos << std::endl;
    std::cout << "El tiempo de ejecución del programa fue de: " << std::setprecision(4) << tiempo_fin - tiempo_inicio << " segundos." << std::endl;
  }

  MPI_Finalize();
  return 0;
}
