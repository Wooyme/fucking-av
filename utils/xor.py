import sys


def xor_encrypt_file(input_file_path, output_file_path, key):
    with open(input_file_path, 'rb') as input_file:
        input_data = input_file.read()

    key_length = len(key)
    output_data = bytearray(input_data)

    for i in range(len(input_data)):
        output_data[i] ^= key[i % key_length]

    with open(output_file_path, 'wb') as output_file:
        output_file.write(output_data)


if __name__ == '__main__':
    xor_encrypt_file(sys.argv[1], sys.argv[2], sys.argv[3].encode('ascii'))
