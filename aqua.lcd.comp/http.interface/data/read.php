<?php

header('Content-Type: application/json');
date_default_timezone_set('Europe/Prague');
session_start();


if (!isset($_SESSION["second"])) {$_SESSION["second"] = 0;}
if (!isset($_SESSION["minute"])) {$_SESSION["minute"] = 0;}
if (!isset($_SESSION["hour"])) {$_SESSION["hour"] = 0;}
if (!isset($_SESSION["day"])) {$_SESSION["day"] = 0;}
if (!isset($_SESSION["month"])) {$_SESSION["month"] = 0;}
if (!isset($_SESSION["year"])) {$_SESSION["year"] = 0;}

$mytime = mytime();


function mytime() {
    $time = time();

    if ($_SESSION["second"])
        $time = $_SESSION["second"]  * strtotime('+1 second', $time);
    if ($_SESSION["minute"])
        $time = $_SESSION["minute"]  * strtotime('+1 minute', $time);
    if ($_SESSION["hour"])
        $time = $_SESSION["hour"]    * strtotime('+1 hour', $time);
    if ($_SESSION["day"])
        $time = $_SESSION["day"]     * strtotime('+1 day', $time);
    if ($_SESSION["month"])
        $time = $_SESSION["month"]   * strtotime('+1 month', $time);
    if ($_SESSION["year"])
        $time = $_SESSION["year"]    * strtotime('+1 year', $time);
    //echo "<pre>time: ", print_r($_SESSION);
    return time();
}

function get(&$var, $default=null) {
    return isset($var) ? $var : $default;
}

$params = get($_GET["p"]);
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
            array ("id" => "dt",   "v" => date("l-d-M Y-H:i:s", $mytime)),
            array ("id" => "temp", "v" => "23.6-22.3-34.9"),
            array ("id" => "co2",  "v" => "OFF"),
            array ("id" => "light","v" => "Automat Off-10-80-40-30-20-80"),
            array ("id" => "feed", "v" => "7:00-19:00"),
            array ("id" => "ser",  "v" => "neco"),
            array ("p" => "index")
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
            array ("id" => "h",  "v" => date("H"), mytime()),
            array ("id" => "m",  "v" => date("i"), mytime()),
            array ("id" => "s",  "v" => date("s"), mytime())
        )
    );
}
if ($params == "DALO") {
    echo json_encode (
        array(
            array ("id" => "d",  "v" => date("d"), mytime()),
            array ("id" => "m",  "v" => date("m"), mytime()),
            array ("id" => "y",  "v" => date("Y"), mytime())
        )
    );
}
if ($params == "FDLO") {
    echo json_encode (
        array(
            array ("id" => 1, "h" => "7",  "m" => "07"),
            array ("id" => 2, "h" => "10", "m" => "01"),
            array ("id" => 3, "h" => "12", "m" => "02"),
            array ("id" => 4, "h" => "15", "m" => "05")
        )
    );
}

if ($params == "COLO") {
    echo json_encode (
        array(
            array ("id" => "1", "h" => "7", "m" => "07"),
            array ("id" => "2", "h" => "13", "m" => "03"),
            array ("id" => "3", "h" => "15", "m" => "05"),
            array ("id" => "4", "h" => "17", "m" => "07"),
            array ("s" => array(1, 0, 1, 0))
        )
    );
}

if ($params == "TRLO") {
    echo json_encode (
        array(
            array ("id" => "1", "h" => "7", "m" => "07"),
            array ("id" => "2", "h" => "13", "m" => "03"),
            array ("id" => "3", "h" => "15", "m" => "05"),
            array ("id" => "4", "h" => "17", "m" => "07"),
            array ("id" => "5", "h" => "18", "m" => "08"),
            array ("id" => "6", "h" => "19", "m" => "09"),
            array ("id" => "7", "h" => "20", "m" => "10"),
            array ("id" => "8", "h" => "21", "m" => "11"),
            array ("s" => array(1, 1, 1, 1, 0, 0, 0, 0))
        )
    );
}

if ($params == "TILO") {
    echo json_encode (
        array("dt" => date("d-m-Y-H-i-s", $mytime). "-n", "p" => 'time')
    );
}

//echo $params, $value;
if ($params == "TISA") {
    $saved = "n";
    if ($value == "ok") {
        $saved = "y";
    }
    echo json_encode (
        array("dt" => date("d-m-Y-H-i-s", $mytime). "-". $saved)
    );
}

function startsWith($haystack, $needle) {
     $length = strlen($needle);
     return (substr($haystack, 0, $length) === $needle);
}

if ($params == "COSA") {
    if (startsWith($value, "m")) {
        echo json_encode (
            array("dt" => substr($value, 0, 2). "-23")
        );
    }
    if (startsWith($value, "h")) {
        echo json_encode (
            array("dt" => substr($value, 0, 2). "-23")
        );
    }
    if (startsWith($value, "s")) {
        $v = get($_SESSION["co2s"]);
        if ($v == 0) $_SESSION["co2s"] = 1;
        if ($v == 1) $_SESSION["co2s"] = 0;

        echo json_encode (
            array("dt" => substr($value, 0, 2). "-". $_SESSION["co2s"])
        );
    }
    if ($value == "ok") {
        echo json_encode (
            array("dt" => "y")
        );
    }
}

//echo "<pre>", print_r($_SESSION);

?>
