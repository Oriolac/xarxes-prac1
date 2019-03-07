#!/usr/bin/env python
"""
Servidor
"""

import sys


def print_if_debug(debug, cadena):
    """print_if_debug"""
    
    if debug:
        print cadena

def lectura_parametres():
    """lectura_parametres"""

    arxius = {"servidor": "server.cfg", "equips": "equips.dat"}
    debug = False
    trobat_conf_server = False
    trobat_conf_equips = False

    for arg in sys.argv:
        if arg == "-d":
            debug = True
            print_if_debug(debug, "Hola")
        elif arg == "-c":
            trobat_conf_server = True
        elif trobat_conf_server:
            arxius["servidor"] = arg
            trobat_conf_server = False
        elif arg == "-u":
            trobat_conf_equips = True
        elif trobat_conf_equips:
            arxius["equips"] = arg
            trobat_conf_equips = False
    if trobat_conf_equips or trobat_conf_server:
        print "La hemo jodio paco"
    return (debug, arxius)

if __name__ == "__main__":
    DEBUG, ARXIUS = lectura_parametres()
