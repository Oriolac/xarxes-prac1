#!/usr/bin/env python
"""
Servidor
"""

import server_data as serdat
import sys
import time
import socket
import signal
import struct
import threading
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


def imprimir_per_pantalla(equips):
    for equip in equips:
        if equip['estat'].__eq__('DISCONNECTED'):
            print(' ' + equip['nom'] + '\t\t\t' + equip['mac'] + '\t\t' + equip['estat'])
        else:
            print(' ' + equip['nom'] + '\t' + str(equip['address'][0]) + '/' + str(equip['address'][1]) + '\t' + equip['mac'] + '\t' + equip['aleatori'] + '\t' + equip['estat'])


def comandes_consola(DEBUG, EQUIPS, quit, pid):
    print_if_debug(DEBUG, 'Thread per tractar comandes per consola obert')
    while not quit:
        comanda = raw_input('')
        if comanda.__eq__('quit'):
            os.kill(pid, signal.SIGINT)
            quit = True
        elif comanda.__eq__('list'):
            print('-NOM--\t------IP-------\t-----MAC-----\t-ALEA-\t----ESTAT----')
            imprimir_per_pantalla(EQUIPS)


def to_str_dades_udp(dades):
    """ to_str_dades_udp """
    return 'bytes=' + str(len(dades)) + ', tipus=' + str(serdat.to_str_tipus(dades[0])) + ', nom=' + \
        dades[1:7] + ', mac=' + dades[8:20] + ', alea=' + dades[21:27] + ', dades=' + dades[27:]


def create_empty_pack(type, data):
    """ create_rej_pack    """
    camps = ['', '', '', '']
    llargada_camps = (7, 13, 7, 50)
    index_camps = 0
    for llargada in llargada_camps:
        camps[index_camps] = camps[index_camps].zfill(llargada)
        index_camps += 1
    return struct.pack('c7s13s7s50s', chr(type), '', '', '', data)


def enviar_reg_rej(sock, address, data):
    """ enviar_paquet_udp_rej """
    pack = create_empty_pack(0x03, data)
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(pack))
    sock.sendto(pack, address)


def enviar_reg_ack(sock, address, dades_serv, equip):
    """ enviar_paquet_ack """

    def num_aleatori():
        import random
        paraula = str(random.randint(0, 1000000))
        while len(paraula) < 6:
            paraula = '0' + paraula
        return paraula

    if equip['aleatori'].__eq__('000000'):
        equip['aleatori'] = num_aleatori()
    data = struct.pack('c7s13s7s50s', chr(0x01), dades_serv['Nom'], dades_serv['MAC'], equip['aleatori'],
                       dades_serv['TCP-port'])
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(data))
    sock.sendto(data, address)


def enviar_paquet_err(sock, address):
    data = struct.pack('c7s13s7s50s', chr(0x09), '', '', '', '')
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(data))
    sock.sendto(data, address)


def enviar_reg_nack(sock, address, dades):
    pack = create_empty_pack(0x02, dades)
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(pack))
    sock.sendto(pack, address)


def confirmacio_registre(data, sock, address, equips, dades_servidor):
    for equip in equips:
        if equip['nom'].__eq__(data[1:6]) and equip['mac'].__eq__(data[8:20]):
            if data[21:27].__eq__(equip['aleatori']) or not equip['estat'].__eq__('DISCONNECTED'):
                if equip['estat'].__eq__('DISCONNECTED'):
                    equip['estat'] = 'REGISTERED'
                    equip['address'] = address
                    print_if_debug(DEBUG,
                        'Acceptat registre. Equip: nom=' + equip['nom'] + ', ip=' + address[0] + ', mac=' +
                        equip['mac'] + ', alea=' + data[21:27])
                    print_with_time('MSG.  => L\'equip ' + data[1:6] + ' passa a estat ' + equip['estat'])
                    enviar_reg_ack(sock, address, dades_servidor, equip)
                    return True, equip
                elif not equip['address'].__eq__(address):
                    enviar_reg_nack(sock, address, 'Discrepancies amb IP')
                    return False, equip
                else:
                    enviar_reg_ack(sock, address, dades_servidor, equip)
                    return True, equip
            else:
                enviar_reg_nack(sock, address, 'Discrepancies amb el nombre aleatori')
                return False, equip

    enviar_reg_rej(sock, address, 'Equip no autoritzat en el sistema.')
    return False, None


def enviar_alive_ack(sock, address, dades_serv, equip):
    data = struct.pack('c7s13s7s50s', chr(0x11), dades_serv['Nom'], dades_serv['MAC'], equip['aleatori'], '')
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(data))
    sock.sendto(data, address)


def enviar_alive_rej(sock, address, motiu):
    pack = create_empty_pack(0x13, motiu)
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(pack))
    sock.sendto(pack, address)


def enviar_alive_nack(sock, address, motiu):
    pack = create_empty_pack(0x12, motiu)
    print_if_debug(DEBUG, 'Enviat ' + to_str_dades_udp(pack))
    sock.sendto(pack, address)


def control_manteniment_comunicacio(data, sock, address, equips, dades_serv):
    for equip in equips:
        if equip['nom'].__eq__(data[1:6]) and equip['mac'].__eq__(data[8:20]) and equip['address'].__eq__(address) \
                and equip['aleatori'].__eq__(data[21:27]):
            if equip['estat'].__eq__('REGISTERED'):
                equip['cont_alives'] = 0
                equip['estat'] = 'ALIVE'
                print_with_time('MSG.  => L\'equip ' + equip['nom'] + ' passa de REGISTERED a ALIVE.')
                enviar_alive_ack(sock, address, dades_serv, equip)
            elif equip['estat'].__eq__('ALIVE'):
                equip['cont_alives'] = 0
                enviar_alive_ack(sock, address, dades_serv, equip)
            else:
                enviar_alive_rej(sock, address, 'Equip no registrat al sistema.')
        elif equip['nom'].__eq__(data[1:6]) and equip['mac'].__eq__(data[8:20]) and not equip['address'].__eq__(address) \
                and equip['aleatori'].__eq__(data[21:27]):
            enviar_alive_nack(sock, address, 'Discrepancies amb IP address')
        elif equip['nom'].__eq__(data[1:6]) and equip['mac'].__eq__(data[8:20]) and equip['address'].__eq__(address) \
                and not equip['aleatori'].__eq__(data[21:27]):
            enviar_alive_nack(sock, address, 'Discrepancies amb el nombre aleatori')


def tractar_dades_udp(data, sock, address, equips, dades_servidor):
    """ fadsfa """

    if ord(data[0]) == 0x00:
        confirmacio, equip = confirmacio_registre(data, sock, address, equips, dades_servidor)
        if confirmacio:
            print_if_debug(DEBUG, 'Contador ALIVES = 0')
            equip['cont_alives'] = 0
    elif ord(data[0]) == 0x10:
        control_manteniment_comunicacio(data, sock, address, equips, dades_servidor)
    else:
        enviar_paquet_err(sock, address)


def udp(dades, equips, quit_command):
    """ dhsaui"""

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print_if_debug(DEBUG, 'S\'ha creat el socket UDP')

    def quit_handler(sign, frame):
        """ signal_handler """
        if sign == signal.SIGINT:
            global quit_command
            print_if_debug(DEBUG, 'S\'ha executat la comanda \'quit\'. Finalitzat servidor.')
            quit_command = True
            sock.close()
            sys.exit()

    def manteniment_alives():
        global DEBUG, EQUIPS, quit_command
        j_intervals = 2
        k_intervals = 3
        temps_r = 3
        print_if_debug(DEBUG, 'Establert temporitzador pel control d\'alives')
        while not quit_command:
            time.sleep(temps_r)
            for equip in EQUIPS:
                if not equip['estat'].__eq__('DISCONNECTED'):
                    equip['cont_alives'] += 1

                if equip['estat'].__eq__('REGISTERED') and equip['cont_alives'] > j_intervals:
                    equip['estat'] = 'DISCONNECTED'
                    print_with_time('MSG.  => L\'equip ' + equip['nom'] + ' passa de REGISTERED a DISCONNECTED')
                    print_if_debug(DEBUG, 'no s\'ha rebut ' + str(j_intervals) + ' ALIVE_INF')
                elif equip['estat'].__eq__('ALIVE') and equip['cont_alives'] > k_intervals:
                    equip['estat'] = 'DISCONNECTED'
                    print_with_time('MSG.  => L\'equip ' + equip['nom'] + ' passa d\'ALIVE a DISCONNECTED')
                    print_if_debug(DEBUG, 'no s\'ha rebut ' + str(k_intervals) + ' ALIVE_INF')
        print_if_debug(DEBUG, 'S\'ha tancat fil per manteniment d\'ALIVES')

    control_alives = threading.Thread(target=manteniment_alives)
    control_alives.start()
    signal.signal(signal.SIGINT, quit_handler)
    sock.bind(('', int(DADES['UDP-port'])))
    print_if_debug(DEBUG, 'Assignat el socket al port: ' + DADES['UDP-port'])

    while not quit_command:
        buff, address = sock.recvfrom(78)
        print_if_debug(DEBUG, 'Rebut ' + to_str_dades_udp(buff))
        tractar_dades_udp(buff, sock, address, equips, dades)
    sock.close()


if __name__ == '__main__':
    quit_command = False
    DEBUG, ARXIUS = serdat.lectura_parametres()
    print_if_debug(DEBUG, 'Parametres de configuracio llegits.')
    DADES = serdat.agafar_dades_servidor(ARXIUS['servidor'])
    print_with_time('INFO  => Llegits parametres arxiu de configuracio')
    EQUIPS = serdat.agafar_dades_equips(ARXIUS['equips'])
    print_with_time('INFO  => Llegits ' + str(len(EQUIPS)) + ' equips autoritzats en el sistema')
    consola = threading.Thread(target=comandes_consola, args=(DEBUG, EQUIPS, quit_command, os.getpid()))
    consola.start()
    udp(DADES, EQUIPS, quit_command)
