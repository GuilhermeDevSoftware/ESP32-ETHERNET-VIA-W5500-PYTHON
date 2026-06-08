import sqlite3


ARQUIVO_BANCO = "monitoramento_esp32.db"


def mostrar_leituras(conexao):
    cursor = conexao.execute("""
        SELECT
            id,
            temperatura,
            umidade,
            conexao,
            data_hora
        FROM leituras_dht11
        ORDER BY id DESC
        LIMIT 20
    """)

    print("\nÚltimas leituras do DHT11:\n")

    for linha in cursor.fetchall():
        print(
            f"ID: {linha[0]} | "
            f"Temperatura: {linha[1]} °C | "
            f"Umidade: {linha[2]} % | "
            f"Conexão: {linha[3]} | "
            f"Data: {linha[4]}"
        )


def mostrar_eventos_led(conexao):
    cursor = conexao.execute("""
        SELECT
            id,
            estado,
            data_hora
        FROM eventos_led
        ORDER BY id DESC
        LIMIT 20
    """)

    print("\nÚltimos eventos do LED:\n")

    for linha in cursor.fetchall():
        print(
            f"ID: {linha[0]} | "
            f"Estado: {linha[1]} | "
            f"Data: {linha[2]}"
        )


def main():
    conexao = sqlite3.connect(ARQUIVO_BANCO)

    try:
        mostrar_leituras(conexao)
        mostrar_eventos_led(conexao)
    finally:
        conexao.close()


if __name__ == "__main__":
    main()