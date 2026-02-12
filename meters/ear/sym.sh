## List all libraries
ls $EAR_INSTALL_PATH/lib/libear*.so
#
## Check which library defines ear_connect
#nm -D $EAR_INSTALL_PATH/lib/libear.so | grep ear_connect
#
## Check which library defines the missing symbols
#nm -D $EAR_INSTALL_PATH/lib/libear*.so | grep is_cuda_enabled
#nm -D $EAR_INSTALL_PATH/lib/libear*.so | grep module_mpi_is_enabled
#
## Or check all symbols in all EAR libraries
#for lib in $EAR_INSTALL_PATH/lib/libear*.so; do
#  echo "=== $lib ==="
#  nm -D $lib | grep -E "(ear_connect|ear_energy|is_cuda_enabled|module_mpi)"
#done
#
## Search for all ear_ functions in all libraries
#for lib in $EAR_INSTALL_PATH/lib/libear*.so; do
#  echo "=== $lib ==="
#  nm -D $lib | grep " T ear_"
#done

# Or just check where ear_connect and ear_energy are defined:
nm -D $EAR_INSTALL_PATH/lib/libear*.so | grep " T ear_connect"
nm -D $EAR_INSTALL_PATH/lib/libear*.so | grep " T ear_energy"
