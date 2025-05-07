
# 📊 **﻿SemaforoInteligente-RTOS**

Projeto individual para exercício do controle de Tasks com o Free-RTOS, utilização de LEDs PWM, Matriz 5x5, Display SSD1306,buzzer e rotinas de interrupções com deboucing para botões, utilizando a placa **BitDogLab** na linguagem **C**. Desenvolvido para a residência de **Sistemas Embarcados**, **TIC 37 - Embarcatech**.

---

## 🔎 **Objetivos**

Um projeto que consiste na administração de threads(Tasks) com o Free-RTOS, LEDs, display e botões para formar um simulador de semáforo com 2 Modos: Noturno e Diurno, com métodos e processos diferentes, o modo Diurno altera em uma sequência de verde -> amarelo -> vermelho, cada um com um tempo e um simbolo diferente na matriz de LED e um barulho diferente no buzzer, além de uma luz correspondente nos LEDs PWM e um feedback visual pelo Display. O Modo Noturno faz com que apenas o LED amarelo pisque lentamente com espaço de 2 segundos entre os barulhos do buzzer, simbolizando um semáforo de madrugada.

---

## 📊 **Video do Projeto:** https://drive.google.com/file/d/1G63GOcDMm1tIHZwsFUQiteUj-pUA8d1L/view?usp=drive_link

## 🛠️ **Tecnologias Utilizadas**

- **Linguagem de Programação:** C / CMake
- **Placas Microcontroladoras:**
  - BitDogLab
  - Pico W
- **Sistema Operacional:** Free-RTOS
---

## 📖 **Como Utilizar**

- Aperte o botão A para alterar entre o modo Diurno e Noturno

---

## 📊 **Funcionalidades Demonstradas**

- Controle de Concorrência pelas Threads(Tasks)
- Controle de Matriz de led 5x5
- Controle do Display SSD1306
- Controle de LED PWM
- Controle do buzzer
- Manipulação de botões e deboucing


