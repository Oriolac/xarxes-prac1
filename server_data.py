
from server import *

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
        print("No s'han pogut obrir els arxius modificats")
        sys.exit()
    return debug, obrir_arxius(nom_arxius)


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
    dades = {}
    lines = fitxer.readlines()
    for line in lines:
        line = line.split()
        for word in line:
            if word in ('Nom', 'MAC', 'UDP-port', 'TCP-port'):
                dades[word] = line[1]
    return dades


def agafar_dades_equips(fitxer):
    """ agafar_dades_equips """
    llistat_dades = []
    lines = fitxer.readlines()
    for line in lines:
        line = line.split()
        if len(line) == 2:
            llistat_dades.append(dades_equip(line))
    return llistat_dades


def dades_equip(line):
    """ fsjadiofas """
    dades = dict()
    dades['nom'] = line[0]
    dades['mac'] = line[1]
    dades['estat'] = 'DISCONNECTED'
    return dades


def to_str_tipus(tipus):
    """ to_str_tipus """
    tipus = ord(tipus)
    dicc_tipus = {0x00 : 'REGISTER_REQ', 0x01 : 'REGISTER_ACK', 0x02 : 'REGISTER_NACK',\
        0x03 : 'REGISTER_REJ', 0x09 : 'ERROR', 0x10 : 'ALIVE_INF', 0x11 : 'ALIVE_ACK',\
        0x12 : 'ALIVE_NACK', 0x13 : 'ALIVE_REJ'}
    return dicc_tipus[tipus]
