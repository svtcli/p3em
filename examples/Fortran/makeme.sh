#!/bin/bash
#   Copyright(C) 2025 Salvatore Cielo, Leibniz-Rechenzentrum
#   Copyright(C) 2025 Ivan Pribec, Leibniz-Rechenzentrum
#   Copyright(C) 2025 Alexander Pöppl, Intel Corporation
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
#  License. You may obtain a copy of the License at    http:#www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
#  language governing permissions and limitations under the License.

#- choose compilers
MYCC=icx
MYFC=ifx

#- Wrapper of C example interface
$MYCC -c p3em.c -I ../C

#- Compiling
$MYFC -pthread main_capi.f90    -o main_capi    p3em.o
$MYFC -pthread main_wrapped.f90 -o main_wrapped p3em.o
