# documentation for .service files
# '>' sends something, '<' receives something and validates it against data
# use 'binary data (0|1), reads in byte (8 bits) chunks
# refer to examples for more detailed info

import getopt
import socket
import subprocess
import sys
import binascii
import time


def main(argv):
    opts, args = getopt.getopt(argv, "f:", ["file="])

    input_file = ''
    p_args = ''
    program_path = ''
    for opt, arg in opts:
        if opt in ("-f", "--file"):
            input_file = arg

    f = open(input_file, "r")
    contents = f.read()

    socket_data_list = []

    data = ''
    comment = True
    read_new_socket_data = False
    read_program_args = False
    current_socket_data = -1

    for char in contents:
        if comment and char == ':':
            data = ''
            comment = False
            continue
        elif comment:
            continue

        if read_new_socket_data:
            if char == ']':
                socket_data = data.split('|')
                if len(socket_data) == 1:
                    if current_socket_data != -1:
                        socket_data_list[current_socket_data][0] = sock
                    sock = socket_data_list[int(socket_data[0])][0]
                    address = socket_data_list[int(socket_data[0])][1]
                    port = socket_data_list[int(socket_data[0])][2]
                    current_socket_data = int(socket_data[0])
                else:
                    socket_data_list.insert(int(socket_data[0]), (None, socket_data[1], int(socket_data[2])))
                    address = socket_data_list[int(socket_data[0])][1]
                    port = socket_data_list[int(socket_data[0])][2]
                data = ''
                read_new_socket_data = False
            else:
                data += char
        elif read_program_args:
            if char == '}':
                program_data = data.split('|')
                program_path = program_data[0]
                p_args = program_data[1]
                data = ''
                read_program_args = False
            else:
                data += char
        elif char == '[':
            data = ''
            read_new_socket_data = True
        elif char == '{':
            data = ''
            read_program_args = True
        elif char == '>':
            byte_data = bytes.fromhex(data)
            send(sock, byte_data, process)
            data = ''
        elif char == '<':
            byte_data = bytes.fromhex(data)
            rec(sock, byte_data, process)
            data = ''
        elif char == '~':
            sock = setup_server_accept(sock)
        elif char == '^':
            log(f"Starting the Application with arguments '{p_args}'")
            process = subprocess.Popen(f"{program_path} {p_args}".split()) #, stdout=subprocess.PIPE
            time.sleep(1)
        elif char == '%':
            log("Closing connection")
            sock.close()
        elif char == '$':
            log("Checking Command line output in 1 second")
            time.sleep(1)
            out, _err = process.communicate()
            validate(out, bytes.fromhex(data), process)
            data = ''
        elif char == '!' or char == '?':
            if char == '!':
                sock = setup_as_client(address, port)
            else:
                sock = setup_as_server(address, port)
        else:
            data = data + char

    f.close()

    log("Script finished, waiting 2 seconds until cleaning up")
    time.sleep(2)
    return_code = process.poll()
    if return_code is None or return_code != 0:
        if return_code is None:
            err("Application is still running, terminating")
        else:
            err(f"Application returned non-zero ({return_code}), terminating")
        process.terminate()
        exit(-1)
    log("Test finished with no errors")


def setup_as_server(address, port):
    log(f"Creating Server at {address}:{port}")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (address, port)
    sock.bind(server_address)
    sock.listen(1)

    return sock


def setup_server_accept(sock):
    log("Waiting for connection...")

    connection, client_address = sock.accept()
    sock.close()

    log(f"Got connection from {client_address}")

    return connection


def setup_as_client(address, port):
    log(f"Connecting to {address}:{port}")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (address, port)
    sock.connect(server_address)

    return sock


def send(sock, data, process):
    if process is None:
        err("Application is not yet alive, Script error!")
        exit(-1)
    elif process.poll() is not None:
        err(f"Application exited unexpectedly with code {process.poll()}, terminating")
        exit(-1)

    # Send data
    log(f"Sending {binascii.hexlify(data)}")
    sock.sendall(data)
    log("Sent.")
    time.sleep(1)


def validate(received_data, data, process):
    log(f"{binascii.hexlify(received_data)} | Received data")
    log(f"{binascii.hexlify(data)} | Expected data, evaluating...")

    if len(received_data) != len(data):
        err("Received data is not the same length, terminating")
        process.terminate()
        exit(-1)

    for r_byte, o_byte in zip(received_data, data):
        if r_byte != o_byte:
            err("Data is not equal, terminating")
            process.terminate()
            exit(-1)
    log("Verified data as equal")


def rec(sock, data, process):
    if process is None:
        err("Application is not yet alive, Script error!")
        exit(-1)
    elif process.poll() is not None:
        err(f"Application exited unexpectedly with code {process.poll()}, terminating")
        exit(-1)

    received_data = sock.recv(1024)
    if received_data == '':
        err("Socket was closed gracefully by the client, terminating")
        exit(-1)
    log(f"{binascii.hexlify(received_data)} | Received data")
    log(f"{binascii.hexlify(data)} | Expected data, evaluating...")

    if len(received_data) != len(data):
        err("Received data is not the same length, terminating")
        process.terminate()
        exit(-1)

    for r_byte, o_byte in zip(received_data, data):
        if r_byte != o_byte:
            err("Data is not equal, terminating")
            process.terminate()
            exit(-1)
    log("Verified data as equal")


def log(msg):
    print(f"<PY: LOG> {msg}")


def err(msg):
    print(f"<PY: ERR> {msg}")


if __name__ == "__main__":
    main(sys.argv[1:])
