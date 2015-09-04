# MPI-coprime-count

Programa que utiliza la biblioteca MPI. 
Este programa cuenta los coprimos de un número en un rango de 2**27 - 2**32.

## Compilación
```bash
  mpicxx -o coprime_count main.cpp
```

## Ejecución
```
  mpiexec -n 20 -f dis_machine ./coprime_count
```
