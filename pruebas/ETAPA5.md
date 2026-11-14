# Informe de la etapa 5

> Copia pública anonimizada: las rutas locales se sustituyeron por
> marcadores y las invocaciones personales de Python por `python`.
> Los resultados de las pruebas no se modificaron. Se conserva una
> copia íntegra del informe original fuera del repositorio.

Se generaron los tres archivos definitivos de Python exclusivamente con el
transpilador desarrollado con Flex, Bison y C. No se escribió ni se modificó
manualmente su contenido. No fue necesario cambiar el transpilador.

## Estado inicial y protección

Se ejecutó `git status --short` antes de comenzar. Los cambios aprobados de
las etapas anteriores estaban presentes y se conservaron. La carpeta
`salida_python/` todavía no existía.

Se registraron y compararon las huellas SHA-256 de 35 archivos: originales,
material del profesor, PDF, pruebas e informes anteriores y las cinco fuentes
del transpilador/script de compilación. Los 35 permanecieron idénticos.
No se eliminaron pruebas ni se realizaron commits o push.

## Compilación y generación ejecutadas

Se utilizó explícitamente MSYS2 UCRT64 con estas herramientas:

```text
/usr/bin/bash
/usr/bin/bison
/usr/bin/flex
/ucrt64/bin/gcc
```

Se ejecutaron los siguientes comandos desde la raíz:

```bash
bash compilar.sh
mkdir -p salida_python
./compilador.exe ejemplos/ejemplo1.c salida_python/ejemplo1.py
./compilador.exe ejemplos/ejemplo2.c salida_python/ejemplo2.py
./compilador.exe ejemplos/ejemplo3.c salida_python/ejemplo3.py
```

La compilación terminó con código 0, sin advertencias de GCC, Flex o Bison
ni conflictos gramaticales. Cada traducción terminó con código 0 y anunció
su archivo de salida. El script conserva los indicadores de Bison que hacen
fallar la compilación si aparecen conflictos shift/reduce o reduce/reduce.

El flujo de generación fue: Flex reconoce tokens y transmite lexemas mediante
`yylval`; las producciones y acciones semánticas de Bison construyen las
expresiones y sentencias; los auxiliares C validan símbolos y tipos; finalmente
`FILE *archivo_salida` y `fprintf()` escriben el programa en un temporal que
solo se publica cuando termina correctamente.

Los códigos siguientes fueron leídos directamente de los originales y de los
archivos generados; no son traducciones manuales ni respuestas predefinidas.

## Programa 1

Original: `ejemplos/ejemplo1.c`.

```c
#include <stdio.h>

int main() {
    int numero1 = 8;
    int numero2 = 5;
    int suma;
    int resta;
    int multiplicacion;
    suma = numero1 + numero2;
    resta = numero1 - numero2;
    multiplicacion = numero1 * numero2;
    printf("La suma es: %d\n", suma);
    printf("La resta es: %d\n", resta);
    printf("La multiplicacion es: %d\n", multiplicacion);
    return 0;
}
```

Generado: `salida_python/ejemplo1.py`.

```python
def main():
    numero1 = 8
    numero2 = 5
    suma = numero1 + numero2
    resta = numero1 - numero2
    multiplicacion = numero1 * numero2
    print("La suma es: %d\n" % (suma,), end="")
    print("La resta es: %d\n" % (resta,), end="")
    print("La multiplicacion es: %d\n" % (multiplicacion,), end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

## Programa 2

Original: `ejemplos/ejemplo2.c`.

```c
#include <stdio.h>

int main() {
    float calificacion1 = 8.5;
    float calificacion2 = 9.0;
    float calificacion3 = 7.5;
    float promedio;
    promedio = (calificacion1 + calificacion2 + calificacion3) / 3;
    printf("El promedio es: %.2f\n", promedio);
    if (promedio >= 6.0) {
        printf("El alumno aprobo\n");
    } else {
        printf("El alumno no aprobo\n");
    }
    return 0;
}
```

Generado: `salida_python/ejemplo2.py`.

```python
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
```

## Programa 3

Original: `ejemplos/ejemplo3.c`.

```c
#include <stdio.h>

int main() {
    int numero = -3;
    if (numero > 0) {
        printf("El numero es positivo\n");
    } else if (numero < 0) {
        printf("El numero es negativo\n");
    } else {
        printf("El numero es cero\n");
    }
    return 0;
}
```

Generado: `salida_python/ejemplo3.py`.

```python
def main():
    numero = -3
    if numero > 0:
        print("El numero es positivo\n", end="")
    elif numero < 0:
        print("El numero es negativo\n", end="")
    else:
        print("El numero es cero\n", end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

## Ejecución y comparación real

Se validaron los tres archivos con `ast.parse`, sin crear `__pycache__`.
Los originales se compilaron directamente desde `ejemplos/` mediante GCC,
con `-std=c11 -Wall -Wextra`. Los ejecutables estuvieron en un directorio
temporal externo al proyecto y se eliminaron al terminar. Las tres
compilaciones finalizaron con código 0 y sin diagnósticos.

Se ejecutaron los tres programas C y estos comandos Python:

```bash
python -B salida_python/ejemplo1.py
python -B salida_python/ejemplo2.py
python -B salida_python/ejemplo3.py
```

| Programa | Resultado impreso en ambos lenguajes | Código C | Código Python | Sintaxis Python | Comparación |
|---|---|---:|---:|---|---|
| ejemplo1 | Suma 13; resta 3; multiplicación 40 | 0 | 0 | Válida | Bytes idénticos |
| ejemplo2 | Promedio 8.33; el alumno aprobó | 0 | 0 | Válida | Bytes idénticos |
| ejemplo3 | El número es negativo | 0 | 0 | Válida | Bytes idénticos |

Salidas exactas, mostradas como texto:

`ejemplo1`:

```text
La suma es: 13
La resta es: 3
La multiplicacion es: 40
```

`ejemplo2`:

```text
El promedio es: 8.33
El alumno aprobo
```

`ejemplo3`:

```text
El numero es negativo
```

La igualdad se comprobó directamente sobre los bytes capturados, sin
normalizar finales de línea. En este entorno, cada línea termina en CRLF
(`0D 0A`), incluido el salto final, tanto en C como en Python. No hubo salida
de error. La precisión `%.2f`, los textos, la selección de las ramas y los
códigos de terminación coincidieron para estas entradas originales.

Los nombres de variables se conservaron. Solo el segundo programa necesita
`_c_f32`, para representar las conversiones y operaciones de C `float`.
No se sustituyeron las expresiones por resultados precalculados.

## Regresiones repetidas

```bash
python -B pruebas/probar_etapa3.py
python -B pruebas/probar_etapa4.py
```

| Comprobación | Resultado obtenido | Código del ejecutor |
|---|---|---:|
| Batería de etapa 3 | 33/33 aprobadas | 0 |
| Batería de etapa 4 | 68/68 aprobadas | 0 |
| Total de regresiones repetidas | 101/101 aprobadas | 0 |
| Comparaciones de los tres archivos definitivos | 3/3 aprobadas | 0 |
| SHA-256 de archivos protegidos desde el inicio | 35/35 intactos | No aplica |

La batería de etapa 4 volvió a comprobar, además, las 37 comparaciones entre
C y Python de sus programas válidos. No se cambió ninguna prueba anterior.

## Huellas de los archivos Python definitivos

| Archivo | SHA-256 |
|---|---|
| `salida_python/ejemplo1.py` | `96b7a60296781ddcd8159cd7698e7432b1babe8a35512478bf72dfd5f0bbd2c5` |
| `salida_python/ejemplo2.py` | `1175c3e6997bb4eb2c34d10328aea2b42b0d8217d2968bbbf5de7ace51d9cbd0` |
| `salida_python/ejemplo3.py` | `48f224689158f17025b7875e33a7f771ae4a5c97cd824bcdcef358ba522d5454` |

Se verificó que estas huellas siguieran iguales después de las regresiones.

## Archivos creados y artefactos

Se crearon únicamente estos archivos permanentes en el proyecto:

- `salida_python/ejemplo1.py`.
- `salida_python/ejemplo2.py`.
- `salida_python/ejemplo3.py`.
- `pruebas/ETAPA5.md`, este informe.

La compilación regeneró `compilador.exe`, `src/parser.tab.c`,
`src/parser.tab.h` y `src/lex.yy.c`, que ya estaban excluidos de Git.
Las fuentes del transpilador y `compilar.sh` no se modificaron.

Los manifiestos inicial/final, la comparación con las salidas y los logs
completos de las dos baterías se conservaron fuera del proyecto en:

```text
%TEMP%\ProyectoProgramacionDeSistemas-etapa5-20261113-224301
```

## Estructura actualizada

Se omite el contenido interno de `.git/`:

```text
ProyectoProgramacionDeSistemas/
├── ejemplos/
│   ├── 01 Calculadora/
│   │   ├── calc.l
│   │   └── calc.y
│   ├── 02 Calculadora con Variables/
│   │   ├── calc.l
│   │   └── calc.y
│   ├── 03 Generador de codigo/
│   │   ├── lexer.l
│   │   └── parser.y
│   ├── 04 Generador de codigo 2/
│   │   ├── generador.l
│   │   ├── parser.y
│   │   └── programaprueba.txt
│   ├── 05 Generador de codigo 3/
│   │   ├── generador.l
│   │   ├── parser.y
│   │   └── programa.txt
│   ├── compilar.txt
│   ├── ejemplo1.c
│   ├── ejemplo2.c
│   └── ejemplo3.c
├── pruebas/
│   ├── ETAPA3.md
│   ├── ETAPA4.md
│   ├── ETAPA5.md
│   ├── invalido_cadena_sin_cerrar.c
│   ├── invalido_caracter_entre_printf.c
│   ├── invalido_comentario_sin_cerrar.c
│   ├── invalido_parentesis.c
│   ├── invalido_sin_punto_y_coma.c
│   ├── invalido_texto_entre_cadenas.c
│   ├── probar_etapa3.py
│   ├── probar_etapa4.py
│   ├── valido_cadenas_escapadas.c
│   ├── valido_comentarios.c
│   └── valores_semanticos.c
├── salida_python/
│   ├── ejemplo1.py
│   ├── ejemplo2.py
│   └── ejemplo3.py
├── src/
│   ├── analizador_c.l
│   ├── lex.yy.c
│   ├── parser.tab.c
│   ├── parser.tab.h
│   ├── parser.y
│   ├── traduccion.c
│   └── traduccion.h
├── .gitignore
├── 04 05 PS Construcción de un compilador (1).pdf
├── compilador.exe
├── compilar.sh
├── GRAMATICA.md
├── PRESENTACION.md
└── README.md
```

## Comandos para repetir la comparación con GCC

Desde MSYS2 UCRT64, los ejecutables C pueden mantenerse fuera del proyecto:

```bash
carpeta_c=$(mktemp -d)
gcc -std=c11 -Wall -Wextra ejemplos/ejemplo1.c -o "$carpeta_c/ejemplo1.exe"
gcc -std=c11 -Wall -Wextra ejemplos/ejemplo2.c -o "$carpeta_c/ejemplo2.exe"
gcc -std=c11 -Wall -Wextra ejemplos/ejemplo3.c -o "$carpeta_c/ejemplo3.exe"
"$carpeta_c/ejemplo1.exe"
"$carpeta_c/ejemplo2.exe"
"$carpeta_c/ejemplo3.exe"
python -B salida_python/ejemplo1.py
python -B salida_python/ejemplo2.py
python -B salida_python/ejemplo3.py
```

La invocación local de Python se representa aquí con el nombre disponible
en PATH:

```bash
python -B salida_python/ejemplo1.py
```

## Problemas y limitaciones

No se encontró un defecto que requiriera modificar el transpilador.
Los tres programas originales se tradujeron y ejecutaron correctamente.
Se mantienen las limitaciones del subconjunto y del perfil numérico/printf
registradas en `pruebas/ETAPA4.md`; esta etapa no amplía el lenguaje.

Las ejecuciones de los originales comprueban las ramas que toman sus valores
actuales. Las demás rutas se ejercitaron con los programas adicionales de la
batería de etapa 4, sin modificar los originales.

La etapa 5 termina aquí. No se inició la etapa 6 ni se realizaron commits o push.
