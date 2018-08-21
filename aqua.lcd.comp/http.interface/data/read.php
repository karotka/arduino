<?php

header('Content-Type: application/json');
date_default_timezone_set('Europe/Prague');

function get(&$var, $default=null) {
    return isset($var) ? $var : $default;
}

$params = $_GET["p"];
$value  = get($_GET["v"], "");

if ($params == "LISA") {
    echo json_encode (
        array("id" => $value)
    );
}

if ($params == "HOLO") {
    //header("HTTP/1.0 204 No Content");
    //die();

    echo json_encode (
        array(
            array ("id" => "dt",   "v" => date("l-d-M Y-H:i:s")),
            array ("id" => "temp",  "v" => "23.6-22.3-34.9"),
            array ("id" => "co2",   "v" => "OFF"),
            array ("id" => "light", "v" => "Automat Off-10-80-40-30-20-80"),
            array ("id" => "feed", "v" => "7:00-19:00"),
            array ("id" => "ser", "v" => "neco")
        )
    );
}

if ($params == 'LILO') {
    echo json_encode (
        array(
            array ("id" => "cool", "v" => 10),
            array ("id" => "warm", "v" => 20),
            array ("id" => "yellow", "v" => 30),
            array ("id" => "red", "v" => 40),
            array ("id" => "green", "v" => 50),
            array ("id" => "blue", "v" => 60),

            array ("id" => "B0", "v" => 0),
            array ("id" => "B1", "v" => 0),
            array ("id" => "B2", "v" => 0),
            array ("id" => "B3", "v" => 1),
            array ("id" => "B4", "v" => 0),
            array ("id" => "B5", "v" => 0)

        )
    );
}

if ($params == "TMLO") {
    echo json_encode (
        array(
            array ("id" => "h",  "v" => date("H")),
            array ("id" => "m",  "v" => date("i")),
            array ("id" => "s",  "v" => date("s"))
        )
    );
}
if ($params == "DALO") {
    echo json_encode (
        array(
            array ("id" => "d",  "v" => date("d")),
            array ("id" => "m",  "v" => date("m")),
            array ("id" => "y",  "v" => date("Y"))
        )
    );
}
if ($params == "FDLO") {
    echo json_encode (
        array(
            array ("id" => "h1",  "v" => "7"),
            array ("id" => "m1",  "v" => "00"),
            array ("id" => "h2",  "v" => "10"),
            array ("id" => "m2",  "v" => "30"),
            array ("id" => "h3",  "v" => "15"),
            array ("id" => "m3",  "v" => "45"),
            array ("id" => "h4",  "v" => "19"),
            array ("id" => "m4",  "v" => "50"),
        )
    );
}
if ($params == "TILO") {
    echo json_encode (
        array("dt" => date("d-m-Y-H-i-s"))
    );
}
if ($params == "TISA") {
    echo json_encode (
        array("dt" => date("d-m-Y-H-i-s"))
    );
}
if ($params == 'COLO') {
    echo json_encode (
        array(
            array ("id" => "B0", "v" => "D1"),
            array ("id" => "B1", "v" => "D2"),
            array ("id" => "B2", "v" => "OF"),
            array ("id" => "B3", "v" => "N1"),
            array ("id" => "B4", "v" => "N2"),
            array ("id" => "B5", "v" => "AU")
        )
    );
}

?>
