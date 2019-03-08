#!/usr/bin/env python
"""
Servidor
"""

import sys
import time
import tokenize


def print_if_debug(debug, cadena):
    """print_if_debug"""
    if debug:
        print time.strftime("%H:%M:%S DEBUG => " + cadena)

def print_if_error(cadena):
    """ print_if_debug """
    print time.strftime("%H:%M:%S ERROR => " + cadena)

def lectura_parametres():
    """lectura_parametres"""

    nom_arxius = {'servidor': 'server.cfg', 'equips': 'equips.dat'}
    debug = False
    trobat_conf_server = False
    trobat_conf_equips = False

    for arg in sys.argv:
        if arg == "-d":
            debug = True
            print_if_debug(debug, "Mode DEBUG ON.")
        elif arg == "-c":
            trobat_conf_server = True
        elif trobat_conf_server:
            nom_arxius["servidor"] = arg
            print_if_debug(debug, "Arxiu de dades de software modificat: " + arg)
            trobat_conf_server = False
        elif arg == "-u":
            trobat_conf_equips = True
        elif trobat_conf_equips:
            nom_arxius["equips"] = arg
            print_if_debug(debug, "Arxiu d'equips autoritzats modificat: " + arg)
            trobat_conf_equips = False
    if trobat_conf_equips or trobat_conf_server:
        print "No s'han pogut obrir els arxius modificats"
        sys.exit()
    return (debug, obrir_arxius(nom_arxius))

def obrir_arxius(nom_arxius):
    """fdsa """
    try:
        f_server = open(nom_arxius['servidor'], 'r')
        f_equip = open(nom_arxius['equips'], 'r')
    except EnvironmentError:
        print_if_error("No s'han pogut obrir els arxius.")
        sys.exit()
    return {'servidor': f_server, 'equips': f_equip}

def agafar_dades_servidor(fitxer):
    """ agafar_dades_servidor """
    lines = fitxer.readlines()
    for line in lines:
        tokenize.tokenize(line)
        
if __name__ == '__main__':
    DEBUG, ARXIUS = lectura_parametres()
    agafar_dades_servidor(ARXIUS['servidor'])
