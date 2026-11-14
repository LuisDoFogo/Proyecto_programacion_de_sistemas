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
