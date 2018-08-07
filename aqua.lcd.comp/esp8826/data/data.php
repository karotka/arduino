<?php

list($keys, ) = array_keys ($_REQUEST);
header('Content-Type: application/json');
date_default_timezone_set('Europe/Prague');

$hour = 0;

if ($keys == 'TMLO') {
    echo json_encode (
        array(
            array ("id" => "time", "value" => date("H:i:s")),
            array ("id" => "date", "value" => date("d.m.Y"))
        )
    );
    die();
}

if ($keys == 'HOLO') {
    echo json_encode (
        array(
            array ("cl" => "time", "value" => date("H:i:s")),
            array ("cl" => "date", "value" => date("d.m.Y")),
            array ("cl" => "temp1", "value" => "23.0 C"),
            array ("cl" => "mode", "value" => "MODE: AUTO (OFF)"),
            array ("cl" => "co2", "value" => "CO2: OFF &nbsp; TB: 26.8C"),
            array ("cl" => "temp2", "value" => "T1: 0.0C T2: 0.0C")
        )
    );
    die();
}

if ($keys == 'SELO') {
    echo json_encode (
        array(
            array ("cl" => "time", "value" => date("H:i:s"))
        )
    );
    die();
}

if ($keys == 'CLLO') {
    echo json_encode (array(
            array ("id" => "cool", "value" => 50),
            array ("id" => "warm", "value" => 60),
            array ("id" => "yellow", "value" => 70),
            array ("id" => "red", "value" => 80),
            array ("id" => "green", "value" => 90),
            array ("id" => "blue", "value" => 100))
    );
    die();
}

if ($keys == 'TILO') {
    echo json_encode (
        array(
          "span" => array(
            array ("id" => "t1", "value" => "06:50m"),
            array ("id" => "t2", "value" => "07:00m"),
            array ("id" => "t3", "value" => "14:55m"),
            array ("id" => "t4", "value" => "06:50m"),
            array ("id" => "t5", "value" => "13:44m"),
            array ("id" => "t6", "value" => "15:50m"),
            array ("id" => "t7", "value" => "16:50m"),
            array ("id" => "t8", "value" => "26:50m")),
          "btn" => array(
            array ("id" => "T1MO", "value" => "N2"),
            array ("id" => "T2MO", "value" => "N2"),
            array ("id" => "T3MO", "value" => "N2"),
            array ("id" => "T4MO", "value" => "N2"),
            array ("id" => "T5MO", "value" => "N2"),
            array ("id" => "T6MO", "value" => "N2"),
            array ("id" => "T7MO", "value" => "N2"),
            array ("id" => "T8MO", "value" => "N2"),
          )
        )
    );
    die();
}

if ($keys == 'T8MO') {
    echo json_encode (
        array (
            "id" => strtolower(substr($keys, 0, 2)),
            "key" => $keys,
            "value" => "N1"
        )
    );
    die();
}

echo json_encode(
    array (
        "id" => strtolower(substr($keys, 0, 2)),
        "hour" => 11,
        "min" => 30) );

?>
