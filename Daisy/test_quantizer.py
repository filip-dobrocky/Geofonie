"""Mirror of the gen~ quantizer codebox in geofonie_gen.maxpat. Run to check the scale math."""
import math

MAJOR = [0, 2, 4, 5, 7, 9, 11]


def quantize(note, root, mode):
    mode = int(mode) % 7
    tonic = MAJOR[mode]
    note = note - root
    octave = math.floor(note / 12)
    degree = note - octave * 12

    min_dif, q_deg = 127, 0
    for i in range(7):
        d = (MAJOR[(i + mode) % 7] - tonic + 12) % 12
        dif = abs(degree - d)
        if dif < min_dif:
            min_dif, q_deg = dif, d
    return root + q_deg + octave * 12


def degrees(mode):
    return sorted({quantize(n, 0, mode) for n in range(12)})


if __name__ == "__main__":
    assert degrees(0) == MAJOR, degrees(0)
    assert degrees(1) == [0, 2, 3, 5, 7, 9, 10], degrees(1)   # dorian
    assert degrees(5) == [0, 2, 3, 5, 7, 8, 10], degrees(5)   # aeolian
    assert degrees(6) == [0, 1, 3, 5, 6, 8, 10], degrees(6)   # locrian

    assert quantize(66, 60, 0) == 65          # nearest degree
    assert quantize(-1, 0, 0) == -1           # below the root: octave/degree stay consistent
    assert all(quantize(n, 7, 3) % 12 in [(d + 7) % 12 for d in degrees(3)] for n in range(36, 96))
    print("ok")
