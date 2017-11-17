cab-chaos_5_x86_64_ib-clang@4.0.1.cmake                                                             0000664 0153350 0004253 00000010271 13203400101 016353  0                                                                                                    ustar   atk                             axomdev                                                                                                                                                                                                                ##################################
# uberenv host-config
#
# This is a generated file, edit at own risk.
##################################
# chaos_5_x86_64_ib-clang@4.0.1
##################################

# cmake from uberenv
# cmake executable path: /usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/clang-4.0.1/cmake-3.8.2-ogx3v5xe7tjiworftog2b65rue7fj5tu/bin/cmake

#######
# using clang@4.0.1 compiler spec
#######

# c compiler used by spack
set(CMAKE_C_COMPILER "/usr/global/tools/clang/chaos_5_x86_64_ib/clang-4.0.1/bin/clang" CACHE PATH "")

# cpp compiler used by spack
set(CMAKE_CXX_COMPILER "/usr/global/tools/clang/chaos_5_x86_64_ib/clang-4.0.1/bin/clang++" CACHE PATH "")

# fortran compiler used by spack
set(ENABLE_FORTRAN ON CACHE BOOL "")

set(CMAKE_Fortran_COMPILER "/usr/apps/gnu/4.9.3/bin/gfortran" CACHE PATH "")

# Root directory for generated TPLs
set(TPL_ROOT "/usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/clang-4.0.1" CACHE PATH "")

# hdf5 from uberenv
set(HDF5_DIR "${TPL_ROOT}/hdf5-1.8.16-f354ty3s3eovs5voooegrffh3yg5zde5" CACHE PATH "")

# scr not built by uberenv

# conduit from uberenv
set(CONDUIT_DIR "${TPL_ROOT}/conduit-0.2.1-3xjcjdkzkulzgnfyghbnenfewqq4wtoc" CACHE PATH "")

# mfem from uberenv
set(MFEM_DIR "${TPL_ROOT}/mfem-3.3.2-5ilanyoky7ep7egpzp5cttp5n3t3mehe" CACHE PATH "")

# boost headers from uberenv
set(BOOST_DIR "${TPL_ROOT}/boost-headers-1.58.0-alcsuy6edvcugusksrr2jhsnp5vyqk66" CACHE PATH "")

# python from uberenv
set(PYTHON_EXECUTABLE "${TPL_ROOT}/python-2.7.11-jeoiapalyouzfiwq45njnixltqvkqzi6/bin/python" CACHE PATH "")

# lua from uberenv
set(LUA_DIR "${TPL_ROOT}/lua-5.1.5-uhbj2j5qw5rjczce5rner57grpvowwp5" CACHE PATH "")

# doxygen from uberenv
set(DOXYGEN_EXECUTABLE "${TPL_ROOT}/doxygen-1.8.11-os5b5ulif6d2mo2cfbnjqg6q46vxsw74/bin/doxygen" CACHE PATH "")

# sphinx from uberenv
set(SPHINX_EXECUTABLE "${TPL_ROOT}/python-2.7.11-jeoiapalyouzfiwq45njnixltqvkqzi6/bin/sphinx-build" CACHE PATH "")

# shroud from uberenv
set(SHROUD_EXECUTABLE "${TPL_ROOT}/python-2.7.11-jeoiapalyouzfiwq45njnixltqvkqzi6/bin/shroud" CACHE PATH "")

# uncrustify from uberenv
set(UNCRUSTIFY_EXECUTABLE "${TPL_ROOT}/uncrustify-0.61-soshhb7er6sanbcrag2i2a4zts3f6vt5/bin/uncrustify" CACHE PATH "")

# lcov and genhtml from uberenv
set(LCOV_PATH "${TPL_ROOT}/lcov-1.11-x6zdnfkjncb5qu35zrpzmcfol5xtik63/usr/bin/lcov" CACHE PATH "")

set(GENHTML_PATH "${TPL_ROOT}/lcov-1.11-x6zdnfkjncb5qu35zrpzmcfol5xtik63/usr/bin/genhtml" CACHE PATH "")

# Disable CXX11 on chaos5 intel/clang builds
set(BLT_CXX_STD "c++98" CACHE PATH "")

##################################
# end uberenv host-config
##################################

##
## Copyright (c) 2017, Lawrence Livermore National Security, LLC.
##
## Produced at the Lawrence Livermore National Laboratory.
##
## LLNL-CODE-741217
##
## All rights reserved.
##
## This file is part of Axom.
##
## For details about use and distribution, please read axom/LICENSE.
##

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################
# Options added manually to
# lc chaos5 clang@4.0.1  host configs
##############################################################################

set(ENABLE_GTEST_DEATH_TESTS ON CACHE BOOL "")

##############################################################################
# MPI - manually added for now.
##############################################################################
set(ENABLE_MPI ON CACHE BOOL "")

set(MPI_HOME             "/usr/global/tools/clang/chaos_5_x86_64_ib/clang-4.0.1" CACHE PATH "")
set(MPI_C_COMPILER       "${MPI_HOME}/bin/mpiclang" CACHE PATH "")
set(MPI_CXX_COMPILER     "${MPI_HOME}/bin/mpiclang++" CACHE PATH "")

set(MPIEXEC              "/usr/bin/srun" CACHE PATH "")
set(MPIEXEC_NUMPROC_FLAG "-n" CACHE PATH "")

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################

                                                                                                                                                                                                                                                                                                                                       cab-chaos_5_x86_64_ib-gcc@4.9.3.cmake                                                               0000664 0153350 0004253 00000010461 13203400101 016037  0                                                                                                    ustar   atk                             axomdev                                                                                                                                                                                                                ##################################
# uberenv host-config
#
# This is a generated file, edit at own risk.
##################################
# chaos_5_x86_64_ib-gcc@4.9.3
##################################

# cmake from uberenv
# cmake executable path: /usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/gcc-4.9.3/cmake-3.8.2-cjuiep5cinfy7q6khr7hpsb3utzs34cd/bin/cmake

#######
# using gcc@4.9.3 compiler spec
#######

# c compiler used by spack
set(CMAKE_C_COMPILER "/usr/apps/gnu/4.9.3/bin/gcc" CACHE PATH "")

# cpp compiler used by spack
set(CMAKE_CXX_COMPILER "/usr/apps/gnu/4.9.3/bin/g++" CACHE PATH "")

# fortran compiler used by spack
set(ENABLE_FORTRAN ON CACHE BOOL "")

set(CMAKE_Fortran_COMPILER "/usr/apps/gnu/4.9.3/bin/gfortran" CACHE PATH "")

# Root directory for generated TPLs
set(TPL_ROOT "/usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/gcc-4.9.3" CACHE PATH "")

# hdf5 from uberenv
set(HDF5_DIR "${TPL_ROOT}/hdf5-1.8.16-2xgsiskrcxzxgs7avics5ghpclty2cr6" CACHE PATH "")

# scr not built by uberenv

# conduit from uberenv
set(CONDUIT_DIR "${TPL_ROOT}/conduit-0.2.1-3t74cwgfr32a6c2i3babrx7q6vehzvkj" CACHE PATH "")

# mfem from uberenv
set(MFEM_DIR "${TPL_ROOT}/mfem-3.3.2-fcmuhkaq7537pk3y62q4fbetyvuuokxr" CACHE PATH "")

# boost headers from uberenv
set(BOOST_DIR "${TPL_ROOT}/boost-headers-1.58.0-6wpeycxvyxiyu6g47hgwjefq4x5yur7w" CACHE PATH "")

# python from uberenv
set(PYTHON_EXECUTABLE "${TPL_ROOT}/python-2.7.11-eujx7frnxd5vpwolmye2fzq4tcylnbnv/bin/python" CACHE PATH "")

# lua from uberenv
set(LUA_DIR "${TPL_ROOT}/lua-5.1.5-ntnx5eksnbyip4iuvfullhypcbakqfm5" CACHE PATH "")

# doxygen from uberenv
set(DOXYGEN_EXECUTABLE "${TPL_ROOT}/doxygen-1.8.11-f7mrermmtt7ftitznsjwgy23lpy3poqb/bin/doxygen" CACHE PATH "")

# sphinx from uberenv
set(SPHINX_EXECUTABLE "${TPL_ROOT}/python-2.7.11-eujx7frnxd5vpwolmye2fzq4tcylnbnv/bin/sphinx-build" CACHE PATH "")

# shroud from uberenv
set(SHROUD_EXECUTABLE "${TPL_ROOT}/python-2.7.11-eujx7frnxd5vpwolmye2fzq4tcylnbnv/bin/shroud" CACHE PATH "")

# uncrustify from uberenv
set(UNCRUSTIFY_EXECUTABLE "${TPL_ROOT}/uncrustify-0.61-px2meiscmkbwcnmmom3qnlzdzmf2yx7x/bin/uncrustify" CACHE PATH "")

# lcov and genhtml from uberenv
set(LCOV_PATH "${TPL_ROOT}/lcov-1.11-dc44m4x5flevrfvqwytiscdc3dody4zx/usr/bin/lcov" CACHE PATH "")

set(GENHTML_PATH "${TPL_ROOT}/lcov-1.11-dc44m4x5flevrfvqwytiscdc3dody4zx/usr/bin/genhtml" CACHE PATH "")

##################################
# end uberenv host-config
##################################

##
## Copyright (c) 2017, Lawrence Livermore National Security, LLC.
##
## Produced at the Lawrence Livermore National Laboratory.
##
## LLNL-CODE-741217
##
## All rights reserved.
##
## This file is part of Axom.
##
## For details about use and distribution, please read axom/LICENSE.
##

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################
# Options added manually to 
# lc chaos5 gcc@4.9.3  host configs
##############################################################################

set(ENABLE_GTEST_DEATH_TESTS ON CACHE BOOL "")

##############################################################################
# MPI - manually added for now
##############################################################################
set(ENABLE_MPI ON CACHE BOOL "")

set(MPI_HOME             "/usr/local/tools/mvapich2-gnu-2.0" CACHE PATH "")
set(MPI_C_COMPILER       "${MPI_HOME}/bin/mpicc" CACHE PATH "")
set(MPI_CXX_COMPILER     "${MPI_HOME}/bin/mpicxx" CACHE PATH "")
set(MPI_Fortran_COMPILER "${MPI_HOME}/bin/mpif90" CACHE PATH "")

set(MPIEXEC              "/usr/bin/srun" CACHE PATH "")
set(MPIEXEC_NUMPROC_FLAG "-n" CACHE PATH "")

##############################################################################
# GCOV - manually added for now
##############################################################################
set(GCOV_PATH "/usr/apps/gnu/4.9.3/bin/gcov" CACHE PATH "")

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################

                                                                                                                                                                                                               cab-chaos_5_x86_64_ib-intel@15.0.223.cmake                                                          0000664 0153350 0004253 00000010276 13203400101 016637  0                                                                                                    ustar   atk                             axomdev                                                                                                                                                                                                                ##################################
# uberenv host-config
#
# This is a generated file, edit at own risk.
##################################
# chaos_5_x86_64_ib-intel@15.0.223
##################################

# cmake from uberenv
# cmake executable path: /usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/intel-15.0.223/cmake-3.8.2-zdtvp7o2nvfh2hoefrnyqjshcir6woje/bin/cmake

#######
# using intel@15.0.223 compiler spec
#######

# c compiler used by spack
set(CMAKE_C_COMPILER "/usr/local/tools/ic-15.0.223/bin/icc" CACHE PATH "")

# cpp compiler used by spack
set(CMAKE_CXX_COMPILER "/usr/local/tools/ic-15.0.223/bin/icpc" CACHE PATH "")

# fortran compiler used by spack
set(ENABLE_FORTRAN ON CACHE BOOL "")

set(CMAKE_Fortran_COMPILER "/usr/local/tools/ic-15.0.223/bin/ifort" CACHE PATH "")

# Root directory for generated TPLs
set(TPL_ROOT "/usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/intel-15.0.223" CACHE PATH "")

# hdf5 from uberenv
set(HDF5_DIR "${TPL_ROOT}/hdf5-1.8.16-aa73gjd3lrfity3yvt7wjbaxes5qlora" CACHE PATH "")

# scr not built by uberenv

# conduit from uberenv
set(CONDUIT_DIR "${TPL_ROOT}/conduit-0.2.1-ptupdxpaof5r7wwkbjfqcdionzyeqhj7" CACHE PATH "")

# mfem from uberenv
set(MFEM_DIR "${TPL_ROOT}/mfem-3.3.2-tl7kkswvhuifndhkgoryqtd2mhhwgshh" CACHE PATH "")

# boost headers from uberenv
set(BOOST_DIR "${TPL_ROOT}/boost-headers-1.58.0-puia3m3rglbiel47yayv7ohbw7ptk2nv" CACHE PATH "")

# python from uberenv
set(PYTHON_EXECUTABLE "${TPL_ROOT}/python-2.7.11-tjkk6ivuckww53r46hizt5e4ijuuthvk/bin/python" CACHE PATH "")

# lua from uberenv
set(LUA_DIR "${TPL_ROOT}/lua-5.1.5-aekux35obqucmenc2swvdplkn6fn7amy" CACHE PATH "")

# doxygen from uberenv
set(DOXYGEN_EXECUTABLE "${TPL_ROOT}/doxygen-1.8.11-zrkkyd6e5eb5tfiwtapc6q7eh4vqw2zf/bin/doxygen" CACHE PATH "")

# sphinx from uberenv
set(SPHINX_EXECUTABLE "${TPL_ROOT}/python-2.7.11-tjkk6ivuckww53r46hizt5e4ijuuthvk/bin/sphinx-build" CACHE PATH "")

# shroud from uberenv
set(SHROUD_EXECUTABLE "${TPL_ROOT}/python-2.7.11-tjkk6ivuckww53r46hizt5e4ijuuthvk/bin/shroud" CACHE PATH "")

# uncrustify from uberenv
set(UNCRUSTIFY_EXECUTABLE "${TPL_ROOT}/uncrustify-0.61-xzpfd3clyegn7p7crwgi73hyx2o2sfm3/bin/uncrustify" CACHE PATH "")

# lcov and genhtml from uberenv
set(LCOV_PATH "${TPL_ROOT}/lcov-1.11-2kxin7rtj5ejs3cbxxgwvuyqqhmrlotu/usr/bin/lcov" CACHE PATH "")

set(GENHTML_PATH "${TPL_ROOT}/lcov-1.11-2kxin7rtj5ejs3cbxxgwvuyqqhmrlotu/usr/bin/genhtml" CACHE PATH "")

# Disable CXX11 on chaos5 intel/clang builds
set(BLT_CXX_STD "c++98" CACHE PATH "")

##################################
# end uberenv host-config
##################################

##
## Copyright (c) 2017, Lawrence Livermore National Security, LLC.
##
## Produced at the Lawrence Livermore National Laboratory.
##
## LLNL-CODE-741217
##
## All rights reserved.
##
## This file is part of Axom.
##
## For details about use and distribution, please read axom/LICENSE.
##

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################
# Options added manually to 
# lc chaos5 intel@15.0.223 host configs
##############################################################################

set(ENABLE_GTEST_DEATH_TESTS ON CACHE BOOL "")

##############################################################################
# MPI - manually added for now
##############################################################################
set(ENABLE_MPI ON CACHE BOOL "")

set(MPI_HOME             "/usr/local/tools/mvapich2-intel-2.0" CACHE PATH "")
set(MPI_C_COMPILER       "${MPI_HOME}/bin/mpicc" CACHE PATH "")
set(MPI_CXX_COMPILER     "${MPI_HOME}/bin/mpicxx" CACHE PATH "")
set(MPI_Fortran_COMPILER "${MPI_HOME}/bin/mpif90" CACHE PATH "")

set(MPIEXEC              "/usr/bin/srun" CACHE PATH "")
set(MPIEXEC_NUMPROC_FLAG "-n" CACHE PATH "")

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################

                                                                                                                                                                                                                                                                                                                                  cab-chaos_5_x86_64_ib-intel@16.0.258.cmake                                                          0000664 0153350 0004253 00000010276 13203400101 016650  0                                                                                                    ustar   atk                             axomdev                                                                                                                                                                                                                ##################################
# uberenv host-config
#
# This is a generated file, edit at own risk.
##################################
# chaos_5_x86_64_ib-intel@16.0.258
##################################

# cmake from uberenv
# cmake executable path: /usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/intel-16.0.258/cmake-3.8.2-2esy2nq3rdz3fpan6scsvrxibu3sfag4/bin/cmake

#######
# using intel@16.0.258 compiler spec
#######

# c compiler used by spack
set(CMAKE_C_COMPILER "/usr/local/tools/ic-16.0.258/bin/icc" CACHE PATH "")

# cpp compiler used by spack
set(CMAKE_CXX_COMPILER "/usr/local/tools/ic-16.0.258/bin/icpc" CACHE PATH "")

# fortran compiler used by spack
set(ENABLE_FORTRAN ON CACHE BOOL "")

set(CMAKE_Fortran_COMPILER "/usr/local/tools/ic-16.0.258/bin/ifort" CACHE PATH "")

# Root directory for generated TPLs
set(TPL_ROOT "/usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/intel-16.0.258" CACHE PATH "")

# hdf5 from uberenv
set(HDF5_DIR "${TPL_ROOT}/hdf5-1.8.16-zvfopdsqd7w4vjwfswxp5utura6sr3cv" CACHE PATH "")

# scr not built by uberenv

# conduit from uberenv
set(CONDUIT_DIR "${TPL_ROOT}/conduit-0.2.1-klgff7xygiguj7kbild575hwf3icqqta" CACHE PATH "")

# mfem from uberenv
set(MFEM_DIR "${TPL_ROOT}/mfem-3.3.2-xbfzoiwzyh5kxekrxd2r4mmcimhg245t" CACHE PATH "")

# boost headers from uberenv
set(BOOST_DIR "${TPL_ROOT}/boost-headers-1.58.0-yiarrlaedsp4aidnxesdu2iqougxt365" CACHE PATH "")

# python from uberenv
set(PYTHON_EXECUTABLE "${TPL_ROOT}/python-2.7.11-t4izxxjuwjf2ryyv65vddraz6guolocp/bin/python" CACHE PATH "")

# lua from uberenv
set(LUA_DIR "${TPL_ROOT}/lua-5.1.5-slj6twcch6tvfdomhkqdknjjxiwswmzf" CACHE PATH "")

# doxygen from uberenv
set(DOXYGEN_EXECUTABLE "${TPL_ROOT}/doxygen-1.8.11-jqhrxckf2kswxfogk73qrgc2cru3dulc/bin/doxygen" CACHE PATH "")

# sphinx from uberenv
set(SPHINX_EXECUTABLE "${TPL_ROOT}/python-2.7.11-t4izxxjuwjf2ryyv65vddraz6guolocp/bin/sphinx-build" CACHE PATH "")

# shroud from uberenv
set(SHROUD_EXECUTABLE "${TPL_ROOT}/python-2.7.11-t4izxxjuwjf2ryyv65vddraz6guolocp/bin/shroud" CACHE PATH "")

# uncrustify from uberenv
set(UNCRUSTIFY_EXECUTABLE "${TPL_ROOT}/uncrustify-0.61-53naixuibvacrffbdkwexmef7wgj5fyf/bin/uncrustify" CACHE PATH "")

# lcov and genhtml from uberenv
set(LCOV_PATH "${TPL_ROOT}/lcov-1.11-hwb6f57wogxsdgial7urrqaijbh6ufjz/usr/bin/lcov" CACHE PATH "")

set(GENHTML_PATH "${TPL_ROOT}/lcov-1.11-hwb6f57wogxsdgial7urrqaijbh6ufjz/usr/bin/genhtml" CACHE PATH "")

# Disable CXX11 on chaos5 intel/clang builds
set(BLT_CXX_STD "c++98" CACHE PATH "")

##################################
# end uberenv host-config
##################################

##
## Copyright (c) 2017, Lawrence Livermore National Security, LLC.
##
## Produced at the Lawrence Livermore National Laboratory.
##
## LLNL-CODE-741217
##
## All rights reserved.
##
## This file is part of Axom.
##
## For details about use and distribution, please read axom/LICENSE.
##

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################
# Options added manually to 
# lc chaos5 intel@16.0.258 host configs
##############################################################################

set(ENABLE_GTEST_DEATH_TESTS ON CACHE BOOL "")

##############################################################################
# MPI - manually added for now
##############################################################################
set(ENABLE_MPI ON CACHE BOOL "")

set(MPI_HOME             "/usr/local/tools/mvapich2-intel-2.0" CACHE PATH "")
set(MPI_C_COMPILER       "${MPI_HOME}/bin/mpicc" CACHE PATH "")
set(MPI_CXX_COMPILER     "${MPI_HOME}/bin/mpicxx" CACHE PATH "")
set(MPI_Fortran_COMPILER "${MPI_HOME}/bin/mpif90" CACHE PATH "")

set(MPIEXEC              "/usr/bin/srun" CACHE PATH "")
set(MPIEXEC_NUMPROC_FLAG "-n" CACHE PATH "")

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################

                                                                                                                                                                                                                                                                                                                                  cab-chaos_5_x86_64_ib-intel@17.0.174.cmake                                                          0000664 0153350 0004253 00000010276 13203400101 016646  0                                                                                                    ustar   atk                             axomdev                                                                                                                                                                                                                ##################################
# uberenv host-config
#
# This is a generated file, edit at own risk.
##################################
# chaos_5_x86_64_ib-intel@17.0.174
##################################

# cmake from uberenv
# cmake executable path: /usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/intel-17.0.174/cmake-3.8.2-j4f2uiw4gpt5acb6f4p7rjn6jvpbue5i/bin/cmake

#######
# using intel@17.0.174 compiler spec
#######

# c compiler used by spack
set(CMAKE_C_COMPILER "/usr/local/tools/ic-17.0.174/bin/icc" CACHE PATH "")

# cpp compiler used by spack
set(CMAKE_CXX_COMPILER "/usr/local/tools/ic-17.0.174/bin/icpc" CACHE PATH "")

# fortran compiler used by spack
set(ENABLE_FORTRAN ON CACHE BOOL "")

set(CMAKE_Fortran_COMPILER "/usr/local/tools/ic-17.0.174/bin/ifort" CACHE PATH "")

# Root directory for generated TPLs
set(TPL_ROOT "/usr/workspace/wsa/axom/thirdparty_libs/builds/2017_11_16_09_25_52/spack/opt/spack/chaos_5_x86_64_ib/intel-17.0.174" CACHE PATH "")

# hdf5 from uberenv
set(HDF5_DIR "${TPL_ROOT}/hdf5-1.8.16-yle5lfpy32ffwzm54w2btci2rwgxpnj5" CACHE PATH "")

# scr not built by uberenv

# conduit from uberenv
set(CONDUIT_DIR "${TPL_ROOT}/conduit-0.2.1-owxnk65o2urrjfywf63sdrsxjdzbpjfo" CACHE PATH "")

# mfem from uberenv
set(MFEM_DIR "${TPL_ROOT}/mfem-3.3.2-hyb5naavqgv7cqibw7lozrgom3ynczek" CACHE PATH "")

# boost headers from uberenv
set(BOOST_DIR "${TPL_ROOT}/boost-headers-1.58.0-f3qubdpgb5tjxupxkfeo6vhjqp336w66" CACHE PATH "")

# python from uberenv
set(PYTHON_EXECUTABLE "${TPL_ROOT}/python-2.7.11-gsatbg7hsqa6y57pmds3ysr6zhhxmleh/bin/python" CACHE PATH "")

# lua from uberenv
set(LUA_DIR "${TPL_ROOT}/lua-5.1.5-mne43sd5phd4m5qp23urcngva3xzjfq2" CACHE PATH "")

# doxygen from uberenv
set(DOXYGEN_EXECUTABLE "${TPL_ROOT}/doxygen-1.8.11-33gzcqdym2leojxte3y235y35xf6wmrd/bin/doxygen" CACHE PATH "")

# sphinx from uberenv
set(SPHINX_EXECUTABLE "${TPL_ROOT}/python-2.7.11-gsatbg7hsqa6y57pmds3ysr6zhhxmleh/bin/sphinx-build" CACHE PATH "")

# shroud from uberenv
set(SHROUD_EXECUTABLE "${TPL_ROOT}/python-2.7.11-gsatbg7hsqa6y57pmds3ysr6zhhxmleh/bin/shroud" CACHE PATH "")

# uncrustify from uberenv
set(UNCRUSTIFY_EXECUTABLE "${TPL_ROOT}/uncrustify-0.61-4mhnkbt4d4vjxremrvvyknidkdqtud5o/bin/uncrustify" CACHE PATH "")

# lcov and genhtml from uberenv
set(LCOV_PATH "${TPL_ROOT}/lcov-1.11-4bjbbbg3jjp5apf3qcvbwhxspyfjrkmv/usr/bin/lcov" CACHE PATH "")

set(GENHTML_PATH "${TPL_ROOT}/lcov-1.11-4bjbbbg3jjp5apf3qcvbwhxspyfjrkmv/usr/bin/genhtml" CACHE PATH "")

# Disable CXX11 on chaos5 intel/clang builds
set(BLT_CXX_STD "c++98" CACHE PATH "")

##################################
# end uberenv host-config
##################################

##
## Copyright (c) 2017, Lawrence Livermore National Security, LLC.
##
## Produced at the Lawrence Livermore National Laboratory.
##
## LLNL-CODE-741217
##
## All rights reserved.
##
## This file is part of Axom.
##
## For details about use and distribution, please read axom/LICENSE.
##

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################
# Options added manually to 
# lc chaos5 intel@17.0.174 host configs
##############################################################################

set(ENABLE_GTEST_DEATH_TESTS ON CACHE BOOL "")

##############################################################################
# MPI - manually added for now
##############################################################################
set(ENABLE_MPI ON CACHE BOOL "")

set(MPI_HOME             "/usr/local/tools/mvapich2-intel-2.0" CACHE PATH "")
set(MPI_C_COMPILER       "${MPI_HOME}/bin/mpicc" CACHE PATH "")
set(MPI_CXX_COMPILER     "${MPI_HOME}/bin/mpicxx" CACHE PATH "")
set(MPI_Fortran_COMPILER "${MPI_HOME}/bin/mpif90" CACHE PATH "")

set(MPIEXEC              "/usr/bin/srun" CACHE PATH "")
set(MPIEXEC_NUMPROC_FLAG "-n" CACHE PATH "")

##############################################################################
# !---------------------------------------------------------------------------
##############################################################################

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  