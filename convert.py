import sys
import struct

INPUT = 772
HIDDEN = 128

path = sys.argv[1]

with open(path, "rb") as f:
    data = f.read()

offset = 0

# W1
count = INPUT * HIDDEN
W1_flat = struct.unpack_from(f"{count}f", data, offset)
offset += count * 4

W1 = [
    list(W1_flat[i * HIDDEN:(i + 1) * HIDDEN])
    for i in range(INPUT)
]

# B1
B1 = list(struct.unpack_from(f"{HIDDEN}f", data, offset))
offset += HIDDEN * 4

# W2
W2 = list(struct.unpack_from(f"{HIDDEN}f", data, offset))
offset += HIDDEN * 4

# B2
B2 = struct.unpack_from("f", data, offset)[0]
offset += 4

with open("src/network.h", "w") as f:
    f.write("#pragma once\n\n#include \"nnue.h\"\n\nconst Network NNUE = {\n")

    # W1
    f.write("\t{\n")
    for i in range(0, INPUT):
        f.write("\t\t{ ")
        for j in range(0, HIDDEN):
            f.write(str(W1[i][j]))
            if j < HIDDEN - 1:
                f.write(", ")
        f.write(" }")
        if i < INPUT - 1:
            f.write(",")
        f.write("\n")
    f.write("\t},\n")

    # B1
    f.write("\t{ ")
    for i in range(0, HIDDEN):
        f.write(str(B1[i]))
        if i < INPUT - 1:
            f.write(", ")
    f.write(" },\n")

    # W2
    f.write("\t{ ")
    for i in range(0, HIDDEN):
        f.write(str(W2[i]))
        if i < INPUT - 1:
            f.write(", ")
    f.write(" },\n")

    # B2
    f.write("\t"+str(B2))
    f.write("\n")

    f.write("};")