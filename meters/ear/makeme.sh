#!/bin/bash

ml ear
ml intel

icx -o API API.c \
  -I$EAR_INSTALL_PATH/include -L$EAR_INSTALL_PATH/lib -lear -learld -lear.ompi -lear_api

export LD_LIBRARY_PATH=$EAR_INSTALL_PATH/lib:$LD_LIBRARY_PATH
