from flask import Flask, request, jsonify
import csv
import os

app = Flask(__name__)

# Definisci il percorso e il metodo per gestire le richieste POST
@app.route('/endpoint', methods=['POST'])
def handle_post_request():
    # Verifica se la richiesta è una POST
    if request.method == 'POST':
        # Ottieni i dati JSON dalla richiesta
        data = request.json

        # Estrai le chiavi 'area' e 'cc-id' dal JSON ricevuto
        area = data.get('area')
        cc_id = data.get('cc-id')

        # Verifica che entrambe le chiavi siano presenti
        if area is not None and cc_id is not None:
            # Prepara una risposta di successo
            response = {
                "message": "Dati ricevuti correttamente",
            }

            csv_filename = os.path.join("areas", f'area_1_{cc_id}.csv')

            with open(csv_filename, 'w', newline='') as csv_file:
                csv_writer = csv.writer(csv_file)

                for row in area:
                    csv_writer.writerow(row)

            return jsonify(response), 200



        else:
            # Se mancano una o entrambe le chiavi, ritorna un errore
            return jsonify({"error": "Chiavi 'area' e 'cc-id' mancanti"}), 400
    else:
        # Se il metodo della richiesta non è POST, ritorna un errore
        return jsonify({"error": "Metodo non consentito"}), 405

if __name__ == '__main__':
    # Avvia il server Flask
    app.run(debug=True)
