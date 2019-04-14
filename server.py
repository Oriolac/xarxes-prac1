#!/usr/bin/env python
"""
Servidor
"""

from server_data import *
import sys
import time
import socket
import signal
import struct
import os

def print_if_debug(debug, cadena):
    """print_if_debug"""
    if debug:
        print(time.strftime("%H:%M:%S DEBUG => ") + cadena)


def print_if_error(cadena):
    """ print_if_debug """
    print(time.strftime("%H:%M:%S ERROR => ") + cadena)


def print_with_time(cadena):
    """ print_with_time """
    print(time.strftime("%H:%M:%S ") + cadena)


def to_str_dades_udp(dades):
    """ to_str_dades_udp """
    return 'bytes=' + str(len(dades)) + ', tipus=' + str(to_str_tipus(dades[0])) + ', nom=' + \
        dades[1:7] + ', mac=' + dades[8:20] + ', alea=' + dades[21:27] + ', dades=' + dades[27:]


def create_empty_pack(type, data):
    """ create_rej_pack    """
    camps = ['', '', '', '']
    llargada_camps = (7, 13, 7, 50)
    index_camps = 0
    for llargada in llargada_camps:
        camps[index_camps] = camps[index_camps].zfill(llargada)
        index_camps += 1
    return struct.pack('c7s13s7s50s', chr(3), '', '', '', data)


def enviar_paquet_udp_rej(sock, address, data):
    """ enviar_paquet_udp_rej """
    pack = create_empty_pack(0x00, data)
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(pack))
    sock.sendto(pack, address)


def enviar_paquet_ack(sock, address, dades_serv):
    """ enviar_paquet_ack """

    def num_aleatori():
        import random
        return str(random.randint(0, 1000000))

    data = struct.pack('c7s13s7s50s', chr(0x01), dades_serv['Nom'], dades_serv['MAC'], num_aleatori(),
                       dades_serv['TCP-port'])
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(data))
    sock.sendto(data, address)


def enviar_paquet_err(sock, address):
    data = struct.pack('c7s13s7s50s', chr(0x09))


def enviar_paquet_nack(sock, address, dades):
    pack = create_empty_pack(0x02, dades)
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(pack))
    sock.sendto(pack, address)


def confirmacio_registre(data, sock, address, equips, dades_servidor):
    autoritzat = False

    for equip in equips:
        if equip['nom'].__eq__(data[1:6]) and equip['mac'].__eq__(data[8:20]):
            autoritzat = True
            if data[21:27].__eq__('000000'):
                if equip['estat'].__eq__('DISCONNECTED'):
                    equip['estat'] = 'REGISTERED'
                    equip['address'] = address
                    acceptat = True
                    print_if_debug(DEBUG,
                        'Acceptat registre. Equip: nom=' + equip['nom'] + ', ip=' + address[0] + ', mac=' +
                        equip['mac'] + ', alea=' + data[21:27])
                    print_with_time('MSG.  => Equip ' + data[1:6] + ' passa a estat ' + equip['estat'])
                    enviar_paquet_ack(sock, address, dades_servidor)
                    return True
                elif not equip['address'].__eq__(address):
                    enviar_paquet_nack(sock, address, 'Discrepancies amb IP')
                    return False
                else:
                    enviar_paquet_ack(sock, address, dades_servidor)
                    return True
            else:
                enviar_paquet_nack(sock, address, 'Discrepancies amb el nombre aleatori')
                return False
    if not autoritzat:
        enviar_paquet_udp_rej(sock, address, 'Equip no autoritzat en el sistema')
        return True


def tractar_dades_udp(data, sock, address, equips, dades_servidor):
    """ fadsfa """

    if ord(data[0]) == 0x00:
        if confirmacio_registre(data, sock, address, equips, dades_servidor):
            print_if_debug(DEBUG, 'Sha de fer el foorkkk')
    elif ord(data[0]) == 0x10:
        pass
    else:
        enviar_paquet_err(sock, address)


def udp(dades, equips):
    """ dhsaui"""

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print_if_debug(DEBUG, 'S\'ha creat el socket UDP')

    def signal_handler(sign, frame):
        """ signal_handler """
        if sign == signal.SIGINT:
            print('\nHas premut ^C.')
            sock.close()
            sys.exit()

    break_while = False
    signal.signal(signal.SIGINT, signal_handler)
    sock.bind(('', int(DADES['UDP-port'])))
    print_if_debug(DEBUG, 'Assignat el socket al port: ' + DADES['UDP-port'])
    while not break_while:
        buff, address = sock.recvfrom(78)
        print_if_debug(DEBUG, 'Rebut ' + to_str_dades_udp(buff))
        tractar_dades_udp(buff, sock, address, equips, dades)
    sock.close()


if __name__ == '__main__':
    DEBUG, ARXIUS = lectura_parametres()
    print_if_debug(DEBUG, 'Parametres de configuracio llegits.')
    DADES = agafar_dades_servidor(ARXIUS['servidor'])
    print_with_time('INFO  => Llegits parametres arxiu de configuracio')
    EQUIPS = agafar_dades_equips(ARXIUS['equips'])
    print_with_time('INFO  => Llegits ' + str(len(EQUIPS)) + ' equips autoritzats en el sistema')
    udp(DADES, EQUIPS)
