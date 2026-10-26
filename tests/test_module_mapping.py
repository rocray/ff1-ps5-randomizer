"""Check converted ELF load mappings against the linked source sections.

Usage: python tests/test_module_mapping.py build/llvm-pie.elf build/eboot.elf
This catches a GOT offset incorrectly used as the start of .data.rel.ro.
"""
import struct
import sys
from pathlib import Path


def headers(data):
    offset = struct.unpack_from('<Q', data, 32)[0]
    size, count = struct.unpack_from('<HH', data, 54)
    return [struct.unpack_from('<IIQQQQQQ', data, offset + i * size)
            for i in range(count)]


source, target = (Path(p).read_bytes() for p in sys.argv[1:])
loads = [p for p in headers(target) if p[0] == 1]
for p in loads:
    assert p[2] % p[7] == p[3] % p[7], 'misaligned load segment'
    assert p[2] + p[5] <= len(target), 'truncated load segment'

offset = struct.unpack_from('<Q', source, 40)[0]
size, count, strings_index = struct.unpack_from('<HHH', source, 58)
sections = [struct.unpack_from('<IIQQQQIIQQ', source, offset + i * size)
            for i in range(count)]
strings = sections[strings_index]
names = source[strings[4]:strings[4] + strings[5]]
checked = []
for s in sections:
    name = names[s[0]:].split(b'\0', 1)[0].decode()
    if not (name.startswith('.data.rel.ro') or name in
            ('.text', '.rodata', '.data', '.got', '.got.plt', '.init_array')):
        continue
    if not s[5]:
        continue
    matches = [p for p in loads if p[3] <= s[3] and s[3] + s[5] <= p[3] + p[5]]
    assert len(matches) == 1, (name, 'no unique file-backed load mapping')
    p = matches[0]
    start = p[2] + s[3] - p[3]
    assert target[start:start + s[5]] == source[s[4]:s[4] + s[5]], name
    checked.append(name)
assert '.data.rel.ro' in checked, 'regression fixture must contain pointer tables'
print('PASS: load alignment and mapped section bytes:', ', '.join(checked))
