#!/bin/bash

cd rscripts

# compilar
if [ "$1" = "--build" ]; then 
    # compila algo
    echo "compilando"       # compilar
    shift                   # elimina el primer argumento ($1)
    bash build.sh "$@"      # pasa el resto de parámetros
# ejecutar
elif [ "$1" = "--run" ]; then  
    # solo ejecuta algo
    echo "ejecutando"       # ejecutar
    shift                   # elimina el primer parametro
    bash run.sh "$@"        # pasa el resto de parametros
# si no
else
    echo "comando no conocido"
    echo "--build    construye el proyecto"
    echo "--run      ejecuta el kernel directamente"
    cd ..
fi