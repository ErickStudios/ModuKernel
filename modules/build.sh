# modules/build.sh
#
# archivo de compilacion de los modulos del kernel


cd ..

source rscripts/utils_compiler.sh

# ------------------------------------------------------------------------------------------------------------------------------------------------------
# | Programa/Driver/Modulo                                                                                      | Descripcion/Nombre
# ------------------------------------------------------------------------------------------------------------------------------------------------------

# Fundamental System                                                                                            ( in /kernel )
compile_raw modules/init/main.c build/modules/init.modubin "Fundamental System Initializer"                     # Fundamental System Initializer
compile_raw modules/shell/main.c build/modules/modush.modubin "Fundamental System ModuShell"                    # Fundamental System ModuShell

# Mouse PS/2                                                                                                    ( in /dev )
compile_raw modules/mouse/ps2.c build/moddrivers/mps2.modubin "Mouse PS/2 Enable Routine"                       # Mouse PS/2 Enable Routine
compile_raw modules/mouse/ups2.c build/moddrivers/mups2.modubin "Mouse PS/2 Disable Routine"                    # Mouse PS/2 Disable Routine

# PCi Driver                                                                                                    ( in /dev )
compile_raw modules/pci/driver.c build/moddrivers/pci.modubin "PCi Driver config I/O"                           # PCi Driver config I/O

# Simple NET                                                                                                    ( in /dev )
compile_raw modules/net/SimpleNetInit.c build/moddrivers/snet.modubin "Simple NET initializer"                  # Simple NET initializer
compile_raw modules/net/SimpleNetGetMac.c build/moddrivers/snetm.modubin "Simple NET Mac Get"                   # Simple NET Mac Get
compile_raw modules/net/SimpleNetPkgSend.c build/moddrivers/snetps.modubin "Simple NET Package Send"            # Simple NET Package Send
compile_raw modules/net/SimpleNetPkgRecive.c build/moddrivers/snetpr.modubin "Simple NET Package Recive"        # Simple NET Package Recive
compile_raw modules/net/SimpleNetPkgSet.c build/moddrivers/snetpst.modubin "Simple NET Package Set Buffer"      # Simple NET Package Set Buffer
compile_raw modules/net/SimpleNetPkgUnset.c build/moddrivers/snetus.modubin "Simple NET Package Unset Buffer"   # Simple NET Package Unset Buffer
compile_raw modules/teleq/driver.c build/moddrivers/stlq.modubin "Simple NET simple telecomunication"           # Simple NET simple telecomunication

# Simple Regex                                                                                                  ( in /dev )
compile_cpp modules/regex/regex.cpp build/moddrivers/srex.modubin "Simple Regex Match True/False"               # Simple Regex Match True/False
compile_cpp modules/regex/tregex.cpp build/moddrivers/trex.modubin "Simple Regex Match Typed"                   # Simple Regex Match Typed