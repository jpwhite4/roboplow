import binascii

def convert(input):
    return (input - 1500) / 4; 

def main2():
    for i in xrange(1000, 2000):
        print i, convert(i)

def translate(command):
    if command == 10:
        return 'RIGHT TURN'
    if command == 11:
        return 'LEFT TURN'
    if command == 8:
        return 'FORWARDS'
    if command == 9:
        return 'BACKWARDS'
    return 'UNKNOWN ({})'.format(command)

def main():
    with open("screenlog.0", "rb") as fp:
        datum = fp.read(1)
        while ord(datum[0]) != 128:
            datum = fp.read(1)

        datum = fp.read(4)
        while datum != "":
            if ord(datum[0]) == 128:
                command = ord(datum[1])
                value = ord(datum[2])
                chsum = ord(datum[3])

                valid = 'VALID' if chsum == (command + value + 128) % 128 else 'ERR ({} {})'.format(chsum, (command + value + 128) % 128)
                print translate(command), value, valid
            datum = fp.read(4)
            
if __name__ == "__main__":
    main()
