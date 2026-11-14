from struct import pack as _c_pack, unpack as _c_unpack


def _c_f32(valor):
    return _c_unpack("=f", _c_pack("=f", valor))[0]


def main():
    calificacion1 = _c_f32(8.5)
    calificacion2 = _c_f32(9.0)
    calificacion3 = _c_f32(7.5)
    promedio = _c_f32(_c_f32(_c_f32(calificacion1 + calificacion2) + calificacion3) / _c_f32(3))
    print("El promedio es: %.2f\n" % (promedio,), end="")
    if promedio >= 6.0:
        print("El alumno aprobo\n", end="")
    else:
        print("El alumno no aprobo\n", end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
