#!/bin/bash

# compilar
if [ "$1" = "build" ]; then 
    echo "compilando"
    bash build.sh 
fi

# ejecutar
if [ "$1" = "run" ]; then  
    echo "ejecutando"
    bash run.sh
fi