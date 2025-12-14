#!/bin/bash

cd rscripts

# compilar
if [ "$1" = "--build" ]; then 
    echo "compilando"
    bash build.sh 
# ejecutar
elif [ "$1" = "--run" ]; then  
    echo "ejecutando"
    bash run.sh
# si no
else
    echo "comando no conocido"
    echo "--build    construye el proyecto"
    echo "--run      ejecuta el kernel directamente"
    cd ..
fi