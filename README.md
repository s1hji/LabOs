# Лабораторная работа 1: mycat и mygrep

## Описание
Реализация утилит `mycat` и `mygrep` аналогичных стандартным Unix-утилитам cat и grep.

## Требования
- Утилита `mycat` с поддержкой флагов: `-n`, `-b`, `-E`
- Утилита `mygrep` с поддержкой поиска и туннелирования
- Makefile для сборки

## Сборка
```bash
cd lab1
make

MyCate:

# Базовое использование
./mycat file.txt

# С нумерацией всех строк
./mycat -n file.txt

# С нумерацией непустых строк
./mycat -b file.txt

# С символом $ в конце строк
./mycat -E file.txt

# Комбинация флагов
./mycat -nE file.txt

MyGrep:

# Поиск в файле
./mygrep "pattern" file.txt

# Туннелирование (piping)
./mycat file.txt | ./mygrep "pattern"
ls -l | ./mygrep ".txt"

Автор
Зыков Александр КМБО-05-23  - 5 семестр, Защищенные операционные системы
