# Copyright (c) 2019-2023, Lawrence Livermore National Security, LLC and
# other Serac Project Developers. See the top-level LICENSE file for
# details.
#
# SPDX-License-Identifier: (BSD-3-Clause) 

set(test_dependencies serac_boundary_conditions gtest)
blt_list_append( TO test_dependencies ELEMENTS caliper adiak IF ${SERAC_ENABLE_PROFILING} )

set(boundary_cond_tests
    boundary_cond.cpp)

serac_add_tests( SOURCES ${boundary_cond_tests}
                 DEPENDS_ON ${test_dependencies}
                 NUM_MPI_TASKS 2)
