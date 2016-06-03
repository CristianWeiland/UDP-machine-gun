#!/bin/bash
msgs=$1

if [ $# -eq 0 ]; then
    echo Uso correto: ./connect.sh numero-de-mensagens-por-cliente
    exit
fi

function connectTo {
    ssh -nf cw14@$1 ./redesII/client bowmore 7777 $msgs 0
}

connectTo cohiba
connectTo macalan
connectTo latrappe
connectTo caporal
connectTo priorat
connectTo talisker
connectTo achel

exit 0
