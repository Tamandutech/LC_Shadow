# Script de build do PlatformIO (extra_scripts).
#
# O PlatformIO usa por padrão o "arm-none-eabi-gcc-ar" e o
# "arm-none-eabi-gcc-ranlib" (wrappers pensados pra LTO) pra montar as
# bibliotecas .a. Como o projeto NÃO usa LTO, dá pra usar as versões diretas
# (arm-none-eabi-ar / arm-none-eabi-ranlib), que são outros executáveis do
# mesmo toolchain.
#
# Se algum dia ligar -flto no platformio.ini, remova este script.
Import("env")

env.Replace(
    AR="arm-none-eabi-ar",
    RANLIB="arm-none-eabi-ranlib",
)
