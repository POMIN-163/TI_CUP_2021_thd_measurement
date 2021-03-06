/*
 * Copyright (c) 2016-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== libBuilder.xs ========
 */

var buildInstrumentedLibrary = false; /* don't build instrumented libraries */

/*
 *  ======== makeLibs ========
 */
function makeLibs(name, targets, objects, cOpts)
{
    var lib;
    var gccOpts;
    var attrs = {
        copts: "",
        profile: "release"
    };

    for each (var targetName in targets) {
        /* Only proceed if target already in Build object */
        if (Build.printBuildTargets().indexOf(targetName + "\n") < 0) {
            continue;
        }

        /* Load target module */
        var target = xdc.module(targetName);

        var objs = objects.concat();

        /*
         *  Exlude incompatible assembly files from the object list.
         *  The objects parameter includes the super-set list. We prune that here.
         *  This code assumes that assembly files follow the form xyz_ccs.asm,
         *  xyz_gnu.asm, and xyz_iar.asm.
         */
        if (!targetName.match('clang')) {
            for (var i = objs.length - 1; i >= 0; i--) {
                if (objs[i].match('_clang.asm')) {
                    objs.splice(i, 1);
                }
            }
        }
        if (!targetName.match('ti.targets') || targetName.match('clang')) {
            for (var i = objs.length - 1; i >= 0; i--) {
                if (objs[i].match('_ccs.asm')) {
                    objs.splice(i, 1);
                }
            }
        }
        if (!targetName.match('gnu.targets')) {
            for (var i = objs.length - 1; i >= 0; i--) {
                if (objs[i].match('_gnu.asm')) {
                    objs.splice(i, 1);
                }
            }
        }
        if (!targetName.match('iar.targets')) {
            for (var i = objs.length - 1; i >= 0; i--) {
                if (objs[i].match('_iar.asm')) {
                    objs.splice(i, 1);
                }
            }
        }

        /*
         *  Suppress GCC 4.90's auto check-for-null-pointer-dereference
         *  'UDF' instruction generation
         */
        if (targetName.match('gnu.targets')) {
            gccOpts = " -fno-isolate-erroneous-paths-dereference ";
        }
        else {
            gccOpts = "";
        }

        /* Disable asserts & logs for the non-instrumented library */
        attrs.copts = cOpts + gccOpts +
            " -Dxdc_runtime_Log_DISABLE_ALL -Dxdc_runtime_Assert_DISABLE_ALL";
        lib = Pkg.addLibrary(name, target, attrs);
        lib.addObjects(objs);

        if (buildInstrumentedLibrary) {
            /* Enable asserts & logs for the instrumented library */
            attrs.copts = cOpts + gccOpts +
                " -Dxdc_runtime_Assert_DISABLE_CONDITIONAL_ASSERT" +
                " -DDebugP_ASSERT_ENABLED -DDebugP_LOG_ENABLED";
            lib = Pkg.addLibrary(name + "_instrumented", target, attrs);
            lib.addObjects(objs);
        }
    }
}

var cc26xxTargets = [
    "ti.targets.arm.clang.M4F",
    "ti.targets.arm.elf.M4F",
    "gnu.targets.arm.M4F",
    "iar.targets.arm.M4F",
];

var cc32xxTargets = [
    "ti.targets.arm.clang.M4",
    "ti.targets.arm.elf.M4",
    "gnu.targets.arm.M4",
    "iar.targets.arm.M4",
];

var msp432e4Targets = [
    "ti.targets.arm.clang.M4F",
    "ti.targets.arm.elf.M4F",
    "gnu.targets.arm.M4F",
    "iar.targets.arm.M4F",
];

var mtxxTargets = [
    "ti.targets.arm.clang.M33",
    "ti.targets.arm.clang.M33F",
];
