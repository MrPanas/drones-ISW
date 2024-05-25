import pygame
import sys
import requests
import numpy as np
import time
import datetime
import random

DEBUG = False

TIME_ACCELERATION = 1

# Inizializza pygame
pygame.init()

# Dimensioni della matrice
matrix_size = 300

# Dimensioni iniziali della finestra
window_size = 600
legend_height = 80

# Crea una finestra di pygame
window = pygame.display.set_mode((window_size, window_size + legend_height))

# Titolo della finestra
pygame.display.set_caption('Visualizzazione della Matrice 300x300')

# Funzione per ottenere la matrice dal server
def fetch_matrix():
    url = 'http://127.0.0.1:3000/report/1'  # Modifica l'URL con quello del tuo server
    response = requests.get(url)  # dovrebbe restituire una matrice con numeri da 0 a 5
    response.raise_for_status()  # Verifica eventuali errori nella risposta

    matrix = response.json()
    current_time = datetime.datetime.now(tz=datetime.timezone.utc)

    # Calcola i minuti trascorsi per ogni timestamp nella matrice
    for i in range(len(matrix)):
        for j in range(len(matrix[i])):
            time_in_millis = int(matrix[i][j])
            # print(f"timestamp_ms: {time_in_millis}")
            dt = datetime.datetime.fromtimestamp(time_in_millis / 1000.0, tz=datetime.timezone.utc)

            # Calcola la differenza in minuti
            time_difference = current_time - dt
            minutes_passed = time_difference.total_seconds() / 60.0
            # print(f"minutes_passed: {minutes_passed}")
            matrix[i][j] = minutes_passed

    return np.array(matrix)




if DEBUG:
    # Crea una matrice 300x300 di numeri float casuali tra 0 e 5
    matrix = np.random.uniform(0, 5, (matrix_size, matrix_size))

    num_drones = 226
    # Crea un array di coordinate casuali
    drones_coordinates = np.random.randint(0, matrix_size, size=(num_drones, 2))

else:
    # Ottieni la matrice e le coordinate dei droni dal server
    matrix = fetch_matrix()


def get_color(value):
    if value <= 1 * TIME_ACCELERATION:
        return 0, 255, 0  # Verde
    elif value > 1 * TIME_ACCELERATION and value <= 2 * TIME_ACCELERATION:
        return 0, 128, 0  # Verde scuro
    elif value > 2 * TIME_ACCELERATION and value <= 3 * TIME_ACCELERATION:
        return 255, 255, 0  # Giallo
    elif value > 3 * TIME_ACCELERATION and value <= 4 * TIME_ACCELERATION:
        return 255, 165, 0  # Arancione
    elif value > 4 * TIME_ACCELERATION and value <= 5 * TIME_ACCELERATION:
        return 255, 0, 0  # Rosso arancio
    else:
        return 0, 0, 0  # Rosso



def draw_legend(surface):
    font = pygame.font.SysFont(None, 24)
    legend_items = [
        ("< 1", (0, 255, 0)),    # Verde
        ("1 - 2", (0, 128, 0)),  # Verde scuro
        ("2 - 3", (255, 255, 0)),# Giallo
        ("3 - 4", (255, 165, 0)),# Arancione
        ("4 - 5", (255, 0, 0)),  # Rosso arancio
        ("> 5", (0, 0, 0)),      # Nero
    ]

    x = 10
    y = window_size + 10


    # Aggiungi uno sfondo grigio per la leggenda
    pygame.draw.rect(surface, (100, 100, 100), (0, window_size, window_size, legend_height))

    # Calcolo della percentuale
    total_elements = matrix_size * matrix_size
    elements_less_or_equal_5 = np.count_nonzero(matrix <= 5 * TIME_ACCELERATION)
    percentage = (elements_less_or_equal_5 / total_elements) * 100
    percentage_text = f"Coverage: {percentage:.2f}%"



    # Testo della percentuale
    percentage_surface = font.render(percentage_text, True, (255, 255, 255))
    percentage_width, _ = font.size(percentage_text)

    # Disegna la percentuale sulla destra della leggenda
    surface.blit(percentage_surface, (window_size - percentage_width - 10, y))


    # Testo esplicativo
    explanation_text = "Minuti trascorsi dall'ultima scansione:"
    text_surface = font.render(explanation_text, True, (255, 255, 255))
    surface.blit(text_surface, (x, y))

    y += 30  # Spazio tra la spiegazione e la leggenda

    for label, color in legend_items:
        pygame.draw.rect(surface, color, (x, y, 20, 20))
        text = font.render(label, True, (255, 255, 255))
        surface.blit(text, (x + 30, y))
        x += 90  # Spazio tra gli elementi della leggenda



# Variabili di zoom e scroll
zoom = 1.0  # Imposta un valore iniziale più grande per il zoom
offset_x = 0
offset_y = 0

# Velocità di spostamento
scroll_speed = 20
zoom_speed = 0.1

# Imposta un timer per l'aggiornamento della matrice ogni secondo
UPDATE_EVENT = pygame.USEREVENT + 1
pygame.time.set_timer(UPDATE_EVENT, 1000)  # 1000 millisecondi = 1 secondo

# Loop principale
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == UPDATE_EVENT:
            # Aggiorna la matrice e le coordinate dei droni
            matrix = fetch_matrix()
            # drones_coordinates = fetch_drones()
            # normalized_matrix = matrix / np.max(matrix)

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
            value = matrix[y, x]
            color = get_color(value)
            rect = pygame.Rect(x * cell_size + offset_x, y * cell_size + offset_y, cell_size, cell_size)
            pygame.draw.rect(window, color, rect)

    # Disegna il contorno della matrice
    matrix_rect = pygame.Rect(offset_x, offset_y, matrix_size * cell_size, matrix_size * cell_size)
    pygame.draw.rect(window, (255, 255, 255), matrix_rect, 2)  # Contorno bianco con spessore 2

    draw_legend(window)

    # Aggiorna la finestra
    pygame.display.flip()

# Chiude pygame
pygame.quit()
sys.exit()
