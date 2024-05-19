DO $$
   BEGIN
    -- Verifica se il tipo ENUM esiste già
    IF NOT EXISTS (
        SELECT 1
        FROM pg_type
        WHERE typname = 'status'
          AND typtype = 'e'
    ) THEN
        -- Crea il tipo ENUM solo se non esiste già
        CREATE TYPE status AS ENUM ('READY', 'CHARGING', 'WORKING');
    END IF;
END $$;

DO $$
    BEGIN
        -- Verifica se il tipo ENUM esiste già
        IF NOT EXISTS (
            SELECT 1
            FROM pg_type
            WHERE typname = 'status'
              AND typtype = 'e'
        ) THEN
            -- Crea il tipo ENUM solo se non esiste già
            CREATE TYPE requirement AS ENUM ('NUM_DRONES', 'CC_OVERLOAD', 'DRONE_COME_BACK', 'AREA_COVERAGE');
        END IF;
    END $$;

CREATE TABLE IF NOT EXISTS area (
    id SERIAL PRIMARY KEY,
    width int,
    height int,
    point_expiration_time int
);

CREATE TABLE IF NOT EXISTS control_center (
    id int PRIMARY KEY,
    area_id int,
    FOREIGN KEY (area_id) REFERENCES area (id)
);

CREATE TABLE IF NOT EXISTS drone (
    id int PRIMARY KEY,
    battery float CHECK (battery >= 0 AND battery <= 1),
    status status,
    cc_id int,
    FOREIGN KEY (cc_id) REFERENCES control_center (id)
);

CREATE TABLE IF NOT EXISTS path (
    id int PRIMARY KEY,
    path text
);

CREATE TABLE IF NOT EXISTS area_log (
    area_id int,
    time TIMESTAMP,
    percentage float,
    FOREIGN KEY (area_id) REFERENCES area (id)
);

CREATE TABLE IF NOT EXISTS drone_log (
    log_id SERIAL PRIMARY KEY,
    drone_id int,
    cc_id int,
    time TIMESTAMP NOT NULL,
    path_id int,
    status status,
    FOREIGN KEY (path_id) REFERENCES path (id),
    FOREIGN KEY (drone_id) REFERENCES drone (id),
    FOREIGN KEY (cc_id) REFERENCES control_center (id)
);

CREATE TABLE IF NOT EXISTS report_image (
    image_id  SERIAL PRIMARY KEY,
    cc_id     int,
    image_url varchar(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS monitor_failure (
    failure_id SERIAL PRIMARY KEY,
    cc_id     int,
    time      TIMESTAMP NOT NULL,
    FOREIGN KEY (cc_id) REFERENCES control_center (id)
);


