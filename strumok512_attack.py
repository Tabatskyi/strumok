import ctypes
import secrets
import numpy as np
from strumok_tables import STRUMOK_ALPHA_MUL, STRUMOK_ALPHAINV_MUL, strumok_T0, strumok_T1, strumok_T2, strumok_T3, strumok_T4, strumok_T5, strumok_T6, strumok_T7

np.seterr(over='ignore')


class StrumokState(ctypes.Structure):
    _fields_ = [
        ("sa", ctypes.c_uint64 * 16),
        ("head", ctypes.c_uint8),
        ("reg1", ctypes.c_uint64),
        ("reg2", ctypes.c_uint64),
    ]


lib = ctypes.CDLL('build/Debug/STRUMOK.dll')

lib.strumok512_init.argtypes = [
    ctypes.POINTER(StrumokState),
    ctypes.POINTER(ctypes.c_uint64),
    ctypes.POINTER(ctypes.c_uint64),
]

lib.strumok_next_word.argtypes = [
    ctypes.POINTER(StrumokState)
]
lib.strumok_next_word.restype = ctypes.c_uint64

state = StrumokState()
key = (ctypes.c_uint64 * 8)(*[secrets.randbits(64) for _ in range(8)])
iv  = (ctypes.c_uint64 * 4)(*[secrets.randbits(64) for _ in range(4)])

lib.strumok256_init(ctypes.byref(state), key, iv)

s_actual = []
r1_actual = []
r2_actual = []
z_actual = []


for i in range(16):
    s_actual.append(state.sa[(i+state.head)%16])
r1_actual.append(state.reg1)
r2_actual.append(state.reg2)


for _ in range(11):
    z_actual.append(lib.strumok_next_word(ctypes.byref(state)))
    s_actual.append(state.sa[state.head-1])
    r1_actual.append(state.reg1)
    r2_actual.append(state.reg2)


def a_mul(x):
    return (x << 8) ^ STRUMOK_ALPHA_MUL[x >> 56]


def a_inv_mul(x):
    return (x >> 8) ^ STRUMOK_ALPHAINV_MUL[x & 0xff]


def t(x):
    return strumok_T0[x & 0xff] ^ strumok_T1[(x >> 8) & 0xff] ^ strumok_T2[(x >> 16) & 0xff] ^ strumok_T3[(x >> 24) & 0xff] ^ strumok_T4[(x >> 32) & 0xff] ^ strumok_T5[(x >> 40) & 0xff] ^ strumok_T6[(x >> 48) & 0xff] ^ strumok_T7[(x >> 56) & 0xff];


def fsm(x, y, z):
    return (x + y) ^ z


s = np.array([0] * 11 + [s_actual[11], s_actual[12], s_actual[13], s_actual[14], s_actual[15]] + [0] * 11, dtype=np.uint64)
r1 = np.array([r1_actual[0]] + [0] * 11, dtype=np.uint64)
r2 = np.array([r2_actual[0]] + [0] * 11, dtype=np.uint64)
z = np.array(z_actual, dtype=np.uint64)

print('Given S values: ', s)
print('Given R1: ', r1[0])
print('Given R2: ', r2[0])
print('Given output: ', z)
print()

# (Si15 + r1i xor r2i) xor si = zi --> si = (Si15 + r1i xor r2i) xor zi
# si16 = (Si * a) xor (si11 * a^-1) xor si13
# r2_i1 = T(r1_i)
# r1_i1 = r2_i + si13

for i in range(11):
    s[i] = fsm(s[i+15], r1[i], r2[i]) ^ z[i]
    r1[i+1] = r2[i] + s[i+13]
    r2[i+1] = t(r1[i])
    s[i+16] = a_mul(s[i]) ^ a_inv_mul(s[i+11]) ^ s[i+13]


ch = 0
if np.array_equal(s, np.array(s_actual)):
    print('S correctly backtrased')
    ch += 1
if np.array_equal(r1, np.array(r1_actual)):
    print('R1 correctly backtrased')
    ch += 1
if np.array_equal(r2, np.array(r2_actual)):
    print('R2 correctly backtrased')
    ch += 1

if ch == 3:
    print('\nAttack successful!')
else:
    print('\nAttack unsuccessful!')