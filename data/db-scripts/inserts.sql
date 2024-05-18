INSERT INTO path(path) VALUES ('EEEE'),('EEEE'),('EEEE'),('EEEE'),('EEEE');


INSERT INTO drone_path(path_id, drone_id, start_time) VALUES (1,1, now());
INSERT INTO drone_path(path_id, drone_id, start_time) VALUES (2,2, now());
INSERT INTO drone_path(path_id, drone_id, start_time) VALUES (3,3, now());
INSERT INTO drone_path(path_id, drone_id, start_time) VALUES (4,4, now());
INSERT INTO drone_path(path_id, drone_id, start_time) VALUES (5,5, now());

INSERT INTO drone_log(drone_id, time, path_id, status) VALUES (1, now(), 1, 'SCANNING'),(2, now(), 2, 'SCANNING'),
                                                              (3, now(), 3, 'SCANNING'),(4, now(), 4, 'SCANNING'),
                                                              (5, now(), 5, 'SCANNING');

INSERT INTO drone_log(drone_id, time, path_id, status) VALUES (1, now(), 1, 'CHARGING'),(2, now(), 2, 'CHARGING'),
                                                              (3, now(), 3, 'CHARGING'),(4, now(), 4, 'CHARGING'),
                                                              (5, now(), 5, 'CHARGING');

INSERT INTO drone_log(drone_id, time, path_id, status) VALUES (1, now(), 1, 'SCANNING'),(2, now(), 2, 'SCANNING'),
                                                              (3, now(), 3, 'SCANNING'),(4, now(), 4, 'SCANNING'),
                                                              (5, now(), 5, 'SCANNING');
