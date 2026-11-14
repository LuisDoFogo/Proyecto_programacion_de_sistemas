"""Pruebas reales del traductor: todos los productos se crean fuera del proyecto."""

from pathlib import Path
import ast
import hashlib
import os
import subprocess
import sys
import tempfile


RAIZ = Path(__file__).resolve().parent.parent
COMPILADOR = RAIZ / "compilador.exe"
CC = os.environ.get("CC", "gcc")


def ejecutar(argumentos):
    return subprocess.run(argumentos, cwd=RAIZ, capture_output=True, timeout=30)


def fuente(cuerpo):
    return "#include <stdio.h>\nint main() {\n" + cuerpo + "\n}\n"


def exigir(condicion, mensaje):
    if not condicion:
        raise AssertionError(mensaje)


def diagnostico(resultado):
    return (resultado.stdout + resultado.stderr).decode("utf-8", errors="replace")


def archivos_protegidos():
    archivos = [p for carpeta in ("ejemplos", "pruebas")
                for p in (RAIZ / carpeta).rglob("*") if p.is_file()]
    archivos += list(RAIZ.glob("*.pdf"))
    return {p: hashlib.sha256(p.read_bytes()).hexdigest() for p in archivos}


def main():
    protegidos = archivos_protegidos()
    total = fallos = 0
    print("| Prueba | Esperado | Obtenido | Estado |")
    print("|---|---|---|---|")

    def probar(nombre, esperado, accion):
        nonlocal total, fallos
        total += 1
        try:
            obtenido = accion()
            print(f"| {nombre} | {esperado} | {obtenido} | OK |")
        except (AssertionError, OSError, SyntaxError, subprocess.TimeoutExpired) as error:
            fallos += 1
            print(f"| {nombre} | {esperado} | FALLO | FALLO |")
            print(f"{nombre}: {error}", file=sys.stderr)

    validos = [
        ("nombres conservados", 'int puntos = 12; int extra = 3; puntos = puntos + extra; printf("%d\\n", puntos); return 0;', ["puntos = 12", "extra = 3", "puntos + extra"], ["_c_var_", "_c_f32", "_c_idiv"]),
        ("division entera negativa", 'int a = -7; int b = 3; printf("%d %d %d\\n", a / b, 7 / -3, -7 / -3); return 0;', ["def _c_idiv", "a, b"], ["_c_f32"]),
        ("division entera grande sin convertir a real", 'int x = 2147483647; printf("%d\\n", x / -3); return 0;', ["_c_idiv"], []),
        ("asociatividad derecha", 'int a = 10; int b = 4; int c = 2; printf("%d %d\\n", a - (b - c), a / (b / c)); return 0;', ["a - (b - c)"], []),
        ("precedencia sin parentesis redundantes", 'int a = 2; int b = 3; int c = 4; printf("%d %d\\n", (a) + b * c, (a + b) * c); return 0;', ["a + b * c", "(a + b) * c"], ["((a))"]),
        ("conversion real a entero", 'int a = -3.8; int b = 3.8; a = -7.9; printf("%d %d\\n", a, b); return 0;', ["int(-3.8)", "int(-7.9)"], []),
        ("conversion entero a float", 'float x = 7; x = x / 2; printf("%f\\n", x); return 0;', ["_c_f32(7)", "_c_f32(2)"], []),
        ("redondeo float en operaciones", 'float x = 16777216.0; x = x + 1; printf("%.0f\\n", x); return 0;', ["_c_f32(x + _c_f32(1))"], []),
        ("conversion del operando int a float", 'float x = 16777216.0; int k = 16777217; printf("%.0f\\n", x + k); return 0;', ["_c_f32(k)"], []),
        ("literal double en comparacion", 'float x = 0.1; if (x > 0.1) { printf("mayor\\n"); } else { printf("igual\\n"); } return 0;', ["x > 0.1"], []),
        ("operacion float con literal double", 'float x = 16777216.0; float y = x + 1.0; printf("%.0f\\n", y); return 0;', ["_c_f32(x + 1.0)"], []),
        ("sombreado de variable", 'int x = 4; if (x > 0) { int x = 9; printf("%d\\n", x); } printf("%d\\n", x); return 0;', ["x = 4", "_c_var_2 = 9"], []),
        ("palabras Python y funciones", 'int class = 2; int print = 3; int abs = 4; int _c_idiv = 5; printf("%d %d %d %d\\n", class, print, abs, _c_idiv); return 0;', ["_c_var_1 = 2", "_c_var_4 = 5"], []),
        ("nombres entre bloques hermanos", 'if (1 > 0) { int valor = 3; printf("%d\\n", valor); } if (2 > 0) { int valor = 7; printf("%d\\n", valor); } return 0;', ["valor = 3", "_c_var_2 = 7"], []),
        ("inicializacion en ambas ramas", 'int x; int n = -1; if (n > 0) { x = 4; } else { x = 8; } printf("%d\\n", x); return 0;', ["x = 4", "x = 8"], ["x = 0"]),
        ("return excluye una ruta", 'int x; int n = -1; if (n > 0) { return 7; } else { x = 8; } printf("%d\\n", x); return 0;', ["return 7"], []),
        ("else if y estados anidados", 'int x; int n = 0; if (n > 0) { x = 1; } else if (n < 0) { x = 2; } else { x = 3; } printf("%d\\n", x); return 0;', ["elif n < 0:"], []),
        ("bloque que solo declara", 'if (1 > 0) { int x; } return 0;', ["        pass"], []),
        ("main que solo declara", 'int x;', ["    pass"], ["x = 0"]),
        ("retorno anticipado y codigo", 'int n = 3; if (n > 0) { return 7; } printf("no debe aparecer\\n"); return 0;', ["return 7"], []),
        ("retorno real convertido", 'return 3.8;', ["return int(3.8)"], []),
        ("fin de main sin return", 'printf("sin salto");', ['print("sin salto", end="")'], []),
        ("printf porcentajes y varios argumentos", 'int x = 3; float y = 2.5; printf("100%%\\n"); printf("%d %.3f %%", x, y); return 0;', ['"100%%\\n" % ()', "x, y,"], []),
        ("escapes y comillas", 'printf("\\\"hola\\\"\\tbarra: \\\\\\n"); return 0;', [], []),
        ("rama no ejecutada conserva division", 'int x = 0; if (x > 0) { int y = 1 / x; printf("%d", y); } printf("bien\\n"); return 0;', ["_c_idiv(1, x)"], []),
        ("literales octales normalizados", 'int x = 010; printf("%d\\n", x); return 0;', ["x = 8"], []),
        ("literales reales con punto", 'float x = .5; float y = 2.; printf("%.2f\\n", x + y); return 0;', [], []),
        ("conversion en comparacion mixta", 'float x = 16777216.0; int n = 16777217; if (x == n) { printf("iguales\\n"); } else { printf("distintos\\n"); } return 0;', ["x == _c_f32(n)"], []),
        ("sombreado no cambia inicializacion exterior", 'int x; if (1 > 0) { int x = 8; printf("%d", x); } x = 2; printf("%d", x); return 0;', ["_c_var_2 = 8", "x = 2"], []),
    ]
    invalidos = [
        ("asignacion no declarada", "x = 2; return 0;", "no declarada"),
        ("lectura no declarada", 'printf("%d", x); return 0;', "no declarada"),
        ("declaracion duplicada", "int x = 1; int x = 2; return 0;", "ya esta declarada"),
        ("uso fuera del bloque", 'if (1 > 0) { int x = 2; } printf("%d", x); return 0;', "no declarada"),
        ("lectura sin inicializar", 'int x; printf("%d", x); return 0;', "sin inicializacion"),
        ("autoinicializacion", "int x = x + 1; return 0;", "sin inicializacion"),
        ("autoinicializacion al sombrear", "int x = 1; if (x > 0) { int x = x + 1; } return 0;", "sin inicializacion"),
        ("asignacion en una sola rama", 'int x; if (1 > 0) { x = 2; } printf("%d", x); return 0;', "sin inicializacion"),
        ("rama alternativa no inicializada", 'int x; if (1 > 0) { x = 2; } else { printf("otro"); } return x;', "sin inicializacion"),
        ("estado no se filtra a else", "int x; if (1 > 0) { x = 2; } else { int y = x; } return 0;", "sin inicializacion"),
        ("falta argumento printf", 'printf("%d"); return 0;', "requiere 1 argumentos"),
        ("sobran argumentos printf", 'printf("hola", 2); return 0;', "requiere 0 argumentos"),
        ("tipo incorrecto para d", 'float x = 2.5; printf("%d", x); return 0;', "tipo incompatible"),
        ("tipo incorrecto para f", 'int x = 2; printf("%f", x); return 0;', "tipo incompatible"),
        ("formato fuera del perfil", 'printf("%x", 12); return 0;', "formato printf no admitido"),
        ("porcentaje sin completar", 'printf("100%"); return 0;', "formato printf no admitido"),
        ("precision sin numero", 'printf("%.f", 2.0); return 0;', "precision de printf"),
        ("escape fuera del perfil", 'printf("\\q"); return 0;', "escape"),
        ("literal entero fuera de rango", "int x = 2147483648; return 0;", "literal entero"),
        ("octal invalido", "int x = 08; return 0;", "literal entero"),
        ("limite de simbolos", " ".join(f"int x{i};" for i in range(101)) + " return 0;", "limite de 100"),
        ("error lexico", "@ return 0;", "Error lexico"),
        ("error sintactico", "int x = ; return 0;", "Error sintactico"),
        ("comentario sin cerrar", "return 0; /*", "comentario sin cerrar"),
    ]

    with tempfile.TemporaryDirectory(prefix="ps-etapa4-") as carpeta:
        temporal = Path(carpeta)
        secuencia = 0

        def comparar(nombre, codigo, presentes=(), ausentes=()):
            nonlocal secuencia
            secuencia += 1
            entrada = temporal / f"origen_{secuencia}.c"
            salida = temporal / f"destino_{secuencia}.py"
            binario = temporal / f"referencia_{secuencia}.exe"
            entrada.write_text(codigo, encoding="utf-8")
            traduccion = ejecutar([str(COMPILADOR), str(entrada), str(salida)])
            exigir(traduccion.returncode == 0, diagnostico(traduccion))
            exigir(not traduccion.stderr, diagnostico(traduccion))
            generado = salida.read_text(encoding="utf-8")
            ast.parse(generado, filename=str(salida))
            for texto in presentes:
                exigir(texto in generado, f"falta {texto!r} en:\n{generado}")
            for texto in ausentes:
                exigir(texto not in generado, f"aparece {texto!r} en:\n{generado}")
            compilacion = ejecutar([CC, "-std=c11", "-Wall", "-Wextra", str(entrada), "-o", str(binario)])
            exigir(compilacion.returncode == 0, diagnostico(compilacion))
            original = ejecutar([str(binario)])
            python = ejecutar([sys.executable, "-B", str(salida)])
            exigir(not original.stderr and not python.stderr, diagnostico(original) + diagnostico(python))
            exigir(original.returncode == python.returncode,
                   f"codigos C={original.returncode}, Python={python.returncode}")
            exigir(original.stdout.replace(b"\r\n", b"\n") == python.stdout.replace(b"\r\n", b"\n"),
                   f"salidas C={original.stdout!r}, Python={python.stdout!r}\n{generado}")
            return f"sintaxis OK; C=Python; codigo {python.returncode}"

        # Se cambian los nombres de archivo para descartar respuestas predefinidas.
        for numero in (1, 2, 3):
            codigo = (RAIZ / "ejemplos" / f"ejemplo{numero}.c").read_text(encoding="utf-8")
            probar(f"original {numero} con otra ruta", "C=Python", lambda c=codigo: comparar("original", c))
        for nombre, cuerpo, presentes, ausentes in validos:
            probar(nombre, "sintaxis valida; C=Python",
                   lambda n=nombre, c=cuerpo, p=presentes, a=ausentes: comparar(n, fuente(c), p, a))

        def rechazar(cuerpo, mensaje):
            entrada = temporal / "invalido.c"
            entrada.write_text(fuente(cuerpo), encoding="utf-8")
            # Un destino nuevo no debe aparecer; uno existente debe conservarse.
            for anterior in (None, b"# salida anterior valida\n"):
                salida = temporal / ("rechazado_nuevo.py" if anterior is None else "rechazado_previo.py")
                if anterior is not None:
                    salida.write_bytes(anterior)
                resultado = ejecutar([str(COMPILADOR), str(entrada), str(salida)])
                exigir(resultado.returncode == 1, diagnostico(resultado))
                exigir(mensaje in resultado.stderr.decode("utf-8", errors="replace"), diagnostico(resultado))
                exigir(b"Traduccion correcta" not in resultado.stdout, diagnostico(resultado))
                exigir((not salida.exists()) if anterior is None else salida.read_bytes() == anterior,
                       "se publico o modifico una salida para una entrada invalida")
                exigir(not list(temporal.glob(salida.name + ".tmp.*")), "quedo un temporal incompleto")
            return "codigo 1; destino nuevo ausente y anterior intacto"

        for nombre, cuerpo, mensaje in invalidos:
            probar(nombre, "rechazo sin publicar", lambda c=cuerpo, m=mensaje: rechazar(c, m))

        def publicar_dos_veces():
            entrada = temporal / "reutilizable.c"
            salida = temporal / "reutilizable.py"
            for valor in (3, 11):
                entrada.write_text(fuente(f'printf("%d", {valor}); return 0;'), encoding="utf-8")
                resultado = ejecutar([str(COMPILADOR), str(entrada), str(salida)])
                exigir(resultado.returncode == 0, diagnostico(resultado))
                resultado = ejecutar([sys.executable, "-B", str(salida)])
                exigir(resultado.returncode == 0 and resultado.stdout == str(valor).encode(), diagnostico(resultado))
            exigir(not list(temporal.glob(salida.name + ".tmp.*")), "quedo un temporal")
            return "salida reemplazada correctamente"

        probar("reemplazo de salida valida", "nuevo contenido completo", publicar_dos_veces)

        def fallo_ruta(tipo):
            entrada = temporal / "seguro.c"
            codigo = fuente("int valor = 3; return valor;").encode()
            entrada.write_bytes(codigo)
            if tipo == "misma ruta":
                salida = entrada
            elif tipo == "enlace al original":
                salida = temporal / "enlace.c"
                os.link(entrada, salida)
            elif tipo == "destino directorio":
                salida = temporal / "directorio.py"
                salida.mkdir()
            elif tipo == "directorio inexistente":
                salida = temporal / "inexistente" / "salida.py"
            else:
                salida = temporal / "solo_lectura.py"
                salida.write_text("# anterior\n", encoding="utf-8")
                salida.chmod(0o444)
            try:
                resultado = ejecutar([str(COMPILADOR), str(entrada), str(salida)])
                exigir(resultado.returncode == 1, diagnostico(resultado))
                exigir(entrada.read_bytes() == codigo, "se modifico la entrada")
                if tipo == "solo lectura":
                    exigir(salida.read_text() == "# anterior\n", "se modifico la salida protegida")
                exigir(not list(salida.parent.glob(salida.name + ".tmp.*")), "quedo un temporal")
            finally:
                if tipo == "solo lectura":
                    salida.chmod(0o666)
            return "codigo 1; entrada intacta; sin temporales"

        for tipo in ("misma ruta", "enlace al original", "destino directorio", "directorio inexistente", "solo lectura"):
            probar(tipo, "fallo seguro", lambda t=tipo: fallo_ruta(t))

        # Un mismo esquema con valores y nombres distintos comprueba generalidad.
        for i in range(5):
            cuerpo = (f'int dato{i} = {i + 2}; int divisor{i} = {i + 1}; '
                      f'if (dato{i} > divisor{i}) {{ dato{i} = -dato{i}; }} '
                      f'printf("%d\\n", dato{i} / divisor{i}); return 0;')
            probar(f"programa adicional variable {i}", "C=Python", lambda c=cuerpo: comparar("variable", fuente(c)))

        def comprobar_originales():
            exigir(archivos_protegidos() == protegidos, "cambio un archivo protegido")
            return f"{len(protegidos)} archivos SHA-256 intactos"

        probar("integridad de fuentes y pruebas", "SHA-256 iguales", comprobar_originales)

    print(f"\n{total - fallos}/{total} pruebas de etapa 4 aprobadas.")
    return int(fallos != 0)


if __name__ == "__main__":
    raise SystemExit(main())
