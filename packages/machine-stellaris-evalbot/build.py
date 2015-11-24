#
# eChronos Real-Time Operating System
# Copyright (C) 2015  National ICT Australia Limited (NICTA), ABN 62 102 206 173.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, version 3, provided that these additional
# terms apply under section 7:
#
#   No right, title or interest in or to any trade mark, service mark, logo or
#   trade name of of National ICT Australia Limited, ABN 62 102 206 173
#   ("NICTA") or its licensors is granted. Modified versions of the Program
#   must be plainly marked as such, and must not be distributed using
#   "eChronos" as a trade mark or product name, or misrepresented as being the
#   original Program.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# @TAG(NICTA_AGPL)
#

from prj import execute, SystemBuildError
import os


def run(system, configuration=None):
    return system_build(system)


def system_build(system):
    print("USING STELLARIS BUILD FUNCTION")
    inc_path_args = ['-I%s' % i for i in system.include_paths]
    common_flags = ['-mthumb', '-mcpu=cortex-m3', '-MD']
    a_flags = common_flags
    c_flags = common_flags + ['-Os', '-ffunction-sections', '-fdata-sections', '-DPART_LM3S9B96', '-std=c99', '-DTARGET_IS_TEMPEST_RB1']

    stellaris = '/home/schnommos/Dev/echronos/packages/machine-stellaris-evalbot/stellarisware-min/'

    execute(['make', stellaris])

    system.linker_script = stellaris + '../example/motor_demo.ld'

    # Compile all C files.
    c_obj_files = [os.path.join(system.output, os.path.basename(c.replace('.c', '.o'))) for c in system.c_files]

    for c, o in zip(system.c_files, c_obj_files):
        os.makedirs(os.path.dirname(o), exist_ok=True)
        execute(['arm-none-eabi-gcc', '-ffreestanding', '-c', c, '-o', o, '-Wall'] +
                c_flags + inc_path_args)

    # Assemble all asm files.
    asm_obj_files = [os.path.join(system.output, os.path.basename(s.replace('.s', '.o'))) for s in system.asm_files]
    for s, o in zip(system.asm_files, asm_obj_files):
        os.makedirs(os.path.dirname(o), exist_ok=True)
        execute(['arm-none-eabi-as', '-o', o, s] + a_flags + inc_path_args)

    # Perform final link
    obj_files = asm_obj_files + c_obj_files
    obj_files.append(stellaris + 'driverlib/gcc-cm3/libdriver-cm3.a')
    obj_files.append(stellaris + 'boards/ek-evalbot/motor_demo/gcc/io.o')
    execute(['arm-none-eabi-ld', '-T', system.linker_script, '-o', system.output_file] + obj_files)
