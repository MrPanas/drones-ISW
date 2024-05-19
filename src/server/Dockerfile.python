FROM python:3.9-slim

WORKDIR /app


COPY requirements.txt requirements.txt

# Install the dependencies
RUN pip install --no-cache-dir -r requirements.txt


COPY server.py .

# Esponi la porta su cui gira l'applicazione Flask
EXPOSE 3000

# Definisci il comando per eseguire l'applicazione
CMD ["python", "server.py"]