version_hash = 184

for version in range(300):
    str_version = str(version)
    factor = 0
    for c in str_version:
        factor = ((factor * 0x20) + ord(c)) + 1
    n1 = (factor >> 0x18) & 0xff
    n2 = (factor >> 0x10) & 0xff
    n3 = (factor >> 0x8) & 0xff
    n4 = (factor) & 0xff
    hash_calc  = ~(n1^n2^n3^n4) & 0xff
    if hash_calc == version_hash:
        print('version: ' +  str_version)
