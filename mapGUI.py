import pygame
import sys
import requests
import numpy as np
import random

DEBUG = True

# Inizializza pygame
pygame.init()

# Dimensioni della matrice
matrix_size = 300

# Dimensioni iniziali della finestra
window_size = 900

# Crea una finestra di pygame
window = pygame.display.set_mode((window_size, window_size))

# Titolo della finestra
pygame.display.set_caption('Visualizzazione della Matrice 300x300 con Zoom e Scroll')

# Funzione per ottenere la matrice dal server
def fetch_matrix():
    url = 'http://127.0.0.1:5000/get_matrix'  # TODO: Modifica l'URL con quello del tuo server
    response = requests.get(url) # TODO: dovrebbe restituire una matrice con numeri da 0 a 5
    response.raise_for_status()  # Verifica eventuali errori nella risposta
    return response.json()

def fetch_drones():
    url = 'http://127.0.0.1:5000/get_drones'  # TODO: Modifica l'URL con quello del tuo server
    response = requests.get(url) # TODO: dovrebbe restituire un array di coordinate
    response.raise_for_status()  # Verifica eventuali errori nella risposta
    return response.json()



if DEBUG:
    # Crea una matrice 300x300 di numeri float casuali tra 0 e 5
    matrix = np.random.uniform(0, 5, (matrix_size, matrix_size))

    num_drones = 226
    # Crea un array di coordinate casuali
    drones_coordinates = np.random.randint(0, matrix_size, size=(num_drones, 2))

else:
    # Ottieni la matrice e le coordinate dei droni dal server
    matrix = fetch_matrix()
    drones_coordinates = fetch_drones()

# Normalizza la matrice per avere valori compresi tra 0 e 1
normalized_matrix = matrix / 5.0

for coord in drones_coordinates:
    normalized_matrix[coord[0], coord[1]] = 6  # Imposta il colore nero

def get_color(value):
    if value == 6:
        return 0, 0, 0  # Colore nero
    # Interpolazione lineare tra verde e rosso
    green = min(max(1 - value, 0), 1) * 255  # Aumenta da 0 a 1
    red = min(max(value, 0), 1) * 255  # Aumenta da 0 a 1
    return int(red), int(green), 0


# Variabili di zoom e scroll
zoom = 1.0  # Imposta un valore iniziale più grande per il zoom
offset_x = 0
offset_y = 0

# Velocità di spostamento
scroll_speed = 20
zoom_speed = 0.1

# Loop principale
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    keys = pygame.key.get_pressed()
    if keys[pygame.K_PLUS] or keys[pygame.K_EQUALS]:  # Tasto "+" per zoom in
        zoom += zoom_speed
    if keys[pygame.K_MINUS]:  # Tasto "-" per zoom out
        zoom -= zoom_speed
        if zoom < 0.1:
            zoom = 0.1

    if keys[pygame.K_LEFT]:
        offset_x += scroll_speed
    if keys[pygame.K_RIGHT]:
        offset_x -= scroll_speed
    if keys[pygame.K_UP]:
        offset_y += scroll_speed
    if keys[pygame.K_DOWN]:
        offset_y -= scroll_speed

    # Riempie lo sfondo di nero
    window.fill((0, 0, 0))

    # Disegna la matrice
    cell_size = int(window_size / matrix_size * zoom)
    for y in range(matrix_size):
        for x in range(matrix_size):
            value = normalized_matrix[y, x]
            color = get_color(value)
            rect = pygame.Rect(x * cell_size + offset_x, y * cell_size + offset_y, cell_size, cell_size)
            pygame.draw.rect(window, color, rect)

    # Aggiorna la finestra
    pygame.display.flip()

# Chiude pygame
pygame.quit()
sys.exit()
