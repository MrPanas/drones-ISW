import psycopg2
from psycopg2 import Error
import matplotlib.pyplot as plt
import numpy as np
import datetime


def get_coverages_with_timestamp(cc_id):
    try:
        # Creazione di un cursore per eseguire le query
        cursor = connection.cursor()

        # Trova l'ultima sessione avviata per il cc_id fornito
        session_query = """
            SELECT id, start_time, end_time
            FROM session 
            WHERE cc_id = %s 
            ORDER BY start_time DESC 
            LIMIT 1;
        """
        cursor.execute(session_query, (cc_id,))
        session_result = cursor.fetchone()

        # Se non ci sono sessioni per il cc_id fornito, restituisci None
        if not session_result:
            print(f"No sessions found for Control Center ID: {cc_id}")
            return None

        last_session_id, start_time, end_time = session_result

        # Query per ottenere coverage e time per l'ultima sessione
        if end_time is None:
            report_query = """
                SELECT coverage, time 
                FROM report 
                WHERE cc_id = %s
                AND time >= %s
                ORDER BY time;
            """
            cursor.execute(report_query, (cc_id, start_time))
        else:
            report_query = """
                SELECT coverage, time 
                FROM report 
                WHERE cc_id = %s
                AND time >= %s
                AND time <= %s
                ORDER BY time;
            """
            cursor.execute(report_query, (cc_id, start_time, end_time))

        coverages_with_timestamp = cursor.fetchall()

        return coverages_with_timestamp
    except (Exception, Error) as error:
        print("Error while connecting to PostgreSQL:", error)
        return None

    finally:
        if connection:
            cursor.close()
            connection.close()
            print("PostgreSQL connection is closed")

def get_paths():
    try:
        # Creazione di un cursore per eseguire le query
        cursor = connection.cursor()

        # Esempio di query per trovare i path
        query = "SELECT * FROM paths;"
        cursor.execute(query)
        paths = cursor.fetchall()

        return paths

    except (Exception, Error) as error:
        print("Error while connecting to PostgreSQL:", error)
        return None

    finally:
        if connection:
            cursor.close()
            connection.close()
            print("PostgreSQL connection is closed")


def plot_coverage_vs_time(coverages, minutes):
    # Estrai i dati
    timestamps = [row[1] for row in coverages]
    coverages_values = [row[0] for row in coverages]

    # Trova il timestamp più basso e calcola i tempi in minuti a partire da quel timestamp
    min_timestamp = min(timestamps)
    minutes_from_start = [(ts - min_timestamp).total_seconds() / 60 for ts in timestamps]

    # Disegna il grafico
    plt.plot(minutes_from_start, coverages_values)
    plt.xlabel('Tempo (minuti)')
    plt.ylabel('Coverage')
    plt.title(f'Coverage vs Tempo cc <{cc_id}>')

    # Imposta i limiti degli assi
    plt.xlim(0, minutes)  # Limite dell'asse x fino a 3 ore (180 minuti)
    plt.ylim(0, 1.1)    # Limite dell'asse y fino a 1



if __name__ == "__main__":
    # Connessione al database PostgreSQL
    connection = psycopg2.connect(
        user="postgres",
        password="postgres",
        host="localhost",
        port="5432",
        database="postgres"
    )
    cc_id = 1
    coverages = get_coverages_with_timestamp(cc_id)
    if coverages:
        plt.subplot(2, 1, 1)  # Primo subplot
        plot_coverage_vs_time(coverages, 230)  # Mostra il grafico con il limite a 180 minuti
        plt.subplot(2, 1, 2)  # Secondo subplot
        plot_coverage_vs_time(coverages, 30)  # Mostra il grafico con il limite a 30 minuti
        plt.tight_layout()  # Aggiusta il layout dei subplot per evitare sovrapposizioni
        plt.show()

# TODO: plottare i dati "teorici" nella situazione ottimale