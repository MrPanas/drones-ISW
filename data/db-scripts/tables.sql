CREATE TYPE status AS ENUM ('SCANNING', 'CHARGING', 'MOVING', 'IDLE');

CREATE TABLE IF NOT EXISTS area (
    width int,
    height int,
    PRIMARY KEY (width, height)
);

CREATE TABLE IF NOT EXISTS control_center (
    cc_id SERIAL PRIMARY KEY,
    area_width int,
    area_height int,
    FOREIGN KEY (area_width, area_height) REFERENCES area (width, height)
);

CREATE TABLE IF NOT EXISTS drone (
     drone_id SERIAL PRIMARY KEY,
     battery float CHECK (battery >= 0 AND battery <= 1),
    status status,
    cc_id int,
    FOREIGN KEY (cc_id) REFERENCES control_center (cc_id)
);

CREATE TABLE IF NOT EXISTS path (
    path text PRIMARY KEY
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
    area_width int,
    area_height int,
    time TIMESTAMP,
    coord_x int,
    coord_y int,
    time_unchecked bigint,
    FOREIGN KEY (area_width, area_height) REFERENCES area (width, height)
);

CREATE TABLE IF NOT EXISTS drone_log (
    log_id SERIAL PRIMARY KEY,
    drone_id int,
    cc_id int,
    time TIMESTAMP NOT NULL,
    path_id int,
    status status,
    FOREIGN KEY (path_id) REFERENCES path (path_id),
    FOREIGN KEY (drone_id) REFERENCES drone (drone_id),
    FOREIGN KEY (cc_id) REFERENCES control_center (cc_id)
);