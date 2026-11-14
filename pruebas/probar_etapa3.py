"""Regresiones de la etapa 3. Usa Python estandar y no genera archivos Python."""

from pathlib import Path
import os
import subprocess
import sys
import tempfile


RAIZ = Path(__file__).resolve().parent.parent
COMPILADOR = RAIZ / "compilador.exe"


def ejecutar(argumentos, entrada=None):
    return subprocess.run(
        argumentos, input=entrada, cwd=RAIZ, capture_output=True, timeout=30
    )


def programa(cuerpo):
    return ("#include <stdio.h>\nint main() {\n" + cuerpo + "\n}\n").encode()


def main():
    if not COMPILADOR.is_file():
        print("Primero ejecuta bash compilar.sh en MSYS2 UCRT64.", file=sys.stderr)
        return 1

    # Cada caso incluye el codigo y el diagnostico esperado, cuando corresponde.
    archivos = [
        ("ejemplos/ejemplo1.c", 0, ""),
        ("ejemplos/ejemplo2.c", 0, ""),
        ("ejemplos/ejemplo3.c", 0, ""),
        ("pruebas/valido_comentarios.c", 0, ""),
        ("pruebas/invalido_sin_punto_y_coma.c", 1, "linea 4, columna 5"),
        ("pruebas/invalido_parentesis.c", 1, "linea 4, columna 20"),
        ("pruebas/valido_cadenas_escapadas.c", 0, ""),
        ("pruebas/invalido_cadena_sin_cerrar.c", 1, "linea 3, columna 12"),
        ("pruebas/invalido_comentario_sin_cerrar.c", 1, "linea 3, columna 5"),
        ("pruebas/invalido_caracter_entre_printf.c", 1, "Error lexico"),
        ("pruebas/invalido_texto_entre_cadenas.c", 1, "Error sintactico"),
    ]
    base = programa("return 0;")
    memoria = [
        ("stdin sin argumentos", base, 0, ""),
        ("precedencia y menos unario", programa("int x = -(2 + 3) * 4 / (5 - 1); return x;"), 0, ""),
        ("expresion sin constant folding", programa("if (1 > 2) { int x = 1 / 0; } return 0;"), 0, ""),
        ("comentario al final sin cerrar", base + b"/* sin cierre", 1, "comentario sin cerrar"),
        ("cadena al final sin cerrar", base + b'"sin cierre', 1, "cadena sin cerrar"),
        ("cadena con escape al final del archivo", b'#include <stdio.h>\nint main() { printf("abc' + bytes([92]), 1, "cadena sin cerrar"),
        ("comillas escapadas sin cierre real", b'#include <stdio.h>\nint main() { printf("abc' + bytes([92, 34]), 1, "cadena sin cerrar"),
        ("byte NUL al final", base + b"\x00@", 1, "byte NUL"),
        ("byte NUL inicial", b"\x00" + base, 1, "byte NUL"),
        ("byte NUL en comentario de linea", base + b"// texto\x00", 1, "byte NUL"),
        ("byte NUL en comentario de bloque", base + b"/* texto\x00 */", 1, "byte NUL"),
        ("byte NUL en cadena", programa('printf("abc"); return 0;').replace(b"abc", b"ab\x00c"), 1, "Error lexico"),
        ("caracter invalido al final", base + b"@", 1, "Error lexico"),
        ("comentarios con asteriscos", programa("/***/ /* uno ** dos */ return 0;"), 0, ""),
        ("lineas CRLF", programa("\tint x = 2;\n\treturn x;").replace(b"\n", b"\r\n"), 0, ""),
        ("comentario multilinea y ubicacion", programa("/* primera\nsegunda */\n    @"), 1, "linea 5, columna 5"),
        ("EOF y ubicacion", b"#include <stdio.h>\nint main() {\n    return 0;\n", 1, "linea 4, columna 1"),
        ("identificador pendiente tras error", programa("int nombre = (2 + ); return 0;"), 1, "Error sintactico"),
        ("expresion pendiente tras error", programa("int nombre = 2 + 3 * ; return 0;"), 1, "Error sintactico"),
        ("STRING sigue sin ser argumento aritmetico", programa('printf("%s", "hola"); return 0;'), 1, "Error sintactico"),
        ("alcance original: analisis sin tabla de simbolos", programa("x = 2; return x;"), 0, ""),
    ]

    fallos = 0
    total = 0
    print("| Prueba | Esperado | Obtenido | Estado |")
    print("|---|---:|---:|---|")

    def comprobar(nombre, resultado, esperado, diagnostico):
        nonlocal fallos, total
        total += 1
        salida = resultado.stdout.decode("utf-8", errors="replace")
        error = resultado.stderr.decode("utf-8", errors="replace")
        correcto = resultado.returncode == esperado and diagnostico in error
        if esperado == 0:
            correcto = correcto and not error and "Analisis sintactico correcto" in salida
        else:
            correcto = correcto and "Analisis sintactico correcto" not in salida
        if not correcto:
            fallos += 1
        print(f"| {nombre} | {esperado} | {resultado.returncode} | {'OK' if correcto else 'FALLO'} |")
        if not correcto:
            print(salida + error, file=sys.stderr)

    for archivo, esperado, diagnostico in archivos:
        comprobar(archivo, ejecutar([str(COMPILADOR), archivo]), esperado, diagnostico)
    for nombre, entrada, esperado, diagnostico in memoria:
        comprobar(nombre, ejecutar([str(COMPILADOR)], entrada), esperado, diagnostico)

    # Compilamos una prueba de los valores en un temporal fuera del proyecto.
    with tempfile.TemporaryDirectory(prefix="ps-etapa3-") as carpeta:
        binario = Path(carpeta) / "valores_semanticos.exe"
        compilacion = ejecutar([
            os.environ.get("CC", "gcc"), "-Wall", "-Wextra", "-Werror", "-Isrc",
            "pruebas/valores_semanticos.c", "src/traduccion.c", "-o", str(binario),
        ])
        total += 1
        correcto = compilacion.returncode == 0 and not compilacion.stderr
        obtenido = compilacion.returncode
        if correcto:
            resultado = ejecutar([str(binario)])
            obtenido = resultado.returncode
            correcto = obtenido == 0 and not resultado.stderr
        if not correcto:
            fallos += 1
            print(compilacion.stderr.decode("utf-8", errors="replace"), file=sys.stderr)
        print(f"| valores semanticos (C) | 0 | {obtenido} | {'OK' if correcto else 'FALLO'} |")

    print(f"\n{total - fallos}/{total} pruebas aprobadas.")
    return 1 if fallos else 0


if __name__ == "__main__":
    raise SystemExit(main())
