CREATE TYPE status AS ENUM ('SCANNING', 'CHARGING', 'MOVING', 'IDLE');

CREATE TABLE IF NOT EXISTS drone (
    drone_id SERIAL PRIMARY KEY,  
    battery float CHECK (battery >= 0 AND battery <= 1),  
    status status
);

CREATE TABLE IF NOT EXISTS path (
    path_id SERIAL PRIMARY KEY,  
    path text
);

CREATE TABLE IF NOT EXISTS cc_log (
    cc_log_id int PRIMARY KEY,
    drone_id  int,
    path_id   int, 
    istante   TIMESTAMP,
    message   text NOT NULL,
    FOREIGN KEY (drone_id) REFERENCES drone (drone_id),
    FOREIGN KEY (path_id) REFERENCES path (path_id)
);

CREATE TABLE IF NOT EXISTS area_log (
    area_log_id int PRIMARY KEY,
    time TIMESTAMP,
    coord_x int,
    coord_y int,
    time_unchecked bigint
);


CREATE TABLE IF NOT EXISTS drone_path (
    path_id int,
    drone_id int,
    start_time TIMESTAMP,  
    PRIMARY KEY (path_id, drone_id),
    FOREIGN KEY (path_id) REFERENCES path (path_id),
    FOREIGN KEY (drone_id) REFERENCES drone (drone_id)
);

CREATE TABLE IF NOT EXISTS drone_log (
    log_id SERIAL PRIMARY KEY,  
    drone_id int,
    time TIMESTAMP NOT NULL,  
    path_id int,
    status status,
    FOREIGN KEY (path_id) REFERENCES path (path_id),
    FOREIGN KEY (drone_id) REFERENCES drone (drone_id)
);
