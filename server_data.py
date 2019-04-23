
from server import *


def lectura_parametres():
    """ Lectura dels parametres d'entrada. """

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
    """ Retorna les dades del servidor i dels equips"""
    try:
        f_server = open(nom_arxius['servidor'], 'r')
        f_equip = open(nom_arxius['equips'], 'r')
    except EnvironmentError:
        print_if_error("No s'han pogut obrir els arxius.")
        sys.exit()
    return {'servidor': f_server, 'equips': f_equip}


def agafar_dades_servidor(fitxer):
    """ Retorna les dades del servidor"""
    dades = {}
    lines = fitxer.readlines()
    for line in lines:
        line = line.split()
        for word in line:
            if word in ('Nom', 'MAC', 'UDP-port', 'TCP-port'):
                dades[word] = line[1]
    return dades


def agafar_dades_equips(fitxer):
    """ Retorna una llista amb tots els equips """
    llistat_dades = []
    lines = fitxer.readlines()
    for line in lines:
        line = line.split()
        if len(line) == 2:
            llistat_dades.append(dades_equip(line))
    return llistat_dades


def dades_equip(line):
    """ Retorna les dades d'un equip. Per defecte, disconnected i 000000 """
    dades = dict()
    dades['nom'] = line[0]
    dades['mac'] = line[1]
    dades['estat'] = 'DISCONNECTED'
    dades['aleatori'] = '000000'
    return dades


def to_str_tipus(tipus):
    """ Retorna la comanda segons el tipus """
    tipus = ord(tipus)
    dicc_tipus = {0x00: 'REGISTER_REQ', 0x01: 'REGISTER_ACK', 0x02: 'REGISTER_NACK', 0x03: 'REGISTER_REJ', 0x09: 'ERROR', 0x10: 'ALIVE_INF', 0x11: 'ALIVE_ACK', 0x12: 'ALIVE_NACK', 0x13: 'ALIVE_REJ',
                  0x20: 'SEND_FILE', 0x21: 'SEND_ACK', 0x22: 'SEND_NACK', 0x23: 'SEND_REJ', 0x24: 'SEND_DATA', 0x25: 'SEND_END',
                  0x30: 'GET_FILE', 0x31: 'GET_ACK', 0x32: 'GET_NACK', 0x33: 'GET_REJ', 0x34: 'GET_DATA', 0x35: 'GET_END'
                  }
    return dicc_tipus[tipus]


def to_int_tipus(str):
    """Retorna l'enter que correspon a la comanda"""
    dicc_tipus = {'REGISTER_REQ': 0x00, 'REGISTER_ACK': 0x01, 'REGISTER_NACK': 0x02, 'REGISTER_REJ': 0x03, 'ERROR': 0x09, 'ALIVE_INF': 0x10, 'ALIVE_ACK': 0x11, 'ALIVE_NACK': 0x12, 'ALIVE_REG': 0x13,
                  'SEND_FILE': 0x20, 'SEND_ACK': 0x21, 'SEND_NACK': 0x22, 'SEND_REJ': 0x23, 'SEND_DATA': 0x24, 'SEND_END': 0x25,
                  'GET_FILE': 0x20, 'GET_ACK': 0x21, 'GET_NACK': 0x22, 'GET_REJ': 0x23, 'GET_DATA': 0x24, 'GET_END': 0x25}
    return dicc_tipus[str]
