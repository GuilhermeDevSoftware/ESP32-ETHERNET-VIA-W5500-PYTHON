import sqlite3
import time
from datetime import datetime

import requests


# ==================================================
# CONFIGURAÇÕES
# ==================================================

IP_ESP32 = "192.168.100.102"

URL_STATUS = f"http://{IP_ESP32}/status"

INTERVALO_SEGUNDOS = 10
TEMPO_LIMITE_REQUISICAO = 4

ARQUIVO_BANCO = "monitoramento_esp32.db"


# ==================================================
# BANCO DE DADOS
# ==================================================

def conectar_banco():
    conexao = sqlite3.connect(ARQUIVO_BANCO)

    conexao.execute("""
        CREATE TABLE IF NOT EXISTS leituras_dht11 (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            temperatura REAL,
            umidade REAL,
            conexao TEXT,
            data_hora TEXT NOT NULL
        )
    """)

    conexao.execute("""
        CREATE TABLE IF NOT EXISTS eventos_led (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            estado TEXT NOT NULL,
            data_hora TEXT NOT NULL
        )
    """)

    conexao.commit()

    return conexao


# ==================================================
# CONSULTAR ESP32
# ==================================================

def consultar_status():
    resposta = requests.get(
        URL_STATUS,
        timeout=TEMPO_LIMITE_REQUISICAO,
        headers={
            "Connection": "close"
        }
    )

    try:
        resposta.raise_for_status()
        return resposta.json()
    finally:
        resposta.close()


# ==================================================
# SALVAR LEITURA DO DHT11
# ==================================================

def salvar_leitura_dht11(
    conexao,
    temperatura,
    umidade,
    tipo_conexao,
    data_hora
):
    conexao.execute("""
        INSERT INTO leituras_dht11 (
            temperatura,
            umidade,
            conexao,
            data_hora
        )
        VALUES (?, ?, ?, ?)
    """, (
        temperatura,
        umidade,
        tipo_conexao,
        data_hora
    ))

    conexao.commit()


# ==================================================
# SALVAR EVENTO DO LED
# ==================================================

def salvar_evento_led(
    conexao,
    estado,
    data_hora
):
    conexao.execute("""
        INSERT INTO eventos_led (
            estado,
            data_hora
        )
        VALUES (?, ?)
    """, (
        estado,
        data_hora
    ))

    conexao.commit()


# ==================================================
# PROGRAMA PRINCIPAL
# ==================================================

def main():
    conexao = conectar_banco()

    ultimo_estado_led = None

    print("Monitoramento iniciado.")
    print(f"Consultando: {URL_STATUS}")
    print(f"Banco de dados: {ARQUIVO_BANCO}")
    print("Pressione Ctrl+C para encerrar.")
    print()

    try:
        while True:
            try:
                dados = consultar_status()

                temperatura = dados.get("temperatura")
                umidade = dados.get("umidade")
                estado_led = dados.get("led")
                tipo_conexao = dados.get("conexao")

                data_hora = datetime.now().strftime(
                    "%Y-%m-%d %H:%M:%S"
                )

                salvar_leitura_dht11(
                    conexao,
                    temperatura,
                    umidade,
                    tipo_conexao,
                    data_hora
                )

                print(
                    f"[{data_hora}] "
                    f"Temperatura: {temperatura} °C | "
                    f"Umidade: {umidade} % | "
                    f"LED: {estado_led}"
                )

                if estado_led != ultimo_estado_led:
                    salvar_evento_led(
                        conexao,
                        estado_led,
                        data_hora
                    )

                    print(
                        f"Evento do LED salvo: {estado_led}"
                    )

                    ultimo_estado_led = estado_led

            except requests.exceptions.ConnectionError:
                print(
                    "Erro: não foi possível conectar ao ESP32."
                )

            except requests.exceptions.Timeout:
                print(
                    "Erro: o ESP32 demorou para responder."
                )

            except requests.exceptions.HTTPError as erro:
                print(
                    f"Erro HTTP: {erro}"
                )

            except requests.exceptions.JSONDecodeError:
                print(
                    "Erro: resposta JSON inválida."
                )

            except Exception as erro:
                print(
                    f"Erro inesperado: {erro}"
                )

            time.sleep(INTERVALO_SEGUNDOS)

    except KeyboardInterrupt:
        print()
        print("Monitoramento encerrado.")

    finally:
        conexao.close()


if __name__ == "__main__":
    main()