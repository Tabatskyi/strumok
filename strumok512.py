import os

output_dir = os.path.curdir

def strumok512(T=11):
    cipher_name = 'strumok'
    recommended_mg = 9
    recommended_ms = 12
    eqs = '#%s %d Rounds\n' % (cipher_name, T)
    eqs += 'connection relations\n'
    for t in range(T):
        # eqs += 'S_%d, S_%d, S_%d, S_%d\n' % (t + 16, t + 11, t + 2, t)
        # eqs += 'S_%d, R_%d, R_%d, S_%d\n' % (t + 15, t + 1, t, t)
        # eqs += 'R_%d, S_%d, R_%d\n' % (t + 2, t + 5, t)
        eqs += 'S_%d, S_%d, S_%d, S_%d\n' % (t + 16, t + 13, t + 11, t)
        eqs += 'S_%d, R2_%d, R2_%d, S_%d\n' % (t + 15, t + 1, t, t)
        eqs += 'R2_%d, R2_%d, S_%d\n' % (t + 1, t, t + 13)
        # eqs += 'R2_%d, R1_%d\n' % (t + 1, t)
    eqs += 'end'
    eqsfile_path = os.path.join(output_dir, 'relationfile_strumok.txt')
    with open(eqsfile_path, 'w') as relation_file:
        relation_file.write(eqs)


def main():
    strumok512(T=11)


if __name__ == '__main__':
    main()
